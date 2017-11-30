/* logger - Simple message logger
 * This file is under the MIT license
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#include "yalog.h"


struct _log {
  int l_fd;       /* The file descriptor to write */
  int l_isfile;   /* Set when file is openned by yalog */
};
/* Structure for the logs */

static int _num = 0;
/* The number of logs created */

static int _len = 0;
/* The number of elements on the logs array */

static struct _log ** _logs;
/* Array containing all log pointers */

int
_add_log(struct _log * l) {
  if (_num == 0) {
    _logs = malloc(sizeof(struct _log));
    _len++;
  }
  else if (_num == _len) {
    _len = _len * 2;
    _logs = realloc(_logs, sizeof(struct _log) * _len);
    for (int i = _num + 1; i < _len; i++) {
      _logs[i] = NULL;
    }
  }
  _logs[_num] = l;
  return _num++;
}
/* Adds a log on the array */

int
_open_file(const char * filename) {
  int fd;

  if ((fd = open(filename, O_WRONLY | O_APPEND | O_CREAT, 0644)) < 0)
    return -1;

  return fd;
}
/* Opens a file and returns the file descriptor */

int
_writefd(int fd, const void * buf, size_t count) {
  ssize_t bytes;

  bytes = write(fd, buf, count);

  if (bytes < 0) {
    if (errno == EINTR || errno == EWOULDBLOCK || errno == EAGAIN) {
      return _logger_write_descriptor(fd, buf + bytes, count - bytes, err);
    }
    else {
      return -1;
    }
  }

  return 0;
}
/* Writes all bytes to a file descriptor */

int
yalogfn(char * filename) {
  struct _log * l;
  int fd, index;

  if ((fd = _open_file(filename)) < 0) {
    perror("yalogfn : error opening file descriptor");
    return -1;
  }
  l = malloc(sizeof(struct _log));
  l->l_fd = fd;
  l->l_isfile = 1;
  index = _add_log(l);

  return index;
}
/* Initializes an instance of the logger with a filename */

int
yalogfd(int fd) {
  struct _log * l;
  int index;

  l = malloc(sizeof(struct _log));
  l->l_fd = fd;
  l->l_isfile = 0;
  index = _add_log(l);

  return index;
}
/* Initializes an instance of the logger with an open file descriptor */

int
yalog_close(int numlog) {
  int retval;

  if (numlog >= _num) {
    errno = EFAULT;
    perror("yalog_close : index out of range");
    return -1;
  }

  if (_logs[numlog] == NULL) {
    errno = EIO;
    perror("yalog_close : log already closed");
    return -2;
  }

  retval = 0;
  if (_logs[numlog]->l_isfile) {
    if (close(_logs[numlog]->l_fd) != 0) {
      perror("yalog_close : error closing file");
      retval = -3;
    }
  }

  free(_logs[numlog]);
  _logs[numlog] = NULL;

  return retval;
}
/* Closes a single log */

int
yalog_end() {
  for (int i = 0; i < _num; i++) {
    if (_logs[i] != NULL) {
      yalog_close(i);
    }
  }

  _num = 0;

  free(_logs);

  return 0;
}
/* Ends all the logs and frees all the memory */
