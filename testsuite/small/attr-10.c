/* { dg-options "-DCE_EXTRACT_FUNCTIONS=main -DCE_NO_EXTERNALIZATION" }*/

typedef unsigned short int sa_family_t;
typedef unsigned int __socklen_t;
typedef __socklen_t socklen_t;
typedef int ares_socket_t;
typedef socklen_t ares_socklen_t;

struct sockaddr {
  sa_family_t sa_family;
  char sa_data[14];
};

typedef union {
  struct sockaddr *__restrict __sockaddr__;
} __SOCKADDR_ARG __attribute__((__transparent_union__));

extern int getsockname(int __fd, __SOCKADDR_ARG __addr,
                       socklen_t *__restrict __len)
    __attribute__((__nothrow__));

int main(void) {
  struct sockaddr *src_addr;
  ares_socket_t sock;
  ares_socklen_t len;
  getsockname(sock, src_addr, &len);
}

/* { dg-final { scan-tree-dump "__attribute__\(\(__transparent_union__\)\)" } } */
