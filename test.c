#include <stdio.h>
#include <unistd.h>
#include "yalog.h"

#define FILENAME "/tmp/log_test.log"

int
main(void) {
  int mylog;

  // if ((mylog = yalogfd(STDOUT_FILENO)) < 0) {
  if ((mylog = yalogfn(FILENAME)) < 0) {
    perror("error creating log");
    return 1;
  }

  yalog_async_write(mylog, "Hello World!\n");

  yalog_end();

  return 0;
}
