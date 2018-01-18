#include <stdio.h>
#include "yalog.h"

int
newlog(int fd) {
  int retval;

  retval = yalogfd(fd);
  printf("Added log with fd %d on index %d\n", fd, retval);
  
  return retval;
}

int
dellog(int index) {
  int retval;
  
  if ((retval = endlog(index)) < 0)
    printf("Error deleting log with index %d\n", index);
  else
    printf("Deleted log with index %d\n", index);
    
  return retval;
}

int
main(void) {
  
  int logs[5];
  printf("Add three logs: \n");
  logs[0] = newlog(1);
  logs[1] = newlog(2);
  logs[2] = newlog(3);
  showlogs();
  
  printf("\n");
  
  printf("Erase log 1: \n");
  dellog(logs[1]);
  showlogs();
  
  printf("\n");
  
  printf("Erase log 3: \n");
  dellog(3);
  showlogs();
  
  printf("\n");
  
  printf("Add one log: \n");
  logs[3] = newlog(4);
  showlogs();
  
  delyalog();

  return 0;
}
