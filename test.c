#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include "yalog.h"

#define FILENAME "/tmp/log_test.log"
#define BASENAME "/tmp/log_test"
#define MAXLINES 3

int
main(void) {

  int fd;
  int mylog_a, mylog_b, mylog_c;

  fd = STDOUT_FILENO;

  printf("Opening log with a file descriptor pointing stdout\n");
  if ((mylog_a = yalogfd(fd)) < 0) {
    fprintf(stderr, "test : error creaing log\n");
    fprintf(stderr, "error trace : %s\n", strerror(errno));
    return 1;
  }
  printf("mylog_a : %d\n", mylog_a);

  printf("Opening log with with a filename: '%s'\n", FILENAME);
  if ((mylog_b = yalogfn(FILENAME)) < 0) {
    fprintf(stderr, "test : error creaing log\n");
    fprintf(stderr, "error trace : %s\n", strerror(errno));
    return 1;
  }
  printf("mylog_b : %d\n", mylog_b);

  printf("Opening log with basename '%s' and maxlines %d\n", BASENAME, MAXLINES);
  if ((mylog_c = yalogml(BASENAME, MAXLINES)) < 0) {
    fprintf(stderr, "test : error creaing log\n");
    fprintf(stderr, "error trace : %s\n", strerror(errno));
    return 1;
  }
  printf("mylog_c : %d\n", mylog_c);

  printf("Writing on mylog_a\n");
  yalog_sync_write(mylog_a, "This should be seen on terminal :O");

  printf("Writing on mylog_b\n");
  yalog_sync_write(mylog_b, "Hello World!");

  printf("Writing on mylog_c\n");
  yalog_sync_write(mylog_c, "First line");
  yalog_sync_write(mylog_c, "Second line");
  yalog_sync_write(mylog_c, "Third line");
  yalog_sync_write(mylog_c, "New file");

  printf("Try to close log 5 (non existent error)\n");
  yalog_close(5);

  printf("Close and deallocate all memory\n");
  yalog_end();

  return 0;
}
