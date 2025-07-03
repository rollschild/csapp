#include "rio.h"
#include "sys/select.h"
#include <fcntl.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <unistd.h>

#define LISTENQ 1024 /* Second argument to listen() */
typedef struct sockaddr SA;

/**
 * represents a pool of connected descriptors
 */
typedef struct {
  int max_fd;                   // largest descriptor in read_set
  fd_set read_set;              // set of all active descriptors
  fd_set ready_set;             // subset of descriptors ready for reading
  int n_ready;                  // number of ready descriptors from select
  int max_i;                    // high water index into client array
  int client_fd[FD_SETSIZE];    // set of active descriptors
  rio_t client_rio[FD_SETSIZE]; // set of active read buffers
} pool;

int byte_cnt = 0; // total bytes received by server

/**
Return a listening descriptor that is ready to receive connection requests on
`port`
*/
int open_listenfd(char *port) {
  struct addrinfo hints, *listp, *p;
  int listenfd, rc, optval = 1;

  // get list of potential server addresses
  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_socktype = SOCK_STREAM;             // accept connections
  hints.ai_flags = AI_PASSIVE | AI_ADDRCONFIG; // ... on any IP addresses
  hints.ai_flags |= AI_NUMERICSERV;            // ... using port number
  if ((rc = getaddrinfo(NULL, port, &hints, &listp)) != 0) {
    fprintf(stderr, "getaddrinfo failed (port %s): %s\n", port,
            gai_strerror(rc));
    return -2;
  }

  // walk the list for one that we can bind to
  for (p = listp; p; p = p->ai_next) {
    // create socket descriptor
    if ((listenfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0) {
      continue; // socket failed, try next
    }

    // eliminate "address already in use" error from bind
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval,
               sizeof(int));

    // bind descriptor to address
    if (bind(listenfd, p->ai_addr, p->ai_addrlen) == 0) {
      break; // success
    }

    close(listenfd); // bind failed, try next
  }

  // cleanup
  freeaddrinfo(listp);
  if (!p) {
    return -1;
  }

  // make it a listening socket ready to accept connection requests
  if (listen(listenfd, LISTENQ) < 0) {
    close(listenfd);
    return -1;
  }

  return listenfd;
}

/**
 * Initializes the pool of active clients
 */
void init_pool(int listen_fd, pool *p) {
  // initially, no connected descriptors
  int i;
  p->max_i = -1;
  for (i = 0; i < FD_SETSIZE; i++) {
    p->client_fd[i] = -1;
  }

  // initially, listenfd is only member of select read set
  p->max_fd = listen_fd;
  FD_ZERO(&p->read_set);
  FD_SET(listen_fd, &p->read_set);
}

void app_error(char *msg) /* Application error */
{
  fprintf(stderr, "%s\n", msg);
  exit(0);
}

/**
 * Add new client connection to the pool
 */
void add_client(int conn_fd, pool *p) {
  int i;
  p->n_ready--;
  for (i = 0; i < FD_SETSIZE; i++) { /* find an available slot */
    if (p->client_fd[i] < 0) {
      // add connected descriptor to pool
      p->client_fd[i] = conn_fd;
      rio_readinitb(&p->client_rio[i], conn_fd);

      // add descriptor to descriptor set
      FD_SET(conn_fd, &p->read_set);

      if (conn_fd > p->max_fd) {
        p->max_fd = conn_fd;
      }
      if (i > p->max_i) {
        p->max_i = i;
      }
      break;
    }
  }

  if (i == FD_SETSIZE) { /* could NOT find an empty slot */
    app_error("add_client error: Too many clients!");
  }
}

void check_clients(pool *p) {
  int i, conn_fd, n;
  char buf[MAXLINE];
  rio_t rio;

  for (i = 0; (i <= p->max_i) && (p->n_ready > 0); i++) {
    conn_fd = p->client_fd[i];
    rio = p->client_rio[i];

    // if descriptor is read, echo a text line from it
    if ((conn_fd > 0) && (FD_ISSET(conn_fd, &p->ready_set))) {
      p->n_ready--;
      if ((n = rio_readlineb(&rio, buf, MAXLINE))) {
        byte_cnt += n;
        printf("Server received %d (%d total) bytes on fd %d\n", n, byte_cnt,
               conn_fd);
        rio_writen(conn_fd, buf, n);
      } else { /* EOF detected, remove descriptor from pool */
        // because client has closed its end of the connection
        close(conn_fd);
        FD_CLR(conn_fd, &p->read_set);
        p->client_fd[i] = -1;
      }
    }
  }
}

int main(int argc, char **argv) {
  int listen_fd, conn_fd;
  socklen_t client_len;
  struct sockaddr_storage client_addr;
  static pool pool;

  if (argc != 2) {
    fprintf(stderr, "Usage: %s <port>\n", argv[0]);
    exit(0);
  }

  listen_fd = open_listenfd(argv[1]);
  init_pool(listen_fd, &pool);

  while (1) {
    // wait for listening/connected descriptors to become ready
    pool.ready_set = pool.read_set;
    pool.n_ready = select(pool.max_fd + 1, &pool.ready_set, NULL, NULL, NULL);

    // if listening descriptor is ready, add new client to pool
    if (FD_ISSET(listen_fd, &pool.ready_set)) {
      client_len = sizeof(struct sockaddr_storage);
      conn_fd = accept(listen_fd, (SA *)&client_addr, &client_len);
      add_client(conn_fd, &pool);
    }

    // echo a text line from each ready connected descriptor
    check_clients(&pool);
  }
}
