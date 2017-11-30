/* logger - Simple message logger
 * This file is under the MIT license
 */

#ifndef YALOG_H
#define YALOG_H

int yalogfn(char * filename);
/* Initializes an instance of the logger with a filename */

int yalogfd(int fd);
/* Initializes an instance of the logger with an open file descriptor */

int yalog_close(int numlog);
/* Closes a single log */

int yalog_end();
/* Ends all the logs and frees all the memory */

#endif
