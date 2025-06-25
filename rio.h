#ifndef INCLUDED_RIO_H
#define INCLUDED_RIO_H

#define MAXLINE 4096
#define RIO_BUFSIZE 8192

#include "stdio.h"
#include <stddef.h>
#include <sys/types.h>

typedef struct {
  int rio_fd;                // descriptor for this internal buf
  int rio_cnt;               // unread bytes in internal buf
  char *rio_bufptr;          // next unread byte in internal buf
  char rio_buf[RIO_BUFSIZE]; // internal buffer
} rio_t;

/**
 * unbuffered read
 */
ssize_t rio_readn(int fd, void *usrbuf, size_t n);
/**
 * unbuffered write
 */
ssize_t rio_writen(int fd, void *usrbuf, size_t n);

/**
 * associates descriptor `fd` with a read buffer of type `rio_t` at address `rp`
 */
void rio_readinitb(rio_t *rp, int fd);

/**
 * buffered readline
 */
ssize_t rio_readlineb(rio_t *rp, void *usrbuf, size_t max_len);

/**
 * buffered read
 */
ssize_t rio_readnb(rio_t *rp, void *usrbuf, size_t n);

#endif
