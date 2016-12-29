/*  FUSE: Filesystem in Userspace
  Copyright (C) 2001-2007  Miklos Szeredi <miklos@szeredi.hu>

  This program can be distributed under the terms of the GNU LGPLv2.
  See the file COPYING.LIB.
*/

#ifndef FUSE_COMMON_H_
#define FUSE_COMMON_H_

#include <stdint.h>
#include <sys/types.h>

#define FUSE_MAJOR_VERSION 3

#define FUSE_MINOR_VERSION 0

#define FUSE_MAKE_VERSION(maj, min)  ((maj) * 10 + (min))
#define FUSE_VERSION FUSE_MAKE_VERSION(FUSE_MAJOR_VERSION, FUSE_MINOR_VERSION)

#ifdef __cplusplus
extern "C" {
#endif

struct fuse_file_info {
    int flags;

    unsigned int writepage : 1;

    unsigned int direct_io : 1;

    unsigned int keep_cache : 1;

    unsigned int flush : 1;

    unsigned int nonseekable : 1;

    /* Indicates that flock locks for this file should be
       released.  If set, lock_owner shall contain a valid value.
       May only be set in ->release(). */
    unsigned int flock_release : 1;

    unsigned int padding : 27;

    uint64_t fh;

    uint64_t lock_owner;

    uint32_t poll_events;
};



/**************************************************************************
 * Capability bits for 'fuse_conn_info.capable' and 'fuse_conn_info.want' *
 **************************************************************************/

#define FUSE_CAP_ASYNC_READ     (1 << 0)

#define FUSE_CAP_POSIX_LOCKS        (1 << 1)

#define FUSE_CAP_ATOMIC_O_TRUNC     (1 << 3)

#define FUSE_CAP_EXPORT_SUPPORT     (1 << 4)

#define FUSE_CAP_DONT_MASK      (1 << 6)

#define FUSE_CAP_SPLICE_WRITE       (1 << 7)

#define FUSE_CAP_SPLICE_MOVE        (1 << 8)

#define FUSE_CAP_SPLICE_READ        (1 << 9)

#define FUSE_CAP_FLOCK_LOCKS        (1 << 10)

#define FUSE_CAP_IOCTL_DIR      (1 << 11)

#define FUSE_CAP_AUTO_INVAL_DATA    (1 << 12)

#define FUSE_CAP_READDIRPLUS        (1 << 13)

#define FUSE_CAP_READDIRPLUS_AUTO   (1 << 14)

#define FUSE_CAP_ASYNC_DIO      (1 << 15)

#define FUSE_CAP_WRITEBACK_CACHE    (1 << 16)

#define FUSE_CAP_NO_OPEN_SUPPORT    (1 << 17)

#define FUSE_CAP_PARALLEL_DIROPS        (1 << 18)

#define FUSE_CAP_POSIX_ACL              (1 << 19)

#define FUSE_CAP_HANDLE_KILLPRIV         (1 << 20)

#define FUSE_IOCTL_COMPAT   (1 << 0)
#define FUSE_IOCTL_UNRESTRICTED (1 << 1)
#define FUSE_IOCTL_RETRY    (1 << 2)
#define FUSE_IOCTL_DIR      (1 << 4)

#define FUSE_IOCTL_MAX_IOV  256

struct fuse_conn_info {
    unsigned proto_major;

    unsigned proto_minor;

    unsigned max_write;

    unsigned max_read;

    unsigned max_readahead;

    unsigned capable;

    unsigned want;

    unsigned max_background;

    unsigned congestion_threshold;

    unsigned time_gran;

    unsigned reserved[22];
};

struct fuse_session;
struct fuse_pollhandle;
struct fuse_conn_info_opts;


/* ----------------------------------------------------------- *
 * Data buffer                             *
 * ----------------------------------------------------------- */

enum fuse_buf_flags {
    FUSE_BUF_IS_FD      = (1 << 1),

    FUSE_BUF_FD_SEEK    = (1 << 2),

    FUSE_BUF_FD_RETRY   = (1 << 3),
};

enum fuse_buf_copy_flags {
    FUSE_BUF_NO_SPLICE  = (1 << 1),

    FUSE_BUF_FORCE_SPLICE   = (1 << 2),

    FUSE_BUF_SPLICE_MOVE    = (1 << 3),

    FUSE_BUF_SPLICE_NONBLOCK= (1 << 4),
};

struct fuse_buf {
    size_t size;

    enum fuse_buf_flags flags;

    void *mem;

    int fd;

    off_t pos;
};

struct fuse_bufvec {
    size_t count;

    size_t idx;

    size_t off;

    struct fuse_buf buf[1];
};

/* Initialize bufvec with a single buffer of given size */
#define FUSE_BUFVEC_INIT(size__)                \
    ((struct fuse_bufvec) {                 \
        /* .count= */ 1,                \
        /* .idx =  */ 0,                \
        /* .off =  */ 0,                \
        /* .buf =  */ { /* [0] = */ {           \
            /* .size =  */ (size__),        \
            /* .flags = */ (enum fuse_buf_flags) 0, \
            /* .mem =   */ NULL,            \
            /* .fd =    */ -1,          \
            /* .pos =   */ 0,           \
        } }                     \
    } )

#ifdef __cplusplus
}
#endif


/*
 * This interface uses 64 bit off_t.
 *
 * On 32bit systems please add -D_FILE_OFFSET_BITS=64 to your compile flags!
 */

#if defined(__GNUC__) && (__GNUC__ > 4 || __GNUC__ == 4 && __GNUC_MINOR__ >= 6) && !defined __cplusplus
_Static_assert(sizeof(off_t) == 8, "fuse: off_t must be 64bit");
#else
struct _fuse_off_t_must_be_64bit_dummy_struct \
    { unsigned _fuse_off_t_must_be_64bit:((sizeof(off_t) == 8) ? 1 : -1); };
#endif

#endif /* FUSE_COMMON_H_ */
