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
#include <math.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

struct _threads {
  int t_num;                    /* Number of active threads */
  int t_len;                    /* Lenght of the threads array */
  pthread_t * * t_threads;      /* Array of threads */
};
/* Control of active threads */

struct _limits {
  char * l_bname;               /* Basename of the logs filename */
  int l_index;                  /* File number */
  int l_mlines;                 /* Maximum number of lines per log */
  int l_lines;                  /* Actual number of lines */
};
/* Contains the limits of the log files */

struct _log {
  int l_fd;                     /* File descriptor to write the log */
  sem_t * l_wsem;               /* Semaphore lock while writing */
  struct _threads * l_threads;  /* Threads created by this log */
  int l_isfile;                 /* Set when the file descriptor must be closed */
  struct _limits * l_limits;    /* Limits of the log, such a maximum number of
                                   lines per file */
};
/* Defines a single log. This structure controls the work of the log */

struct _args {
  struct _log * a_l;            /* The log to perform the write */
  const void * a_buf;           /* Buffer to write on the log */
};
/* The parameters for the write function */

static size_t _len = 0;
/* Lenght of the array of logs */

static struct _log * * _logs;
/* Array of logs */

static int
_getfree(void * ptrs, size_t size) {
  int index;

  for (index = 0; index < size && (*(void * * *)ptrs)[index] != NULL; index++);

  if (index >= size)
    return -1;
  return index;
}
/* Returns a free position on an array of pointers. If no free position is
   found returns -1 */

static void
_resize(void * ptrs, size_t size, size_t elem, int num) {
  *(void * * *)ptrs = realloc(*(void * * *)ptrs, elem * num);

  for (int i = size; i < num; i++)
    (*(void * * *)ptrs)[i] = NULL;
}
/* Resizes an array of pointers */

static void *
_get(void * ptrs, size_t size, int index) {
  void * element;

  if (index >= size) {
    errno = EFAULT;
    perror("_get : index out of range");
    return NULL;
  }

  element = (*(void * * *)ptrs)[index];
  if (element == NULL) {
    errno = EIO;
    perror("_get : bad number");
    return NULL;
  }

  return element;
}
/* Returns a pointer from the array of pointers */

static int
_add(void * ptrs, size_t * size, void * elem) {
  int index;
  if (*size == 0) {
    *(void * * *)ptrs = malloc(sizeof(elem));
    (*size)++;
    index = 0;
  }
  else {
    if ((index = _getfree(ptrs, *size)) < 0) {
      _resize(ptrs, *size, sizeof(elem), *size * 2);
      index = *size;
      *size = *size * 2;
    }
  }
  (*(void * * *)ptrs)[index] = elem;
  return index;
}
/* Adds an element to the array of pointers */

static int
_exists(char * fname) {
  if (access(fname, F_OK) < 0)
    return -1;
  return 1;
}
/* Checks the existence of a file by it's file name */

static int
_count(int fd) {
  int lines;
  char buf[2];

  lines = 0;
  while (read(fd, buf, 1) > 0) {
    if (buf[0] == '\n')
      lines++;
  }

  return lines;
}
/* Counts the lines of a file given a file descriptor */

static int
_open(const char * fname, int flags) {
  int fd;

  if ((fd = open(fname, flags, 0644)) < 0) {
    perror("_open : error opening file");
    return -1;
  }

  return fd;
}
/* Opens a file by it's file name and flags, and returns the file descriptor */

static int
_write(int fd, const void * buf, size_t size) {
  ssize_t bytes;

  if ((bytes = write(fd, buf, size)) < 0) {
    if (errno == EINTR || errno == EWOULDBLOCK || errno == EAGAIN) {
      return _write(fd, buf + bytes, size - bytes);
    }
    else {
      perror("_write : error while writing");
      return -1;
    }
  }

  return 0;
}
/* Writes buf to a file descriptor the first size bytes */

static int
_mklog(struct _log * l, int fd, int isfile, struct _limits * limits) {
  sem_t * wsem;
  struct _threads * threads;

  l->l_fd = fd;

  wsem = malloc(sizeof(sem_t));
  if (sem_init(wsem, 0, 1) < 0) {
    perror("_mklog : error creating semaphore");
    return -1;
  }
  l->l_wsem = wsem;

  threads = malloc(sizeof(struct _threads));
  threads->t_num = 0;
  threads->t_len = 0;
  threads->t_threads = NULL;
  l->l_threads = threads;

  l->l_isfile = isfile;

  l->l_limits = limits;

  return 0;
}
/* Creates the log and sets all the data ready to write */

static char *
_mkname(char * bname, int index) {
  char * str;
  int digits;

  if (index == 0)
    digits = 1;
  else
    digits = floor(log10(abs(index))) + 1;

  str = malloc(sizeof(char) * (strlen(bname) + 1 + digits + 4 + 2));
  sprintf(str, "%s-%d.log", bname, index);

  return str;
}
/* Creates a string containing the base name for the logs and the actual index */

static int
_mknext(struct _limits * limits) {
  int fd;
  char * fname;

  limits->l_index++;
  fname = _mkname(limits->l_bname, limits->l_index);
  if ((fd = _open(fname, O_WRONLY | O_APPEND | O_CREAT)) < 0) {
    perror("_mknext : error opening file descriptor");
    return -1;
  }
  free(fname);

  limits->l_lines = 0;

  return fd;
}
/* Opens the next log and returns the file descriptor */

static int
_mklimits(struct _limits * limits, char * bname, int mlines) {
  int fd;
  char * fname;
  int index, lines;

  index = 0;
search:
  fname = _mkname(bname, index);
  if (_exists(fname) < 0) {
    if ((fd = _open(fname, O_WRONLY | O_APPEND | O_CREAT)) < 0) {
      perror("_mklimits : error creating a new file");
      return -1;
    }
    lines = 0;
  }
  else {
    if ((fd = _open(fname, O_RDWR)) < 0) {
      perror("_mklimits : error opening file");
      return -2;
    }
    if ((lines = _count(fd)) >= mlines) {
      free(fname);
      index++;
      goto search;
    }
  }

  free(fname);

  limits->l_bname = malloc(sizeof(char) * (strlen(bname) + 1));
  strcpy(limits->l_bname, bname);
  limits->l_index = index;
  limits->l_mlines = mlines;
  limits->l_lines = lines;

  return fd;
}
/* Creates the limits for the log and opens the next file descriptor */

int
_mkwrite(struct _args * argv) {
  int retval;
  ssize_t size;

  if (sem_wait(argv->a_l->l_wsem) != 0) {
    perror("_mkwrite : error on write semaphore wait");
    return -1;
  }

  size = (ssize_t)strlen(argv->a_buf);
  if (_write(argv->a_l->l_fd, argv->a_buf, size) < 0) {
    perror("_mkwrite : error writing to descriptor");
    retval = -2;
    goto post;
  }

  if (_write(argv->a_l->l_fd, "\n", 1) < 0) {
    perror("_mkwrite : error writing to descriptor");
    retval = -2;
    goto post;
  }

  if (argv->a_l->l_limits != NULL) {
    argv->a_l->l_limits->l_lines++;
    if (argv->a_l->l_limits->l_lines >= argv->a_l->l_limits->l_mlines) {
      int fd;
      if ((fd = _mknext(argv->a_l->l_limits)) < 0) {
        perror("_mkwrite : error opening next file");
        retval = -3;
        goto post;
      }
      if (close(argv->a_l->l_fd) != 0) {
        perror("_mkwrite : error closing old descriptor");
        retval = -4;
        goto post;
      }
      argv->a_l->l_fd = fd;
    }
  }

  retval = 0;
post:
  if (sem_post(argv->a_l->l_wsem) != 0) {
    perror("_mkwrite : error posting write semaphore");
    return -5;
  }

  free(argv);
  argv = NULL;

  return retval;
}
/* Waits for a free semaphore to write and after blocking it, writes the buf */

int
yalogfd(int fd) {
  struct _log * l;
  int index;

  l = malloc(sizeof(struct _log));
  if (_mklog(l, fd, 0, NULL) < 0) {
    perror("yalogfd : error creating log");
    return -1;
  }

  index = _add(&_logs, &_len, l);

  return index;
}
/* Creates a log given it's file descriptor and returns the index of the log */

int
yalogfn(char * fname) {
  struct _log * l;
  int fd, index;

  if ((fd = _open(fname, O_WRONLY | O_APPEND | O_CREAT)) < 0) {
    perror("yalogfn : error opening the log file");
    return -1;
  }

  l = malloc(sizeof(struct _log));
  if (_mklog(l, fd, 1, NULL) < 0) {
    perror("yalogfn : error creating log");
    return -2;
  }

  index = _add(&_logs, &_len, l);

  return index;
}
/* Creates a log given a file name and returns the index of the log */

int
yalogml(char * bname, int mlines) {
  struct _log * l;
  struct _limits * limits;
  int fd, index;

  limits = malloc(sizeof(struct _limits));
  if ((fd = _mklimits(limits, bname, mlines)) < 0) {
    perror("yalogml : error creating limits");
    return -1;
  }

  l = malloc(sizeof(struct _log));
  if (_mklog(l, fd, 1, limits) < 0) {
    perror("yalogml : error creating log");
    return -2;
  }

  index = _add(&_logs, &_len, l);

  return index;
}
/* Creates a log with a maximum amount of lines per file */

int
yalog_sync_write(int numlog, const char * buf) {
  struct _log * l;
  struct _args * argv;

  if ((l = _get(&_logs, _len, numlog)) == NULL) {
    perror("yalog_sync_write : error obtaining log");
    return -1;
  }

  argv = malloc(sizeof(struct _args));
  argv->a_l = l;
  argv->a_buf = buf;

  if (_mkwrite(argv) < 0) {
    free(argv);
    perror("yalog_sync_write : error on log write");
    return -1;
  }

  argv = NULL;

  return 0;
}
/* Writes a message to the log synchronous */

int
yalog_close(int numlog) {
  int retval;
  struct _log * l;

  if ((l = _get(&_logs, _len, numlog)) == NULL) {
    perror("yalog_close : error obtaining log");
    return -1;
  }

  retval = 0;

  for (int i = 0; i < l->l_threads->t_num; i++)
    if (l->l_threads->t_threads[i] != NULL)
      pthread_join(*l->l_threads->t_threads[i], NULL);
  free(l->l_threads->t_threads);
  free(l->l_threads);
  l->l_threads = NULL;

  if (sem_destroy(l->l_wsem) < 0) {
    perror("yalog_close : error destroying write semaphore");
    retval = -2;
  }
  free(l->l_wsem);
  l->l_wsem = NULL;

  if (l->l_isfile) {
    if (close(l->l_fd) != 0) {
      perror("yalog_close : error closing file descriptor");
      retval = -3;
    }
  }

  if (l->l_limits != NULL) {
    free(l->l_limits->l_bname);
    free(l->l_limits);
    l->l_limits = NULL;
  }

  free(l);

  l = NULL;

  _logs[numlog] = l;

  return retval;
}
/* Closes the log and cleans all the memory usage */

int
yalog_end() {
  for (int i = 0; i < _len; i++) {
    if (_logs[i] != NULL) {
      yalog_close(i);
    }
  }

  _len = 0;

  free(_logs);

  return 0;
}
/* Closes all the logs and clears all the memory used by yalog */
