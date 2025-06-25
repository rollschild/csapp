#include "rio.h"
#include "stdio.h"
#include <errno.h>
#include <string.h>
#include <unistd.h>

/**
 * unbuffered read
 */
ssize_t rio_readn(int fd, void *usrbuf, size_t n) {
  size_t n_left = n;
  ssize_t n_read;
  char *bufp = usrbuf;

  while (n_left > 0) {
    if ((n_read = read(fd, bufp, n_left)) < 0) {
      if (errno == EINTR) {
        // interrupted by sig handler return
        n_read = 0;

      } else {
        return -1;
      }
    } else if (n_read == 0) {
      // EOF
      break;
    }
    n_left -= n_read;
    bufp += n_read;
  }

  return (n - n_left);
}

/**
 * unbuffered write
 */
ssize_t rio_writen(int fd, void *usrbuf, size_t n) {
  size_t n_left = n;
  ssize_t n_written;
  char *bufp = usrbuf;

  while (n_left > 0) {
    if ((n_written = write(fd, bufp, n_left)) <= 0) {
      if (errno == EINTR) {
        // interrupted
        n_written = 0;
      } else {
        return -1;
      }
    }

    n_left -= n_written;
    bufp += n_written;
  }

  return n;
}

/**
 * associates descriptor `fd` with a read buffer of type `rio_t` at address `rp`
 */
void rio_readinitb(rio_t *rp, int fd) {
  rp->rio_fd = fd;
  rp->rio_cnt = 0;
  rp->rio_bufptr = rp->rio_buf;
}

/**
 * internal buffered read
 */
static ssize_t rio_read(rio_t *rp, char *usrbuf, size_t n) {
  int cnt;

  while (rp->rio_cnt <= 0) { /* refill if buf is empty */
    rp->rio_cnt = read(rp->rio_fd, rp->rio_buf, sizeof(rp->rio_buf));
    if (rp->rio_cnt < 0) {
      if (errno != EINTR) { /* interrupted by sig handler return */
        return -1;
      }
    } else if (rp->rio_cnt == 0) { /* EOF */
      return 0;

    } else {
      rp->rio_bufptr = rp->rio_buf; /* reset buffer ptr */
    }
  }

  /* Copy min(n, rp->rio_cnt) bytes from internal buf to user buf */
  cnt = n;
  if (rp->rio_cnt < n) {
    cnt = rp->rio_cnt;
  }
  memcpy(usrbuf, rp->rio_bufptr, cnt);
  rp->rio_bufptr += cnt;
  rp->rio_cnt -= cnt;

  return cnt;
}

/**
 * buffered readline
 */
ssize_t rio_readlineb(rio_t *rp, void *usrbuf, size_t max_len) {
  int n, rc;
  char c, *bufp = usrbuf;

  for (n = 1; n < max_len; n++) {
    if ((rc = rio_read(rp, &c, 1)) == 1) {
      *bufp++ = c;
      if (c == '\n') {
        n++;
        break;
      }
    } else if (rc == 0) {
      if (n == 1) {
        return 0; /* EOF, no data read */
      } else {
        break; /* EOF, some data were read */
      }
    } else {
      return -1; /* error */
    }
  }

  *bufp = 0;    /* terminate with NULL */
  return n - 1; /* excludes NULL */
}

/**
 * buffered read
 */
ssize_t rio_readnb(rio_t *rp, void *usrbuf, size_t n) {
  size_t n_left = n;
  ssize_t n_read;
  char *bufp = usrbuf;

  while (n_left > 0) {
    if ((n_read = rio_read(rp, bufp, n_left)) < 0) {
      return -1; /* errno set by read() */
    } else if (n_read == 0) {
      break; /* EOF */
    }

    n_left -= n_read;
    bufp += n_read;
  }

  return (n - n_left);
}

int main() {
  int n;
  rio_t rio;
  char buf[MAXLINE];
  rio_readinitb(&rio, STDIN_FILENO);
  while ((n = rio_readlineb(&rio, buf, MAXLINE)) != 0) {
    rio_writen(STDOUT_FILENO, buf, n);
  }
}
