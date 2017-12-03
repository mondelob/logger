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

int yalogfd(int fd);
/* Initializes an instance of the logger with an open file descriptor */

int yalogfn(char * filename);
/* Initializes an instance of the logger with a filename */

int yalog_sync_write(int numlog, const char * buf);
/* Writes to log sync */

int yalog_async_write(int numlog, const char * buf);
/* Writes to log async */

int yalog_close(int numlog);
/* Closes a single log */

int yalog_end();
/* Ends all the logs and frees all the memory */

#endif
