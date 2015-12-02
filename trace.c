/*
 * @file trace.c: function call time tracer
 *
 * $ gcc -finstrument-functions \
 *	-finstrument-functions-exclude-function-list=vprintf,gettimeofday,\
 *	 va_start,va_end,logp,__android_log_vprint,backtrace,backtrace_symbols,\
 *	 printf,fclose,snprintf,getenv,fopen -g -c -o main.o main.c
 * $ gcc -c -o trace.o trace.c
 * $ gcc main.o trace.o -o main
 *
 * LDFLAGS = ${LIBS} -L<trace_lib_path> -ltrace
 * #-ldl -rdynamic
 * CFLAGS += -g
 * CFLAGS += -finstrument-functions
 *
 * @author: <aliaksei.katovich at gmail.com>
 *
 * Copyright (C) 2015  Aliaksei Katovich
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 2 of the License.
 */

#include <time.h>
#include <stdio.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdlib.h>
#include <sys/time.h>
#include <pthread.h>

#ifdef LOGPATH
#define LOGFILE
#endif

#define __skip__ __attribute__ ((no_instrument_function))

static FILE *logfile__;

#ifdef LOGFILE
static void __skip__ logp(const char *fmt, ...)
{
	va_list param;

	va_start(param, fmt);
	if (!logfile__) {
		vprintf(fmt, param);
	} else {
		vfprintf(logfile__, fmt, param);
		fflush(logfile__);
	}
	va_end(param);
}
#else /* !LOGFILE */
static void __skip__ logp(const char *fmt, ...)
{
	va_list param;
	va_start(param, fmt);
#ifdef ANDROID
#include <android/log.h>
	__android_log_vprint(ANDROID_LOG_INFO, "(tc)", fmt, param);
#else
	vprintf(fmt, param);
#endif
	va_end(param);
}
#endif /* LOGFILE */

void __attribute__ ((constructor)) trace_begin(void)
{
#ifdef LOGPATH
	const char *path = LOGPATH;
#else
	const char *path = getenv("LOGPATH");
#endif
	if (path)
		logfile__ = fopen(path, "w");
}

void __attribute__ ((destructor)) trace_end(void)
{
	if (logfile__ != NULL)
		fclose(logfile__);
}

struct trace_ctx {
	struct timeval enter_time;
	struct timeval exit_time;
	void *func;
	const char *func_name;
	void *caller;
	const char *caller_name;
#ifdef MULTI_THREAD
	pthread_t id;
#else
	int id;
#endif
};

#ifndef MULTI_THREAD
static struct trace_ctx ctx;
#else

static pthread_key_t priv_key;
static pthread_once_t thread_once = PTHREAD_ONCE_INIT;

static void __skip__ trace_make_key(void)
{
	pthread_key_create(&priv_key, NULL);
}
#endif /* MULTI_THREAD */

void __cyg_profile_func_enter(void *func, void *caller)
{
	struct trace_ctx *tmp;
#ifndef MULTI_THREAD
	tmp = &ctx;
#else
	tmp = (struct trace_ctx *) pthread_getspecific(priv_key);

	if (!tmp) {
		pthread_once(&thread_once, trace_make_key);
		tmp = calloc(1, sizeof(*tmp));
		pthread_setspecific(priv_key, tmp);
	}
	tmp->id = pthread_self();
#endif /* MULTI_THREAD */
        gettimeofday(&tmp->enter_time, NULL);
}

#ifdef PRINT_NAMES
#define __GNU_SOURCE
#define __USE_GNU
#include <dlfcn.h>
#endif /* PRINT_NAMES */

static inline void __skip__ trace_exit(struct trace_ctx *ctx)
{
#ifdef PRINT_NAMES
        logp("(tc) %ld.%ld %.06lf %s %s %p\n",
	     start__.tv_sec, start__.tv_usec,
             ((double) stop.tv_sec +
             (double) stop.tv_usec * 1.0e-6) -
             ((double) start__.tv_sec +
             (double) start__.tv_usec * 1.0e-6),
	     ctx->func_name, ctx->caller_name, ctx->id);
#else /* PRINT_NAMES */
        logp("(tc) %ld.%ld %.06lf %p %p %p\n",
	     ctx->enter_time.tv_sec, ctx->enter_time.tv_usec,
             ((double) ctx->exit_time.tv_sec +
             (double) ctx->exit_time.tv_usec * 1.0e-6) -
             ((double) ctx->enter_time.tv_sec +
             (double) ctx->enter_time.tv_usec * 1.0e-6),
	     ctx->func, ctx->caller, ctx->id);
#endif /* PRINT_NAMES */
}

void __cyg_profile_func_exit(void *func, void *caller)
{
	struct trace_ctx *tmp;

#ifdef MULTI_THREAD
	tmp = (struct trace_ctx *) pthread_getspecific(priv_key);
#else /* MULTI_THREAD */
	tmp = &ctx;
#endif /* MULTI_THREAD */
        gettimeofday(&tmp->exit_time, NULL);
	tmp->func = func;
	tmp->caller = caller;

#ifndef PRINT_NAMES
	tmp->func_name = NULL;
	tmp->caller_name = NULL;
#else /* PRINT_NAMES */
	Dl_info func_info, caller_info;

	dladdr(func, &func_info);
	dladdr(caller, &caller_info);
	tmp->func_name = func_info.dli_sname;
	tmp->caller_name = caller_info.dli_sname;
#if 0
	logp("%p %s -> %p %s\n", caller, caller_info.dli_sname,
	     func, func_info.dli_sname);
#endif
#endif /* PRINT_NAMES */
	trace_exit(tmp);
}
