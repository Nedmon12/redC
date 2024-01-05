#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/ip.h>


static void die(const char *msg) {
  int err = errno;
  fprintf(stderr, "[%d] %s\n", err, msg);
  abort();
}

static int32_t query(int fd, const char *text) {
  uint32_t len = (uint32_t)strlen(text);
  if (len > k_max_msg) {
    return -1;
  }

  char wbuf[4 + k_max_msg];
  memcpy(wbuf, &len, 4);
  memcpy(&wbuf[4], text, len);
  //TODO helper function write_all() not implemented yet
  if (int32_t err = write_all(fd, wbuf, 4 + len)) {
    return err;
  }

  char rbuf[4 + k_max_msg + 1];
  errno = 0;
  //TODO helper function read_all()
  int32_t err = read_full(fd, rbuf, 4);
  if (err) {
    if (errno == 0) {
      msg("EOF");
    } else {
      msg("read () error");
    }
    return err;
  }

  memcpy(&len, rbuf, 4); //little endian
  if (len > k_max_msg) {
    msg("too long");
    return -1;
  }

  err = read_full(fd, &rbuf[4], len);
  if (err) {
    msg("read() error");
    return err;
  }

  rbuf[4+len] = '\0';
  printf("server says: %s\n", &rbuf[4]);
  return 0;
}



int main() {
  int fd = socket(AF_INET, SOCK_STREAM, 0);

  if (fd<0) {
    die("socket()");
  }

  struct sockaddr_in addr = {};

  addr.sin_family = AF_INET;
  addr.sin_port = ntohs(1234);
  addr.sin_addr.s_addr = ntohl(INADDR_LOOPBACK); // 127.0.0.1

  int rv = connect(fd, (const struct sockaddr *)&addr, sizeof(addr));

  if (rv) {
    die("connect");
  }
  char msg[] = "hello";
  write(fd,msg,strlen(msg));

  char rbuf[64] = {};
  ssize_t n = read(fd, rbuf, sizeof(rbuf) - 1);

  if (n<0) {
    die("read");
  }

  printf("server says: %s\n", rbuf);
  close(fd);
  return 0;
}



