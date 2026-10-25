/* Minimal GLib shim */
#pragma once
#include <assert.h>
#include <alloca.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#define GLIB_CHECK_VERSION(a,b,c) ((a) < 2 || ((a) == 2 && (b) <= 29))
#define TRUE 1
#define FALSE 0
#define GLIB_MAJOR_VERSION 2
#define GLIB_MINOR_VERSION 29
#define G_BYTE_ORDER __BYTE_ORDER__
#define G_BIG_ENDIAN __ORDER_BIG_ENDIAN__
#define GINT16_FROM_LE(v) (v)
#define GINT32_FROM_LE(v) (v)
#define G_LIKELY(v) (v)
#define G_UNLIKELY(v) (v)
typedef void *gpointer;
typedef int gint;
typedef int64_t gint64;
typedef struct { int code; const char *message; } GError;
#define g_new(t,n) ((t *)malloc(sizeof(t)*(n)))
#define g_newa(t,n) ((t *)alloca(sizeof(t)*(n)))
#define g_alloca alloca
#define g_free free
#define g_assert assert
#define g_return_val_if_fail(e,v) do { if (!(e)) return (v); } while (0)
#define g_clear_error(e) do { *(e) = NULL; } while (0)
#define g_vsnprintf vsnprintf
#define g_snprintf snprintf
#define g_usleep usleep
#define g_thread_supported() 1
#define g_thread_init(p) ((void)0)
#define G_FILE_TEST_EXISTS 1
#define G_FILE_TEST_IS_REGULAR 2

static inline int g_file_test(const char *path, int flags)
{
    struct stat st;
    if (stat(path, &st)) return 0;
    return (flags & G_FILE_TEST_EXISTS) || ((flags & G_FILE_TEST_IS_REGULAR) && S_ISREG(st.st_mode));
}

static inline int64_t g_get_monotonic_time(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (int64_t)ts.tv_sec*1000000 + ts.tv_nsec/1000;
}

typedef struct timeval GTimeVal;

#define g_get_current_time(t) gettimeofday((t), NULL)

typedef pthread_mutex_t GStaticMutex;
typedef pthread_mutex_t GStaticRecMutex;
typedef pthread_mutex_t GMutex;

#define G_STATIC_MUTEX_INIT PTHREAD_MUTEX_INITIALIZER
#define g_static_mutex_init(m) pthread_mutex_init((m), NULL)
#define g_static_mutex_free pthread_mutex_destroy
#define g_static_mutex_lock pthread_mutex_lock
#define g_static_mutex_unlock pthread_mutex_unlock

static inline void g_static_rec_mutex_init(GStaticRecMutex *m)
{
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(m, &attr);
    pthread_mutexattr_destroy(&attr);
}

#define g_static_rec_mutex_free pthread_mutex_destroy
#define g_static_rec_mutex_lock pthread_mutex_lock
#define g_static_rec_mutex_unlock pthread_mutex_unlock
#define g_mutex_lock pthread_mutex_lock
#define g_mutex_unlock pthread_mutex_unlock

static inline GMutex *g_mutex_new(void)
{
    GMutex *m = g_new(GMutex, 1);
    if (m) pthread_mutex_init(m, NULL);
    return m;
}

static inline void g_mutex_free(GMutex *m) { if (m) { pthread_mutex_destroy(m); free(m); } }

typedef pthread_cond_t GCond;

static inline GCond *g_cond_new(void)
{
    GCond *c = g_new(GCond, 1);
    if (c) pthread_cond_init(c, NULL);
    return c;
}

static inline void g_cond_free(GCond *c) { if (c) { pthread_cond_destroy(c); free(c); } }
#define g_cond_signal pthread_cond_signal
#define g_cond_broadcast pthread_cond_broadcast
#define g_cond_wait pthread_cond_wait
typedef pthread_key_t GStaticPrivate;
#define g_static_private_init(p) pthread_key_create((p), NULL)
#define g_static_private_get(p) pthread_getspecific(*(p))
#define g_static_private_set(p,d,f) pthread_setspecific(*(p), (d))
#define g_static_private_free(p) pthread_key_delete(*(p))
#define g_atomic_int_inc(p) ((void)__atomic_fetch_add((p), 1, __ATOMIC_SEQ_CST))
#define g_atomic_int_get(p) __atomic_load_n((p), __ATOMIC_SEQ_CST)
#define g_atomic_int_set(p,v) __atomic_store_n((p), (v), __ATOMIC_SEQ_CST)
#define g_atomic_int_dec_and_test(p) (__atomic_sub_fetch((p), 1, __ATOMIC_SEQ_CST) == 0)
#define g_atomic_int_exchange_and_add(p,v) __atomic_fetch_add((p), (v), __ATOMIC_SEQ_CST)
#define g_atomic_int_add g_atomic_int_exchange_and_add
#define g_atomic_int_compare_and_exchange(p,o,n) __sync_bool_compare_and_swap((p),(o),(n))
#define g_atomic_pointer_get(p) __atomic_load_n((p), __ATOMIC_SEQ_CST)
#define g_atomic_pointer_set(p,v) __atomic_store_n((p),(v), __ATOMIC_SEQ_CST)
#define g_atomic_pointer_compare_and_exchange g_atomic_int_compare_and_exchange

typedef void *(*GThreadFunc)(void *);
typedef struct { pthread_t thread; GThreadFunc fn; void *data; int joinable; } GThread;

static inline void *retro_fluid_thread(void *ptr)
{
    GThread *t = (GThread *)ptr;
    GThreadFunc fn = t->fn;
    void *data = t->data;
    int joinable = t->joinable;
    void *result = fn(data);
    if (!joinable) free(t);
    return result;
}

static inline GThread *g_thread_create(GThreadFunc fn, void *data, int joinable, GError **error)
{
    GThread *t = g_new(GThread, 1);
    pthread_attr_t attr;
    int result;
    if (!t) return NULL;
    t->fn = fn; t->data = data; t->joinable = joinable;
    pthread_attr_init(&attr);
    if (!joinable) pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    result = pthread_create(&t->thread, &attr, retro_fluid_thread, t);
    pthread_attr_destroy(&attr);
    if (result) { free(t); if (error) *error = NULL; return NULL; }
    return t;
}

static inline void *g_thread_join(GThread *t)
{
    void *result = NULL;
    pthread_join(t->thread, &result);
    free(t);
    return result;
}

static inline void g_strfreev(char **v)
{
    char **p;
    if (!v) return;
    for (p=v; *p; ++p) free(*p);
    free(v);
}
