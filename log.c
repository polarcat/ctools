/*
 * @file log.c: loggin tools
 *
 * @author: <aliaksei.katovich at gmail.com>
 *
 * Copyright (C) 2015  Aliaksei Katovich
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 2 of the License.
 */

#include <ctype.h>
#include <stdio.h>
#include <stdarg.h>

#ifdef LOGFILE_PATH
#define LOGFILE
#endif

#ifdef LOGFILE
void logp(const char *fmt, ...)
{
	static FILE *logfile_;
	va_list param;

	if (!logfile_) {
#ifdef LOGFILE_PATH
		const char *path = LOGFILE_PATH;
#else
		const char *path = getenv("LOGFILE");
#endif
		if (path)
			logfile_ = fopen(path, "w");
	}

	va_start(param, fmt);
	if (!logfile_) {
		vprintf(fmt, param);
	} else {
		vfprintf(logfile_, fmt, param);
		fflush(logfile_);
	}
	va_end(param);
}
#else /* !LOGFILE */
void logp(const char *fmt, ...)
{
	va_list param;
	va_start(param, fmt);
#ifdef ANDROID
#include <android/log.h>
	__android_log_vprint(ANDROID_LOG_INFO, "[LOG]", fmt, param);
#else
	vprintf(fmt, param);
#endif
	va_end(param);
}
#endif /* LOGFILE */

void xx(const char *prefix, const void *buf, unsigned int len)
{
	const char *ptr = (const char *) buf;
	unsigned int k, n, i, skip;

	logp("%s\n%u bytes, %p\n", prefix, len, ptr);
	if (!ptr)
		return;

	skip = 0;
	n = len / 16;
	if (len % 16)
		n++;

	for (k = 0, i = 0; k < n; k++, i += 16) {
		unsigned int *tmp = (unsigned int *) &ptr[i];

		if ((tmp[0] == 0xffffffff && tmp[1] == 0xffffffff &&
		     tmp[2] == 0xffffffff && tmp[3] == 0xffffffff) ||
		    (tmp[0] == 0 && tmp[1] == 0 && tmp[2] == 0 && tmp[3] == 0))
			skip++;
		else
			skip = 0;

		if (skip > 1) {
			continue;
		} else if (skip == 1) {
			logp("%p: 00 00 00 00 00 00 00 00  "
			     "00 00 00 00 00 00 00 00 | %u\n%s",
			     &ptr[i + 0], i + 16, (n - 1) - k ? "...\n" : "");
			continue;
		}

		logp("%p: %02x %02x %02x %02x %02x %02x %02x %02x  "
			"%02x %02x %02x %02x %02x %02x %02x %02x | "
			"%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c | %u\n",
			&ptr[i + 0],
			ptr[i + 0] & 0xff, ptr[i + 1] & 0xff, ptr[i + 2] & 0xff,
			ptr[i + 3] & 0xff, ptr[i + 4] & 0xff, ptr[i + 5] & 0xff,
			ptr[i + 6] & 0xff, ptr[i + 7] & 0xff, ptr[i + 8] & 0xff,
			ptr[i + 9] & 0xff, ptr[i + 10] & 0xff,
			ptr[i + 11] & 0xff, ptr[i + 12] & 0xff,
			ptr[i + 13] & 0xff, ptr[i + 14] & 0xff,
			ptr[i + 15] & 0xff,
			isprint(ptr[i + 0]) ? (ptr[i + 0] & 0xff) : '.',
			isprint(ptr[i + 1]) ? (ptr[i + 1] & 0xff) : '.',
			isprint(ptr[i + 2]) ? (ptr[i + 2] & 0xff) : '.',
			isprint(ptr[i + 3]) ? (ptr[i + 3] & 0xff) : '.',
			isprint(ptr[i + 4]) ? (ptr[i + 4] & 0xff) : '.',
			isprint(ptr[i + 5]) ? (ptr[i + 5] & 0xff) : '.',
			isprint(ptr[i + 6]) ? (ptr[i + 6] & 0xff) : '.',
			isprint(ptr[i + 7]) ? (ptr[i + 7] & 0xff) : '.',
			isprint(ptr[i + 8]) ? (ptr[i + 8] & 0xff) : '.',
			isprint(ptr[i + 9]) ? (ptr[i + 9] & 0xff) : '.',
			isprint(ptr[i + 10]) ? (ptr[i + 10] & 0xff) : '.',
			isprint(ptr[i + 11]) ? (ptr[i + 11] & 0xff) : '.',
			isprint(ptr[i + 12]) ? (ptr[i + 12] & 0xff) : '.',
			isprint(ptr[i + 13]) ? (ptr[i + 13] & 0xff) : '.',
			isprint(ptr[i + 14]) ? (ptr[i + 14] & 0xff) : '.',
			isprint(ptr[i + 15]) ? (ptr[i + 15] & 0xff) : '.',
			i + 16);
	}
}
