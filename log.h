/*
 * @file log.h: logging tools
 *
 * @author: <aliaksei.katovich at gmail.com>
 *
 * Copyright (C) 2015  Aliaksei Katovich
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 2 of the License.
 */

#ifndef LOG_H_
#define LOG_H_

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
#endif

void logp(const char *fmt, ...);

#define ee(fmt, arg...) { \
	int errno_##__func__ = errno; \
	logp("(EE) %s[%d]:%s: " fmt, __FILE__, __LINE__,  __func__, ##arg); \
	if (errno_save_ != 0) \
		logp("(EE) %s: %s, errno=%d\n", __func__, \
		     strerror(errno_save_), errno_save_); \
	errno = errno_##__func__; \
}

#ifdef DEBUG
#define dd(fmt, arg...) logp("(DD) %s:%d: " fmt, __func__, __LINE__, ##arg)
#else
#define dd(fmt, arg...) do {} while(0)
#endif

#define mm(fmt, arg...) logp("(==) " fmt, ##arg)
#define ww(fmt, arg...) logp("(WW) " fmt, ##arg)
#define ii(fmt, arg...) logp("(II) " fmt, ##arg)

#ifdef TRACE
#define tt(fmt, arg...) logp("(TT) %s: " fmt, __func__, ##arg)
#else
#define tt(fmt, arg...) do {} while(0)
#endif

void xx(const char *prefix, const void *buf, unsigned int len);

#define iferr(cond, act) { if (cond) { ee(#cond" failed\n"); act; } }

#ifdef TIMING
#include <sys/time.h>
#define tm(fmt, arg...) { \
        struct timeval curr_##__func__; \
        gettimeofday(&curr_##__func__, NULL); \
        logp("[%ld.%ld] %s:%d: " fmt, \
	     curr_##__func__.tv_sec, curr_##__func__.tv_usec, \
	     __func__, __LINE__, ##arg); \
}
#define tc(f, fmt, arg...) { \
	struct timeval prev_##__func__; \
	struct timeval curr_##__func__; \
	gettimeofday(&prev_##__func__, NULL); \
	f; \
	gettimeofday(&curr_##__func__, NULL); \
	logp("(%.06lfs) "#f": "fmt"\n", \
	     ((double) curr_##__func__.tv_sec + \
	     (double) curr_##__func__.tv_usec * 1.0e-6) - \
	     ((double) prev_##__func__.tv_sec + \
	     (double) prev_##__func__.tv_usec * 1.0e-6), ##arg); \
}
#endif

#endif /* LOG_H_ */
