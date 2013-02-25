/*
 * Copyright (c) 2013 The Chromium OS Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#define _GNU_SOURCE
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "futility.h"

#define MYNAME "futility"
#define SUBDIR "old_bins"

#define LOGFILE "/tmp/futility.log"

/******************************************************************************/

static const char * const usage= "\n\
Usage: " MYNAME " PROGRAM|COMMAND [args...]\n\
\n\
This is the unified firmware utility, which will eventually replace\n\
all the distinct userspace tools formerly produced by the\n\
vboot_reference package.\n\
\n\
When symlinked under the name of one of those previous tools, it can\n\
do one of two things: either it will fully implement the original\n\
behavior, or (until that functionality is complete) it will just exec\n\
the original binary.\n\
\n\
In either case it may also record some usage information in /tmp to\n\
help improve coverage and correctness.\n\
\n\
If you invoke it directly instead of via a symlink, it requires one\n\
argument, which is the name of the old binary to exec. That binary\n\
must be located in a directory named \"" SUBDIR "\" underneath\n\
the " MYNAME " executable.\n\
\n";

static int help(int argc, char *argv[])
{
  futil_cmd_t *cmd;
  int i;

  fputs(usage, stdout);

  printf("The following commands are built-in:\n");

  for (cmd = futil_cmds_start(); cmd < futil_cmds_end(); cmd++)
    printf("  %-20s %s\n",
           cmd->name, cmd->shorthelp);

  printf("\n");

  printf("FYI, you added these args that I'm ignoring:\n");
  for (i = 0; i < argc; i++)
    printf("argv[%d] = %s\n", i, argv[i]);

  return 0;
}
DECLARE_FUTIL_COMMAND(help, help, "Show a bit of help");


/******************************************************************************/
/* Logging stuff */

static int log_fd = -1;

/* Write the string and a newline. Silently give up on errors */
static void log_str(char *str)
{
  int len, done, n;

  if (log_fd < 0)
    return;

  if (!str)
    str = "(NULL)";

  len = strlen(str);
  if (len == 0) {
    str = "(EMPTY)";
    len = strlen(str);
  }

  for (done = 0; done < len; done += n) {
    n = write(log_fd, str+done, len-done);
    if (n < 0)
      return;
  }

  write(log_fd, "\n", 1);
}

static void log_close(void)
{
  struct flock lock;

  if (log_fd >= 0) {
    memset(&lock, 0, sizeof(lock));
    lock.l_type = F_UNLCK;
    lock.l_whence = SEEK_SET;
    if (fcntl(log_fd, F_SETLKW, &lock))
      perror("Unable to unlock log file");

    close(log_fd);
    log_fd = -1;
  }
}

static void log_open(void)
{
  struct flock lock;
  int ret;

  log_fd = open(LOGFILE, O_WRONLY|O_APPEND|O_CREAT, 0666);
  if (log_fd < 0) {

    if (errno != EACCES)
      return;

    /* Permission problems should improve shortly ... */
    sleep(1);
    log_fd = open(LOGFILE, O_WRONLY|O_APPEND|O_CREAT, 0666);
    if (log_fd < 0)                     /* Nope, they didn't */
      return;
  }

  /* Let anyone have a turn */
  fchmod(log_fd, 0666);

  /* But only one at a time */
  memset(&lock, 0, sizeof(lock));
  lock.l_type = F_WRLCK;
  lock.l_whence = SEEK_END;

  ret = fcntl(log_fd, F_SETLKW, &lock); /* this blocks */
  if (ret < 0) {
    log_close();
    return;
  }

  /* delimiter */
  log_str("##### HEY #####");

  {
  /* Can we tell who called us? */
    char truename[PATH_MAX+10];
    char buf[80];
    ssize_t r;
    pid_t parent = getppid();
    snprintf(buf, 80, "/proc/%d/exe", parent);
    strncat(truename, "CALLER:", 7);
    r = readlink(buf, truename+7, PATH_MAX-1);
    if (r >= 0) {
      truename[r+7] = '\0';
      log_str(truename);
    }
  }
}

/******************************************************************************/
/* Here we go */

int main(int argc, char *argv[], char *envp[])
{
  char *progname;
  char truename[PATH_MAX];
  char oldname[PATH_MAX];
  char buf[80];
  pid_t myproc;
  ssize_t r;
  char *s;
  int i;
  futil_cmd_t *cmd;

  log_open();
  for (i = 0; i < argc; i++)
    log_str(argv[i]);
  log_close();

  /* How were we invoked? */
  progname = strrchr(argv[0], '/');
  if (progname)
    progname++;
  else
    progname = argv[0];

  /* Invoked directly by name */
  if (0 == strcmp(progname, MYNAME)) {
    if (argc < 2) {                     /* must have an argument */
      fputs(usage, stderr);
      exit(1);
    }

    /* We can just pass the rest along, then */
    argc--;
    argv++;

    /* So now what name do we want to invoke? */
    progname = strrchr(argv[0], '/');
    if (progname)
      progname++;
    else
      progname = argv[0];
  }

  /* See if it's asking for something we know how to do ourselves */
  for (cmd = futil_cmds_start(); cmd < futil_cmds_end(); cmd++)
    if (0 == strcmp(cmd->name, progname))
      return cmd->handler(argc, argv);

  /* Nope, it must be wrapped */

  /* The old binaries live under the true executable. Find out where that is. */
  myproc = getpid();
  snprintf(buf, 80, "/proc/%d/exe", myproc);
  r = readlink(buf, truename, PATH_MAX-1);
  if (r < 0) {
    fprintf(stderr, "%s is lost: %s => %s: %s\n", MYNAME, argv[0],
            buf, strerror(errno));
    exit(1);
  }
  truename[r] = '\0';
  s = strrchr(truename, '/');           /* Find the true directory */
  if (s) {
    *s = '\0';
  } else {                              /* I don't think this can happen */
    fprintf(stderr, "%s says %s doesn't make sense\n", MYNAME, truename);
    exit(1);
  }
  /* We've allocated PATH_MAX. If the old binary path doesn't fit, it can't be
   * in the filesystem. */
  snprintf(oldname, PATH_MAX, "%s/%s/%s", truename, SUBDIR, progname);

  fflush(0);
  execve(oldname, argv, envp);

  fprintf(stderr, "%s failed to exec %s: %s\n", MYNAME,
          oldname, strerror(errno));
  return 1;
}
