#include <stdio.h>
#include <unistd.h>
#include "yalog.h"

#define FILENAME "/tmp/log_test.log"

int
main(void) {
  printf("Log num: %d\n", yalogfd(STDOUT_FILENO));
  printf("Log num: %d\n", yalogfn(FILENAME));
  yalog_close(0);
  yalog_close(1);
  yalog_end();
  printf("Log num: %d\n", yalogfd(STDOUT_FILENO));
  yalog_close(0);
  yalog_end();
  return 0;
}
