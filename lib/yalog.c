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

#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <pthread.h>
#include <semaphore.h>
#include <poll.h>

#include <stdio.h>

#define TMP_FILE "/tmp/.yalog-"

#define SEED_SIZE 50

#define POLL_TIMEOUT 3000

struct _log {
  int l_fd;                 /* File descriptor to the log */
  int l_socket;             /* Local socket */
  int l_isfile;             /* Set when log file is openned by
                               lib */
  sem_t * l_status;         /* Status of the log */
  pthread_t * l_server;     /* Server thread */
  struct _log * l_next;     /* Points to the next log */
};
/* Structure of a log */

static int _num = 0;
/* Number of logs */

static struct _log * _first = NULL;
/* First log */

static int _seed = 0;
/* Seed to generate random numbers */

char *
_mkrand(char * str, size_t size) {
  int c;
  const char set[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
  
  if (_seed <= 0) {
    srand(time(NULL));
    _seed = SEED_SIZE;
  }
  
  if (size) {
    size--;
    for (size_t n = 0; n < size; n++) {
      c = rand() % (int)(sizeof(set) - 1);
      str[n] = set[c];
    }
    str[size] = '\0';
  }
  
  _seed--;
  
  return str;
}
/* Creates a random string */

char *
_mkfn(char * str) {
  char rnd[7];
  
  strcat(str, TMP_FILE);
  _mkrand(rnd, 7);
  strcat(str, rnd);
  
  return str;
}
/* Returns a filename */

int
_rmfile(const char * fn) {
  if (access(fn, F_OK) != -1) {
    if (unlink(fn) == 0)
      return 1;
    else
      return -1;
  }
  return 0;
}
/* Unlinks a file if it's used */

int
_mksocket() {
  int fd;
  struct sockaddr_un addr;
  char fn[strlen(TMP_FILE) + 7];
  
  fn[0] = '\0';
  _mkfn(fn);
  if (_rmfile(fn) < 0) {
    perror("_mksocket : error removing file");
    return -1;
  }
  
  if ((fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
    perror("_mksocket : error creating socket");
    return -2;
  }
  
  addr.sun_family = AF_UNIX;
  strcpy(addr.sun_path, fn);

  if (bind(fd, (struct sockaddr *)(&addr), sizeof(addr)) == -1) {
    perror("_mksocket : error binding socket");
    return -3;
  }

  if (listen(fd, 0) == -1) {
    perror("_mksocket : error setting socket to listen");
    return -4;
  }

  return fd;
}
/* Creates a new socket for the logger to listen */

struct _log *
_mklog(int fd) {
  struct _log * l;
  sem_t * s;
  
  l = malloc(sizeof(struct _log));
  s = malloc(sizeof(sem_t));
  
  l->l_fd = fd;
  l->l_next = NULL;
  l->l_server = NULL;
  
  if ((l->l_socket = _mksocket()) < 0) {
    perror("_mklog : error creating socket");
    return NULL;
  }
  
  if (sem_init(s, 0, 1) < 0) {
    perror("_mklog : error creating semaphore");
    return NULL;
  }
  l->l_status = s;
  
  return l;
}
/* Creates the log */

int
_freelog(struct _log * l) {
  if (l->l_fd == -1)
    return 0;

  l->l_fd = -1;
  
  if (sem_wait(l->l_status) != 0) {
    perror("_freelog : error waiting for semaphore");
    return -1;
  }
  
  if (pthread_join(*l->l_server, NULL) != 0) {
    perror("_freelog : error waiting for server");
    return -2;
  }
  
  if (sem_destroy(l->l_status) < 0) {
    perror("_freelog : error destroying semaphore");
    return -3;
  }
  free(l->l_status);
  l->l_status = NULL;
  
  free(l->l_server);
  l->l_server = NULL;
  
  return 0;
}
/* Sets a log free to use */

int
_rmlog(struct _log * l) {
  if (_freelog(l) < 0)
    return -1;
  
  l->l_next = NULL;
  free(l);
  
  return 0;
}
/* Clears the memory used by a log */

int
_addlog(struct _log * l) {
  int index;
  struct _log * log, * pre;
  
  index = 0;
  
  if (_first == NULL) {
    _first = l;
    goto add;
  }

  log = _first;
  pre = NULL;
  while (log != NULL && log->l_fd != -1) {
    pre = log;
    log = log->l_next;
    index++;
  }
  
  if (pre != NULL)
    pre->l_next = l;
  
  if (log == NULL) {
    log = l;
    goto add;
  }
  l->l_next = log->l_next;
  _rmlog(log);
  goto end;

add:
  _num++;
end:
  return index;
}
/* Adds a log to the list */

struct _log *
_getlog(int index) {
  struct _log * l;
  
  if (index >= _num || index < 0)
    return NULL;
  
  l = _first;
  while (l != NULL && index > 0) {
    l = l->l_next;
    index--;
  }
  
  return l;
}
/* Returns the log on the index */

int
_server(struct _log * l) {
  struct pollfd * fds;
  int numfds;
  int pollval;
  
  fds = malloc(sizeof(struct pollfd));
  fds[0].fd = l->l_socket;
  fds[0].events = POLLIN;
  numfds = 1;
  
  while (l->l_fd >= 0) {
    if (sem_wait(l->l_status) != 0) {
      perror("_server : error waiting for semaphore");
      return -1;
    }
    
    if ((pollval = poll(fds, numfds, POLL_TIMEOUT)) < 0) {
      perror("_server : error polling server");
      return -2;
    }
    else if (pollval == 0) {
      // Timeout
    }
    else {
      for (int i = 0; i < numfds; i++) {
        if (fds[i].revents & POLLIN) {
          if (fds[i].fd == l->l_socket) {
            // Process new connection
          }
          else {
            // Process client message
          }
        }
      }
    }
    
    if (sem_post(l->l_status) != 0) {
      perror("_server : error posting semaphore");
      return -3;
    }
  }
  
  free(fds);
  
  return 0;
}
/* Runs a server listening for new log connections */

int
_runserver(struct _log * l) {
  pthread_t * t;
  
  t = malloc(sizeof(pthread_t));
  
  if (pthread_create(t, NULL, (void *)&_server, l) != 0) {
    perror("_runserver : error creating thread");
    return -1;
  }
  l->l_server = t;
  
  return 0; 
}
/* Creates a new thread that will run the server */

int
yalogfd(int fd) {
  int index;
  struct _log * l;
  
  if ((l = _mklog(fd)) == NULL) {
    perror("yalogfd : error creating log");
    return -1;
  }
  
  index = _addlog(l);
  
  if (_runserver(l) < 0) {
    perror("yalogfd : error starting server");
    return -2;
  }
  
  return index;
}
/* Creates a log given it's file descriptor */

int
endlog(int index) {
  struct _log * l;
  
  if ((l = _getlog(index)) == NULL) {
    return -1;
  }
  
  if (_freelog(l) < 0)
    return -2;
  
  return 0;
}
/* Closes a log and all of it's resources */

int
delyalog() {
  int badlogs;
  struct _log * l;
  
  badlogs = 0;
  l = _first;
  while (l != NULL) {
    _first = l->l_next;
    if (_rmlog(l) < 0)
      badlogs++;
    l = _first;
  }
  
  return badlogs;
}
/* Clears all the memory used by yalog */

void showlogs() {
  struct _log * l;
  int index;

  index = 0;
  while ((l = _getlog(index)) != NULL) {
    printf("%2d - fd : %3d ; next : %s\n", index, l->l_fd,
      l->l_next != NULL ? "yes" : "no");
    index++;
  }
}
