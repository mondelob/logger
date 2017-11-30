/* logger - Simple message logger
 * This file is under the MIT license
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <semaphore.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

#include "logger.h"

struct log {
  int l_fd;                   /* Descriptor to the log */
  sem_t * l_write;            /* Lock while writting */
  int isfile;                 /* Set to check if it's a file openned */
};
/* Structure defining the log */

int
_logger_open_file(const char * filename, const char ** err) {
  int fd;

  if ((fd = open(filename, O_WRONLY | O_APPEND | O_CREAT, 0644)) < 0) {
    *err = strerror(errno);
    return -1;
  }

  return fd;
}
/* Opens a file and returns the file descriptor */

int
_logger_write_descriptor(int fd, const void * buf, size_t count, const char ** err) {
  ssize_t bytes;

  bytes = write(fd, buf, count);

  if (bytes < 0) {
    if (errno == EINTR || errno == EWOULDBLOCK || errno == EAGAIN) {
      return _logger_write_descriptor(fd, buf + bytes, count - bytes, err);
    }
    else {
      *err = strerror(errno);
      return -1;
    }
  }

  return 0;
}
/* Writes to a file descriptor */

int
_logger_write(struct log * l, const char * buf, const char ** err) {
  ssize_t size;

  if (sem_wait(l->l_write) != 0) {
    *err = strerror(errno);
    return -1;
  }

  size = (ssize_t)strlen(buf);

  if (_logger_write_descriptor(l->l_fd, buf, size, err) < 0) {
    return -2;
  }

  if (sem_post(l->l_write) != 0) {
    *err = strerror(errno);
    return -3;
  }

  return 0;
}
/* Writes to the log */

struct log *
logger_filename(const char * filename, const char ** err) {
  struct log * l;
  int fd;
  sem_t write;

  l = malloc(sizeof(struct log *));
  l->isfile = 1;

  if ((fd = _logger_open_file(filename, err)) < 0) {
    return NULL;
  }

  l->l_fd = fd;

  if (sem_init(&write, 0, 1) != 0) {
    *err = strerror(errno);
    return NULL;
  }

  l->l_write = &write;

  return l;
}
/* Initializes an instance of the logger with a filename */

struct log *
logger_descriptor(int fd, const char ** err) {
  struct log * l;
  sem_t write;

  l = malloc(sizeof(struct log *));
  l->isfile = 2;

  l->l_fd = fd;

  if (sem_init(&write, 0, 1) != 0) {
    *err = strerror(errno);
    return NULL;
  }

  l->l_write = &write;

  return l;
}
/* Initializes an instance of the logger with an open file descriptor */

int
logger_sync_write(struct log * l, const char ** err, const char * format, ...) {
  
}
/* Writes synchronly to the log */
