#include <stdio.h>
#include <unistd.h>
#include "yalog.h"

#define FILENAME "/tmp/log_test.log"
#define BASENAME "/tmp/log_test"

int
main(void) {
  int mylog;

  // if ((mylog = yalogfd(STDOUT_FILENO)) < 0) {
  // if ((mylog = yalogfn(FILENAME)) < 0) {
  if ((mylog = yalogml(BASENAME, 10)) < 0) {
    perror("error creating log");
    return 1;
  }

  yalog_async_write(mylog, "Hello World!");
  yalog_async_write(mylog, "Bye World!");

  yalog_end();

  return 0;
}
