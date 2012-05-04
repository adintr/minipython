
/* POSIX module implementation */

/* This file is also used for Windows NT/MS-Win and OS/2.  In that case the
   module actually calls itself 'nt' or 'os2', not 'posix', and a few
   functions are either unimplemented or implemented differently.  The source
   assumes that for Windows NT, the macro 'MS_WINDOWS' is defined independent
   of the compiler used.  Different compilers define their own feature
   test macro, e.g. '__BORLANDC__' or '_MSC_VER'.  For OS/2, the compiler
   independent macro PYOS_OS2 should be defined.  On OS/2 the default
   compiler is assumed to be IBM's VisualAge C++ (VACPP).  PYCC_GCC is used
   as the compiler specific macro for the EMX port of gcc to OS/2. */

/* See also ../Dos/dosmodule.c */

#ifdef __APPLE__
   /*
    * Step 1 of support for weak-linking a number of symbols existing on
    * OSX 10.4 and later, see the comment in the #ifdef __APPLE__ block
    * at the end of this file for more information.
    */
#  pragma weak lchown
#  pragma weak statvfs
#  pragma weak fstatvfs

#endif /* __APPLE__ */

#define PY_SSIZE_T_CLEAN

#include "Python.h"
#include "structseq.h"

#if defined(__VMS)
#    include <unixio.h>
#endif /* defined(__VMS) */

#ifdef __cplusplus
extern "C" {
#endif

PyDoc_STRVAR(posix__doc__,
"This module provides access to operating system functionality that is\n\
standardized by the C Standard and the POSIX standard (a thinly\n\
disguised Unix interface).  Refer to the library manual and\n\
corresponding Unix manual entries for more information on calls.");

#ifndef Py_USING_UNICODE
/* This is used in signatures of functions. */
#define Py_UNICODE void
#endif

#if defined(PYOS_OS2)
#define  INCL_DOS
#define  INCL_DOSERRORS
#define  INCL_DOSPROCESS
#define  INCL_NOPMAPI
#include <os2.h>
#if defined(PYCC_GCC)
#include <ctype.h>
#include <io.h>
#include <stdio.h>
#include <process.h>
#endif
#include "osdefs.h"
#endif

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif /* HAVE_SYS_TYPES_H */

#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif /* HAVE_SYS_STAT_H */

#ifdef HAVE_SYS_WAIT_H
#include <sys/wait.h>           /* For WNOHANG */
#endif

#ifdef HAVE_SIGNAL_H
#include <signal.h>
#endif

#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif /* HAVE_FCNTL_H */

#ifdef HAVE_GRP_H
#include <grp.h>
#endif

#ifdef HAVE_SYSEXITS_H
#include <sysexits.h>
#endif /* HAVE_SYSEXITS_H */

#ifdef HAVE_SYS_LOADAVG_H
#include <sys/loadavg.h>
#endif

/* Various compilers have only certain posix functions */
/* XXX Gosh I wish these were all moved into pyconfig.h */
#if defined(PYCC_VACPP) && defined(PYOS_OS2)
#include <process.h>
#else
#if defined(__WATCOMC__) && !defined(__QNX__)           /* Watcom compiler */
#define HAVE_GETCWD     1
#define HAVE_OPENDIR    1
#define HAVE_SYSTEM     1
#if defined(__OS2__)
#define HAVE_EXECV      1
#define HAVE_WAIT       1
#endif
#include <process.h>
#else
#ifdef __BORLANDC__             /* Borland compiler */
#define HAVE_EXECV      1
#define HAVE_GETCWD     1
#define HAVE_OPENDIR    1
#define HAVE_PIPE       1
#define HAVE_POPEN      1
#define HAVE_SYSTEM     1
#define HAVE_WAIT       1
#else
#ifdef _MSC_VER         /* Microsoft compiler */
#define HAVE_GETCWD     1
#define HAVE_SPAWNV     1
#define HAVE_EXECV      1
#define HAVE_PIPE       1
#define HAVE_POPEN      1
#define HAVE_SYSTEM     1
#define HAVE_CWAIT      1
#define HAVE_FSYNC      1
#define fsync _commit
#else
#if defined(PYOS_OS2) && defined(PYCC_GCC) || defined(__VMS)
/* Everything needed is defined in PC/os2emx/pyconfig.h or vms/pyconfig.h */
#else                   /* all other compilers */
/* Unix functions that the configure script doesn't check for */
#define HAVE_EXECV      1
#define HAVE_FORK       1
#if defined(__USLC__) && defined(__SCO_VERSION__)       /* SCO UDK Compiler */
#define HAVE_FORK1      1
#endif
#define HAVE_GETCWD     1
#define HAVE_GETEGID    1
#define HAVE_GETEUID    1
#define HAVE_GETGID     1
#define HAVE_GETPPID    1
#define HAVE_GETUID     1
#define HAVE_KILL       1
#define HAVE_OPENDIR    1
#define HAVE_PIPE       1
#ifndef __rtems__
#define HAVE_POPEN      1
#endif
#define HAVE_SYSTEM     1
#define HAVE_WAIT       1
#define HAVE_TTYNAME    1
#endif  /* PYOS_OS2 && PYCC_GCC && __VMS */
#endif  /* _MSC_VER */
#endif  /* __BORLANDC__ */
#endif  /* ! __WATCOMC__ || __QNX__ */
#endif /* ! __IBMC__ */

#ifndef _MSC_VER

#if defined(__sgi)&&_COMPILER_VERSION>=700
/* declare ctermid_r if compiling with MIPSPro 7.x in ANSI C mode
   (default) */
extern char        *ctermid_r(char *);
#endif

#ifndef HAVE_UNISTD_H
#if defined(PYCC_VACPP)
extern int mkdir(char *);
#else
#if ( defined(__WATCOMC__) || defined(_MSC_VER) ) && !defined(__QNX__)
extern int mkdir(const char *);
#else
extern int mkdir(const char *, mode_t);
#endif
#endif
#if defined(__IBMC__) || defined(__IBMCPP__)
extern int chdir(char *);
extern int rmdir(char *);
#else
extern int chdir(const char *);
extern int rmdir(const char *);
#endif
#ifdef __BORLANDC__
extern int chmod(const char *, int);
#else
extern int chmod(const char *, mode_t);
#endif
/*#ifdef HAVE_FCHMOD
extern int fchmod(int, mode_t);
#endif*/
/*#ifdef HAVE_LCHMOD
extern int lchmod(const char *, mode_t);
#endif*/
extern int chown(const char *, uid_t, gid_t);
extern char *getcwd(char *, int);
extern char *strerror(int);
extern int link(const char *, const char *);
extern int rename(const char *, const char *);
extern int stat(const char *, struct stat *);
extern int unlink(const char *);
extern int pclose(FILE *);
#ifdef HAVE_SYMLINK
extern int symlink(const char *, const char *);
#endif /* HAVE_SYMLINK */
#ifdef HAVE_LSTAT
extern int lstat(const char *, struct stat *);
#endif /* HAVE_LSTAT */
#endif /* !HAVE_UNISTD_H */

#endif /* !_MSC_VER */

#ifdef HAVE_UTIME_H
#include <utime.h>
#endif /* HAVE_UTIME_H */

#ifdef HAVE_SYS_UTIME_H
#include <sys/utime.h>
#define HAVE_UTIME_H /* pretend we do for the rest of this file */
#endif /* HAVE_SYS_UTIME_H */

#ifdef HAVE_SYS_TIMES_H
#include <sys/times.h>
#endif /* HAVE_SYS_TIMES_H */

#ifdef HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif /* HAVE_SYS_PARAM_H */

#ifdef HAVE_SYS_UTSNAME_H
#include <sys/utsname.h>
#endif /* HAVE_SYS_UTSNAME_H */

#ifdef HAVE_DIRENT_H
#include <dirent.h>
#define NAMLEN(dirent) strlen((dirent)->d_name)
#else
#if defined(__WATCOMC__) && !defined(__QNX__)
#include <direct.h>
#define NAMLEN(dirent) strlen((dirent)->d_name)
#else
#define dirent direct
#define NAMLEN(dirent) (dirent)->d_namlen
#endif
#ifdef HAVE_SYS_NDIR_H
#include <sys/ndir.h>
#endif
#ifdef HAVE_SYS_DIR_H
#include <sys/dir.h>
#endif
#ifdef HAVE_NDIR_H
#include <ndir.h>
#endif
#endif

#ifdef _MSC_VER
#ifdef HAVE_DIRECT_H
#include <direct.h>
#endif
#ifdef HAVE_IO_H
#include <io.h>
#endif
#ifdef HAVE_PROCESS_H
#include <process.h>
#endif
#include "osdefs.h"
#include <malloc.h>
#include <windows.h>
#include <shellapi.h>   /* for ShellExecute() */
#define popen   _popen
#define pclose  _pclose
#endif /* _MSC_VER */

#if defined(PYCC_VACPP) && defined(PYOS_OS2)
#include <io.h>
#endif /* OS2 */

#ifndef MAXPATHLEN
#if defined(PATH_MAX) && PATH_MAX > 1024
#define MAXPATHLEN PATH_MAX
#else
#define MAXPATHLEN 1024
#endif
#endif /* MAXPATHLEN */

#ifdef UNION_WAIT
/* Emulate some macros on systems that have a union instead of macros */

#ifndef WIFEXITED
#define WIFEXITED(u_wait) (!(u_wait).w_termsig && !(u_wait).w_coredump)
#endif

#ifndef WEXITSTATUS
#define WEXITSTATUS(u_wait) (WIFEXITED(u_wait)?((u_wait).w_retcode):-1)
#endif

#ifndef WTERMSIG
#define WTERMSIG(u_wait) ((u_wait).w_termsig)
#endif

#define WAIT_TYPE union wait
#define WAIT_STATUS_INT(s) (s.w_status)

#else /* !UNION_WAIT */
#define WAIT_TYPE int
#define WAIT_STATUS_INT(s) (s)
#endif /* UNION_WAIT */

/* Issue #1983: pid_t can be longer than a C long on some systems */
#if !defined(SIZEOF_PID_T) || SIZEOF_PID_T == SIZEOF_INT
#define PARSE_PID "i"
#define PyLong_FromPid PyInt_FromLong
#define PyLong_AsPid PyInt_AsLong
#elif SIZEOF_PID_T == SIZEOF_LONG
#define PARSE_PID "l"
#define PyLong_FromPid PyInt_FromLong
#define PyLong_AsPid PyInt_AsLong
#elif defined(SIZEOF_LONG_LONG) && SIZEOF_PID_T == SIZEOF_LONG_LONG
#define PARSE_PID "L"
#define PyLong_FromPid PyLong_FromLongLong
#define PyLong_AsPid PyInt_AsLongLong
#else
#error "sizeof(pid_t) is neither sizeof(int), sizeof(long) or sizeof(long long)"
#endif /* SIZEOF_PID_T */

/* Don't use the "_r" form if we don't need it (also, won't have a
   prototype for it, at least on Solaris -- maybe others as well?). */
#if defined(HAVE_CTERMID_R) && defined(WITH_THREAD)
#define USE_CTERMID_R
#endif

#if defined(HAVE_TMPNAM_R) && defined(WITH_THREAD)
#define USE_TMPNAM_R
#endif

/* choose the appropriate stat and fstat functions and return structs */
#undef STAT
#undef FSTAT
#undef STRUCT_STAT
#if defined(MS_WIN64) || defined(MS_WINDOWS)
#       define STAT win32_stat
#       define FSTAT win32_fstat
#       define STRUCT_STAT struct win32_stat
#else
#       define STAT stat
#       define FSTAT fstat
#       define STRUCT_STAT struct stat
#endif

#if defined(MAJOR_IN_MKDEV)
#include <sys/mkdev.h>
#else
#if defined(MAJOR_IN_SYSMACROS)
#include <sys/sysmacros.h>
#endif
#if defined(HAVE_MKNOD) && defined(HAVE_SYS_MKDEV_H)
#include <sys/mkdev.h>
#endif
#endif

#if defined _MSC_VER && _MSC_VER >= 1400
/* Microsoft CRT in VS2005 and higher will verify that a filehandle is
 * valid and throw an assertion if it isn't.
 * Normally, an invalid fd is likely to be a C program error and therefore
 * an assertion can be useful, but it does contradict the POSIX standard
 * which for write(2) states:
 *    "Otherwise, -1 shall be returned and errno set to indicate the error."
 *    "[EBADF] The fildes argument is not a valid file descriptor open for
 *     writing."
 * Furthermore, python allows the user to enter any old integer
 * as a fd and should merely raise a python exception on error.
 * The Microsoft CRT doesn't provide an official way to check for the
 * validity of a file descriptor, but we can emulate its internal behaviour
 * by using the exported __pinfo data member and knowledge of the
 * internal structures involved.
 * The structures below must be updated for each version of visual studio
 * according to the file internal.h in the CRT source, until MS comes
 * up with a less hacky way to do this.
 * (all of this is to avoid globally modifying the CRT behaviour using
 * _set_invalid_parameter_handler() and _CrtSetReportMode())
 */
/* The actual size of the structure is determined at runtime.
 * Only the first items must be present.
 */
typedef struct {
    intptr_t osfhnd;
    char osfile;
} my_ioinfo;

extern __declspec(dllimport) char * __pioinfo[];
#define IOINFO_L2E 5
#define IOINFO_ARRAY_ELTS   (1 << IOINFO_L2E)
#define IOINFO_ARRAYS 64
#define _NHANDLE_           (IOINFO_ARRAYS * IOINFO_ARRAY_ELTS)
#define FOPEN 0x01
#define _NO_CONSOLE_FILENO (intptr_t)-2

/* This function emulates what the windows CRT does to validate file handles */
int
_PyVerify_fd(int fd)
{
    const int i1 = fd >> IOINFO_L2E;
    const int i2 = fd & ((1 << IOINFO_L2E) - 1);

    static int sizeof_ioinfo = 0;

    /* Determine the actual size of the ioinfo structure,
     * as used by the CRT loaded in memory
     */
    if (sizeof_ioinfo == 0 && __pioinfo[0] != NULL) {
        sizeof_ioinfo = _msize(__pioinfo[0]) / IOINFO_ARRAY_ELTS;
    }
    if (sizeof_ioinfo == 0) {
        /* This should not happen... */
        goto fail;
    }

    /* See that it isn't a special CLEAR fileno */
    if (fd != _NO_CONSOLE_FILENO) {
        /* Microsoft CRT would check that 0<=fd<_nhandle but we can't do that.  Instead
         * we check pointer validity and other info
         */
        if (0 <= i1 && i1 < IOINFO_ARRAYS && __pioinfo[i1] != NULL) {
            /* finally, check that the file is open */
            my_ioinfo* info = (my_ioinfo*)(__pioinfo[i1] + i2 * sizeof_ioinfo);
            if (info->osfile & FOPEN) {
                return 1;
            }
        }
    }
  fail:
    errno = EBADF;
    return 0;
}

#endif


