# Network Programming

## Global IP Internet

- **sockets interface**
- socket functions are typically implemented as system calls that trap into the kernel and call various kernel-mode functions in TCP/IP
- **datagrams**

### IP Address

- IP address is **big endian** network byte order
  - even if the host byte order is little-endian

## Sockets

- internet socket address: 16 bytes
  ```c
  struct sockaddr_in {
      uint16_t       sin_family;
      uint16_t       sin_port;
      struct in_addr sin_addr;
      unsigned char  size_zero[8];
  };
  ```
- `connect`, `bind`, and `accept` require pointer to a protocol-specific socket address structure
  ```c
  struct sockaddr {
      uint16_t sa_family;
      char     sa_data[14];
  };
  ```
- use `getaddrinfo` to generate parameters (`AF_INET`, `SOCK_STREAM`, etc) _automatically_
  - protocol-independent
- listening vs. connected descriptors
  - `listenfd` vs. `connfd`
  - useful for concurrent servers

### Host & Service Conversion

- `getaddrinfo`
- `getnameinfo`

```c
/**
Establish a connection with a server running on `hostname` and listening for
connection requests on port number `port`
*/
int open_clientfd(char *hostname, char *port) {
    int clientfd;
    struct addrinfo hints, *listp, *p;

    // get a list of potential server addresses
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_socktype = SOCK_STREAM; // open a connection
    hints.ai_flags = AI_NUMERICSERV; // ... using a numeric port arg
    hints.ai_flags |= AI_ADDRCONFIG; // recommended for connections
    getaddrinfo(hostname, port, &hints, &listp);

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
    int listenfd, optval = 1;

    // get list of potential server addresses
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_socktype = SOCK_STREAM; // accept connections
    hints.ai_flags = AI_PASSIVE | AI_ADDRCONFIG; // ... on any IP addresses
    hints.ai_flags |= AI_NUMERICSERV; // ... using port number
    getaddrinfo(NULL, port, &hints, &listp);

    // walk the list for one that we can bind to
    for (p = listp; p; p = p->ai_next) {
        // create socket descriptor
        if ((listenfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0) {
            continue; // socket failed, try next
        }

        // eliminate "address already in use" error from bind
        setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (const void*)&optval, sizeof(int));

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
```

## Web Servers

### Serving Dynamic Content

- **Common Gateway Interface (CGI)**
- it calls `fork` to create a child process
- then calls `execve` to run the requested program in the context of the _child_
- _before_ calling `execve`, child process set the CGI env variable `QUERY_STRING`
  - e.g if `GET /cgi-bin/adder?15000&213 HTTP/1.1`
  - then `QUERY_STRING` is `15000&213`
- CGI program can reference env variables by `getenv`
- CGI program sends its dynamic content to the standard output
  - but before child process loads & runs the CGI program it uses `dup2` function to redirect stdout to the connected descriptor associated with client
- parent does _NOT_ know size/type of the content the child generates
  - child responsible for generating `Content-Type` & `Content-Length` response headers
