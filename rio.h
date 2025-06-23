#ifndef INCLUDED_RIO_H
#define INCLUDED_RIO_H

#include "stdio.h"
#include <stddef.h>

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
void rio_readinitb(rio_t *rp, void *usrbuf, size_t max_len);

/**
 * buffered readline
 */
ssize_t rio_readlineb(rio_t *rp, void *usrbuf, size_t max_len);

/**
 * buffered read
 */
ssize_t rio_readnb(rio_t *rp, void *usrbuf, size_t n);

#endif
