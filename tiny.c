/**
 * tiny.c - a simple web server
 */
#include "rio.h"
#include <fcntl.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

extern char **environ; /* Defined by libc */
typedef struct sockaddr SA;
#define MAXBUF 8192  /* Max I/O buffer size */
#define LISTENQ 1024 /* Second argument to listen() */

void do_it(int fd);
void read_requesthdrs(rio_t *rp);
int parse_uri(char *uri, char *filename, char *cgi_args);
void serve_static(int fd, char *filename, int filesize);
void get_filetype(char *filename, char *filetype);
void serve_dynamic(int fd, char *filename, char *cgi_args);
void client_error(int fd, char *cause, char *err_num, char *short_msg,
                  char *long_msg);

/**
Establish a connection with a server running on `hostname` and listening for
connection requests on port number `port`
*/
int open_clientfd(char *hostname, char *port) {
  int clientfd, rc;
  struct addrinfo hints, *listp, *p;

  // get a list of potential server addresses
  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_socktype = SOCK_STREAM; // open a connection
  hints.ai_flags = AI_NUMERICSERV; // ... using a numeric port arg
  hints.ai_flags |= AI_ADDRCONFIG; // recommended for connections
  if ((rc = getaddrinfo(hostname, port, &hints, &listp)) != 0) {
    fprintf(stderr, "getaddrinfo failed (%s:%s): %s\n", hostname, port,
            gai_strerror(rc));
    return -2;
  }

  // walk the list for one that we can successfully connect to
  for (p = listp; p; p = p->ai_next) {
    // create socket descriptor
    if ((clientfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0) {
      continue; // socket failed, try next
    }

    // connect to server
    if (connect(clientfd, p->ai_addr, p->ai_addrlen) != -1) {
      break; // success
    }
    close(clientfd); // connect failed, try another
  }

  // clean up
  freeaddrinfo(listp);
  if (!p) {
    // _ALL_ connects failed
    return -1;
  } else {
    return clientfd;
  }
}

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

int main(int argc, char **argv) {
  int listenfd, connfd;
  char hostname[MAXLINE], port[MAXLINE];
  socklen_t client_len;
  struct sockaddr_storage client_addr;

  // check command line args
  if (argc != 2) {
    fprintf(stderr, "usage: %s <port>\n", argv[0]);
    exit(1);
  }

  listenfd = open_listenfd(argv[1]);

  while (1) {
    client_len = sizeof(client_addr);
    connfd = accept(listenfd, (SA *)&client_addr, &client_len);
    getnameinfo((SA *)&client_addr, client_len, hostname, MAXLINE, port,
                MAXLINE, 0);
    printf("Accepted connection from (%s, %s)\n", hostname, port);

    do_it(connfd);
    close(connfd);
  }
}

void do_it(int fd) {
  int is_static;
  struct stat sbuf; // file status
  char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
  char filename[MAXLINE], cgi_args[MAXLINE];
  rio_t rio;

  // read request line and headers
  rio_readinitb(&rio, fd);
  rio_readlineb(&rio, buf, MAXLINE);
  printf("Request headers: \n");
  printf("%s", buf);
  sscanf(buf, "%s %s %s", method, uri, version);
  if (strcasecmp(method, "GET")) {
    // return non-zero if different
    client_error(fd, method, "501", "NOT implemented",
                 "Tiny does not implement this method");
    return;
  }
  read_requesthdrs(&rio);

  // parse URI from GET request
  is_static = parse_uri(uri, filename, cgi_args);
  if (stat(filename, &sbuf) < 0) {
    client_error(fd, filename, "404", "Not found",
                 "Tiny could not find this file!");
    return;
  }

  if (is_static) { /* serve static content */
    if (!(S_ISREG(sbuf.st_mode)) || !(S_IRUSR & sbuf.st_mode)) {
      client_error(fd, filename, "403", "Forbidden",
                   "Tiny could not read the file!");
      return;
    }
    serve_static(fd, filename, sbuf.st_size);
  } else { /* serve dynamic content */
    if (!(S_ISREG(sbuf.st_mode)) || !(S_IXUSR & sbuf.st_mode)) {
      client_error(fd, filename, "403", "Forbidden",
                   "Tiny could not run the CGI program!");
      return;
    }
    serve_dynamic(fd, filename, cgi_args);
  }
}

void client_error(int fd, char *cause, char *errnum, char *shortmsg,
                  char *longmsg) {
  char buf[MAXLINE], body[MAXBUF];

  // build HTTP response body
  sprintf(body, "<html><title>Tiny Error</title>");
  sprintf(body,
          "%s<body bgcolor="
          "ffffff"
          ">\r\n",
          body);
  sprintf(body, "%s%s: %s\r\n", body, errnum, shortmsg);
  sprintf(body, "%s<p>%s: %s\r\n", body, longmsg, cause);
  sprintf(body, "%s<hr><em>The Tiny Web Server</em>\r\n", body);

  // print HTTP response
  sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
  rio_writen(fd, buf, strlen(buf));
  sprintf(buf, "Content-type: text/html\r\n");
  rio_writen(fd, buf, strlen(buf));
  sprintf(buf, "Content-length: %d\r\n\r\n", (int)strlen(body));
  rio_writen(fd, buf, strlen(buf));
  rio_writen(fd, body, strlen(body));
}

/**
 * Reads and _ignores_ request headers
 */
void read_requesthdrs(rio_t *rp) {
  char buf[MAXLINE];

  rio_readlineb(rp, buf, MAXLINE);
  while (strcmp(buf, "\r\n")) {
    rio_readlineb(rp, buf, MAXLINE);
    printf("%s", buf);
  }
  return;
}

int parse_uri(char *uri, char *filename, char *cgi_args) {
  // Assuming home dir for static content is current dir
  // home dir for executable is ./cgi-bin
  // default file name is ./home.html
  char *ptr;

  // strstr: find first occurrence of null-terminated string in the
  // null-terminated string
  if (!strstr(uri, "cgi-bin")) { /* static content */
    strcpy(cgi_args, "");
    strcpy(filename, ".");
    strcat(filename, uri); // append uri to . => ./index.html
    if (uri[strlen(uri) - 1] == '/') {
      strcat(filename, "home.html"); // default file name
    }
    return 1;
  } else { /* dynamic content */
    ptr = index(uri, '?');
    if (ptr) {
      strcpy(cgi_args, ptr + 1);
      *ptr = '\0';
    } else {
      strcpy(cgi_args, "");
    }

    strcpy(filename, ".");
    strcat(filename, uri);
    return 0;
  }
}

void serve_static(int fd, char *filename, int filesize) {
  int src_fd;
  char *srcp, filetype[MAXLINE], buf[MAXLINE];

  // send response headers to client
  get_filetype(filename, filetype);

  sprintf(buf, "HTTP/1.0 200 OK\r\n");
  sprintf(buf, "%sServer: Tiny Web Server\r\n", buf);
  sprintf(buf, "%sConnection: close\r\n", buf);
  sprintf(buf, "%sContent-length: %d\r\n", buf, filesize);
  sprintf(buf, "%sContent-type: %s\r\n\r\n", buf, filetype);

  rio_writen(fd, buf, strlen(buf));
  printf("Response headers: \n");
  printf("%s", buf);

  // send response body to client
  src_fd = open(filename, O_RDONLY, 0);

  // mmap creates a new mapping in virtual addr space of the calling process
  // starting addr for new mapping is the first arg
  // if addr is NULL, then kernel chooses the (page-aligned) addr to create the
  // mapping
  srcp = mmap(0, filesize, PROT_READ, MAP_PRIVATE, src_fd, 0);
  close(src_fd);
  rio_writen(fd, srcp, filesize);
  munmap(srcp, filesize);
}

/**
 * get_filetype - derive file type from filename
 */
void get_filetype(char *filename, char *filetype) {
  if (strstr(filename, ".html")) {
    strcpy(filetype, "text/html");
  } else if (strstr(filename, ".gif")) {
    strcpy(filetype, "image/gif");
  } else if (strstr(filename, ".png")) {
    strcpy(filetype, "image/png");
  } else if (strstr(filename, ".jpg")) {
    strcpy(filetype, "image/jpeg");
  } else {
    strcpy(filetype, "text/plain");
  }
}

void serve_dynamic(int fd, char *filename, char *cgi_args) {
  char buf[MAXLINE], *empty_list[] = {NULL};

  // return first part of HTTP response
  sprintf(buf, "HTTP/1.0 200 OK\r\n");
  rio_writen(fd, buf, strlen(buf));
  sprintf(buf, "Server: Tiny Web Server\r\n");
  rio_writen(fd, buf, strlen(buf));

  if (fork() == 0) { /* child */
    // real server would set all CGI vars here
    setenv("QUERY_STRING", cgi_args, 1);
    dup2(fd, STDOUT_FILENO);               // redirect stdout to client
    execve(filename, empty_list, environ); // run CGI program
  }
  wait(NULL); // parent waits for and reaps child
}
