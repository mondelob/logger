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

#ifndef YALOG_H
#define YALOG_H

int yalogfd(const int fd);
/* Creates a log given it's file descriptor */

int endlog(int index);
/* Closes a log and all of it's resources */

void showlogs();

int delyalog();
/* Clears all the memory used by yalog */

// int yalogfn(char * fname);
/* Creates a log given a file name and returns the index of the log */

// int yalogml(char * bname, int mlines);
/* Creates a log with a maximum amount of lines per file */

// int yalog_sync_write(int numlog, const char * buf);
/* Writes a message to the log synchronous */

// int yalog_close(int numlog);
/* Closes the log and clears all the memory used by the log */

// int yalog_end();
/* Closes all the logs and clears all the memory used by yalog */

// int yalog_async_write(int numlog, const char * buf);
/* Writes to log async */

#endif
