/* yalog - Simple message logger
 * This file is under the MIT license
 *
 * MIT License
 *
 * Copyright (c) 2017 Bruno Mondelo
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include<pthread.h>
#include <semaphore.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#include "yalog.h"

struct _threads {
  int t_num;          /* Number of threads created */
  int t_len;          /* Actual lenght of the thread array */
  pthread_t * t_tid;  /* Array of threads */
};
/* Structure of the subthreads on a log */

struct _log {
  int l_fd;                 /* The file descriptor to write */
  int l_isfile;             /* Set when file is openned by yalog */
  sem_t * l_write;          /* Write semaphore */
  struct _threads * l_tid;  /* Set of subthreads */
};
/* Structure for the logs */

struct _args {
  struct _log * a_l;      /* Log to write */
  const char * a_buf;     /* Buffer to write*/
};
/* Structure for the write log function */

static int _num = 0;
/* The number of logs created */

static int _len = 0;
/* The number of elements on the logs array */

static struct _log ** _logs;
/* Array containing all log pointers */

int
_create_log(struct _log * l, int fd, int isfile) {
  sem_t * s;
  struct _threads * tid;

  l->l_fd = fd;
  l->l_isfile = isfile;

  s = malloc(sizeof(sem_t));
  if (sem_init(s, 0, 1) < 0) {
    perror("_create_log : error creating semaphore");
    return -1;
  }
  l->l_write = s;

  tid = malloc(sizeof(struct _threads));
  tid->t_num = 0;
  tid->t_len = 0;
  l->l_tid = tid;

  return 0;
}

int
_create_thread(struct _log * l) {
  if (l->l_tid->t_num == 0) {
    l->l_tid->t_tid = malloc(sizeof(pthread_t));
    l->l_tid->t_len++;
  }
  else if (l->l_tid->t_num == l->l_tid->t_len) {
    l->l_tid->t_len = l->l_tid->t_len * 2;
    l->l_tid->t_tid = realloc(l->l_tid->t_tid, sizeof(pthread_t) * l->l_tid->t_len);
    for (int i = l->l_tid->t_num + 1; i < l->l_tid->t_len; i++) {
      l->l_tid->t_tid[i] = -1;
    }
  }
  return l->l_tid->t_num++;
}
/* Creates a thread and assigns the thread to the list */

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

struct _log *
_get_log(int numlog) {
  if (numlog >= _num) {
    errno = EFAULT;
    perror("yalog_close : index out of range");
    return NULL;
  }

  if (_logs[numlog] == NULL) {
    errno = EIO;
    perror("yalog_close : log already closed");
    return NULL;
  }

  return _logs[numlog];
}

int
_open_file(const char * filename) {
  int fd;

  if ((fd = open(filename, O_WRONLY | O_APPEND | O_CREAT, 0644)) < 0)
    return -1;

  return fd;
}
/* Opens a file and returns the file descriptor */

int
_write_descriptor(int fd, const void * buf, size_t count) {
  ssize_t bytes;

  bytes = write(fd, buf, count);

  if (bytes < 0) {
    if (errno == EINTR || errno == EWOULDBLOCK || errno == EAGAIN) {
      return _write_descriptor(fd, buf + bytes, count - bytes);
    }
    else {
      perror("_write_descriptor : error writing");
      return -1;
    }
  }

  return 0;
}
/* Writes all bytes to a file descriptor */

int
_write_log(struct _args * argv) {
  struct _log * l = argv->a_l;
  const void * buf = argv->a_buf;
  free(argv);

  ssize_t size;

  if (sem_wait(l->l_write) != 0) {
    perror("_write_log : error waiting for semaphore");
    return -1;
  }

  size = (ssize_t)strlen(buf);

  if (_write_descriptor(l->l_fd, buf, size) < 0) {
    perror("_write_log : error writing to descriptor");
    return -2;
  }

  if (_write_descriptor(l->l_fd, "\n", 1) < 0) {
    perror("_write_log : error writing new line to descriptor");
    return -3;
  }

  if (sem_post(l->l_write) != 0) {
    perror("_write_log : error posting semaphore");
    return -3;
  }

  return 0;
}
/* Writes to a log */

int
yalogfn(char * filename) {
  struct _log * l;
  int fd, index;

  if ((fd = _open_file(filename)) < 0) {
    perror("yalogfn : error opening file descriptor");
    return -1;
  }

  l = malloc(sizeof(struct _log));
  if (_create_log(l, fd, 1) < 0) {
    perror("yalogfn : error creating log");
    return -2;
  }

  index = _add_log(l);

  return index;
}
/* Initializes an instance of the logger with a filename */

int
yalogfd(int fd) {
  struct _log * l;
  int index;

  l = malloc(sizeof(struct _log));
  if (_create_log(l, fd, 1) < 0) {
    perror("yalogfn : error creating log");
    return -2;
  }

  index = _add_log(l);

  return index;
}
/* Initializes an instance of the logger with an open file descriptor */

int
yalog_sync_write(int numlog, const char * buf) {
  struct _log * l;
  struct _args * argv;

  if ((l = _get_log(numlog)) == NULL) {
    perror("yalog_sync_write : error got log");
    return -1;
  }

  argv = malloc(sizeof(struct _args));
  argv->a_l = l;
  argv->a_buf = buf;

  if (_write_log(argv) < 0) {
    free(argv);
    perror("yalog_sync_write : error writing log");
    return -1;
  }

  argv = NULL;

  return 0;
}
/* Writes to log sync */

int
yalog_async_write(int numlog, const char * buf) {
  struct _log * l;
  struct _args * argv;
  int index;

  if ((l = _get_log(numlog)) == NULL) {
    perror("yalog_async_write : error got log");
    return -1;
  }

  argv = malloc(sizeof(struct _args));
  argv->a_l = l;
  argv->a_buf = buf;

  index = _create_thread(l);
  if (pthread_create(&(l->l_tid->t_tid[index]), NULL, (void *)&_write_log, argv) != 0) {
    free(argv);
    perror("yalog_async_write : error creating thread");
    return -2;
  }
  argv = NULL;

  return 0;
}
/* Writes to log async */

int
yalog_close(int numlog) {
  int retval;
  struct _log * l;

  if ((l = _get_log(numlog)) == NULL) {
    perror("yalog_close : error got log");
    return -1;
  }

  retval = 0;

  for (int i = 0; i < l->l_tid->t_num; i++) {
    if (l->l_tid->t_tid[i] != -1) {
      pthread_join(l->l_tid->t_tid[i], NULL);
    }
  }
  free(l->l_tid->t_tid);
  free(l->l_tid);
  l->l_tid = NULL;

  if (sem_destroy(l->l_write) < 0) {
    perror("yalog_close : error destroying semaphore");
    retval = -3;
  }
  free(l->l_write);

  if (l->l_isfile) {
    if (close(l->l_fd) != 0) {
      perror("yalog_close : error closing file");
      retval = -4;
    }
  }

  free(l);
  l = NULL;

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
