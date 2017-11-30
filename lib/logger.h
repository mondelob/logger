/* logger - Simple message logger
 * This file is under the MIT license
 */

#ifndef LOGGER_H
#define LOGGER_H

struct log;
/* Structure with the log to write */

struct log * logger_filename(const char * filename, const char ** err);
/* Initializes an instance of the logger with a filename */

struct log * logger_descriptor(int fd, const char ** err);
/* Initializes an instance of the logger with an open file descriptor */

int logger_sync_write(struct log * l, const char ** err, const char * format, ...);
/* Writes synchronly to the log */

#endif
