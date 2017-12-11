# YALOG

> Simple log API: Yet Another LOG

With *yalog* you can have access to a very simple log utility.
This program is under the *MIT* license, you can freely use it.

## Contents

*yalog* saves all your logs in a **int** variable, not an structure. When you
want to write a buffer to the log, you must pass this **int** to the respective
function. You can have multiple logs stored in different **int** variables.

To start a *yalog* log you can do it in different ways. The most simple way is
passing to *yalog* a **file descriptor**. In this case all the lines to write
will be writed directly to the **file descriptor**:

```c
int fd;
/* Contains file descriptor */

int mylog_a = yalogfd(fd);
/* This will return the number of your log */
```

You can start a log specifing the name of the desired log with a *char \**:

```c
const char * myfile = "my_first_log.log";
/* The file */

int mylog_b = yalogfn(myfile);
/* This will return the number of your log */
```

Also you could specify a base name and a max lines per log file, and *yalog*
will create progressively the log files. On the startup will search for the old
logs and read the number of lines and the first file will be the one with the
number of lines less than the specified. (Known bug: if you change your max
lines on different executions to an upper number, may cause to *yalog* write on
a old log file. This will be fixed)

```c
const char * basefile = "not_my_first_log";
/* The basename file. Logs will be named not_my_first_log-X.log being X the
   number */

int maxlines = 250;
/* Maximum number of lines per log. When this number is reached yalog will open
   the next file */

int mylog_c = yalogml(basefile, maxlines);
/* This will return the number of your log */
```

Once you have an open log, now you can write *synchronous* or *asynchronous* to
the log, doesn't matter how you opened the log:

```c
yalog_sync_write(mylog_a, "Hello World!\n");
/* Synchronous writing */

yalog_async_write(mylog_b, "Bye World!\n");
yalog_async_write(mylog_c, "Ooooh!\n");
/* Asynchronous writing */
```

Once your done with your log you can close it the log. This is not necessary,
but once you end, you should run *yalog_end* to clear all memory usage and
close all the log's that are still opened:

```c
yalog_close(mylog_a);
/* Closes just mylog_a*/

yalog_end();
/* Closes all the logs and clears the memory usage. This is mandatory */
```

## ToDo

- [x] File descriptor support
- [x] Filename support
- [x] Synchronous and asynchronous writing
- [x] Max lenght or size for created files (file base)
- [ ] Add time stamp to all the log lines
- [ ] Clear memory allocated arrays
- [ ] Portability
- [ ] Installer
