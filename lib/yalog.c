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

#include <stdio.h>

struct _log {
  int l_fd;                 /* File descriptor to the log */
  struct _log * l_next;     /* Points to the next log */
};

static int _num = 0;
/* Number of logs */

static struct _log * _first = NULL;
/* First log */

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

struct _log *
_mklog(int fd) {
  struct _log * l;
  
  l = malloc(sizeof(struct _log));
  
  l->l_fd = fd;
  l->l_next = NULL;
  
  return l;
}
/* Creates the log */

int
_freelog(struct _log * l) {
  if (l->l_fd == -1)
    return -1;

  l->l_fd = -1;
  
  // Stop threads and works
  
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

int
yalogfd(int fd) {
  int index;
  struct _log * l;
  
  l = _mklog(fd);
  
  index = _addlog(l);
  
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
















