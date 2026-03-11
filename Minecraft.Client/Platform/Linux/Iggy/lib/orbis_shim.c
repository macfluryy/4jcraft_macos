#define _GNU_SOURCE

#include <pthread.h>
#include <sys/time.h>
#include <unistd.h>
#include <math.h>
#include <errno.h>
#include <time.h>
#include <stdio.h>
#include <ctype.h>
#include <sched.h>
#include <stdlib.h>
#include <stdint.h>
#include <fenv.h>

#include "iggy_audio_shim.h"

_Static_assert(sizeof(pthread_t) == 8, "pthread_t must be 8 bytes");
_Static_assert(sizeof(int) == 4, "int must be 32 bit");
_Static_assert(sizeof(unsigned) == 4, "unsigned must be 32 bit");


/* ============================================================
 * CRT & Math Shims
 * ============================================================ */

double _Sin(double x) { return sin(x); }
double _Log(double x) { return log(x); }

// Map to the C fenv.h rounding modes
int _Fltrounds(void) {
    switch (fegetround()) {
        case FE_TOWARDZERO: return 0;
        case FE_TONEAREST:  return 1;
        case FE_DOWNWARD:   return 2;
        case FE_UPWARD:     return 3;
        default:            return 1;
    }
}

// BSD specific function supposed to return pointer to thread-local errno
#if defined(__GLIBC__)
int *__error(void) { return __errno_location(); }
#else
int *__error(void) { return &errno; }
#endif

// Microsoft specific variant of gmtime
int gmtime_s(struct tm *tmDest, const time_t *sourceTime) {
    if (!tmDest || !sourceTime) return EINVAL;
    if (gmtime_r(sourceTime, tmDest) == NULL) return EINVAL; /* conservative */
    return 0;
}

// Returns pointer to ctype classification table
const unsigned short *_Getpctype(void) {
#if defined(__GLIBC__)
    return (const unsigned short *)(*__ctype_b_loc());
#else
    // We might still be alright
    errno = ENOSYS;
    return NULL;
#endif
}

/* ============================================================
 * Helpers for heap-allocated opaque objects
 * ============================================================ */

static int alloc_and_init(void **out, size_t size, int (*init_fn)(void *), void (*free_fn)(void *)) {
    if (!out) return EINVAL;
    *out = NULL;

    void *p = malloc(size);
    if (!p) return ENOMEM;

    int rc = init_fn(p);
    if (rc != 0) {
        if (free_fn) free_fn(p);
        else free(p);
        return rc;
    }

    *out = p;
    return 0;
}

/* ============================================================
 * Pthread Shims (opaque handles allocated on heap)
 * ============================================================ */

// Mutex Attributes

static int mutexattr_init_thunk(void *p) {
    return pthread_mutexattr_init((pthread_mutexattr_t *)p);
}

int scePthreadMutexattrInit(void **attr) {
    return alloc_and_init(attr, sizeof(pthread_mutexattr_t), mutexattr_init_thunk, free);
}

int scePthreadMutexattrSettype(void **attr, int type) {
    if (!attr || !*attr) return EINVAL;
    return pthread_mutexattr_settype((pthread_mutexattr_t *)*attr, type);
}

int scePthreadMutexattrDestroy(void **attr) {
    if (!attr || !*attr) return EINVAL;
    pthread_mutexattr_t *a = (pthread_mutexattr_t *)*attr;

    int rc = pthread_mutexattr_destroy(a);
    if (rc == 0) {
        free(a);
        *attr = NULL;
    }
    return rc;
}

// Mutexes

int scePthreadMutexInit(void **mutex, void **attr, const char *name) {
    (void)name;

    if (!mutex) return EINVAL;
    *mutex = NULL;

    pthread_mutex_t *m = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
    if (!m) return ENOMEM;

    pthread_mutexattr_t *a = (attr && *attr) ? (pthread_mutexattr_t *)*attr : NULL;

    int rc = pthread_mutex_init(m, a);
    if (rc != 0) {
        free(m);
        return rc;
    }

    *mutex = m;
    return 0;
}

int scePthreadMutexLock(void **mutex) {
    if (!mutex || !*mutex) return EINVAL;
    return pthread_mutex_lock((pthread_mutex_t *)*mutex);
}

int scePthreadMutexTrylock(void **mutex) {
    if (!mutex || !*mutex) return EINVAL;
    return pthread_mutex_trylock((pthread_mutex_t *)*mutex);
}

int scePthreadMutexUnlock(void **mutex) {
    if (!mutex || !*mutex) return EINVAL;
    return pthread_mutex_unlock((pthread_mutex_t *)*mutex);
}

int scePthreadMutexTimedlock(void **mutex, unsigned int usecs) {
    if (!mutex || !*mutex) return EINVAL;

    struct timespec ts;

    // Get current wall clock time
    if (clock_gettime(CLOCK_REALTIME, &ts) == -1)
        return errno;

    // Add the relative microseconds to the current time
    ts.tv_sec += (usecs / 1000000);
    ts.tv_nsec += (usecs % 1000000) * 1000;

    // Handle nanosecond overflow
    if (ts.tv_nsec >= 1000000000L) {
        ts.tv_sec++;
        ts.tv_nsec -= 1000000000L;
    }

    // Call the standard POSIX function with the absolute time
    return pthread_mutex_timedlock(*mutex, &ts);
}

int scePthreadMutexDestroy(void **mutex) {
    if (!mutex || !*mutex) return EINVAL;

    pthread_mutex_t *m = (pthread_mutex_t *)*mutex;
    int rc = pthread_mutex_destroy(m);
    if (rc == 0) {
        free(m);
        *mutex = NULL;
    }
    return rc;
}

// Thread Attributes

static int attr_init_thunk(void *p) {
    return pthread_attr_init((pthread_attr_t *)p);
}

int scePthreadAttrInit(void **attr) {
    return alloc_and_init(attr, sizeof(pthread_attr_t), attr_init_thunk, free);
}

int scePthreadAttrSetstacksize(void **attr, size_t stacksize) {
    if (!attr || !*attr) return EINVAL;
    return pthread_attr_setstacksize((pthread_attr_t *)*attr, stacksize);
}

int scePthreadAttrDestroy(void **attr) {
    if (!attr || !*attr) return EINVAL;

    pthread_attr_t *a = (pthread_attr_t *)*attr;
    int rc = pthread_attr_destroy(a);
    if (rc == 0) {
        free(a);
        *attr = NULL;
    }
    return rc;
}

// Threads

int scePthreadCreate(pthread_t *thread_out, void **attr, void *(*start_routine)(void *), void *arg, const char *name) {
    (void)name;

    if (!thread_out || !start_routine) return EINVAL;
    pthread_attr_t *a = (attr && *attr) ? (pthread_attr_t *)*attr : NULL;
    return pthread_create(thread_out, a, start_routine, arg);
}

int scePthreadJoin(pthread_t thread, void **value_ptr) {
    return pthread_join(thread, value_ptr);
}

int scePthreadDetach(pthread_t thread) {
    return pthread_detach(thread);
}

pthread_t scePthreadSelf(void) {
    return pthread_self();
}

void scePthreadYield(void) {
    sched_yield();
}

int scePthreadSetschedparam(pthread_t thread, int policy, const struct sched_param *param) {
    if (!param) return EINVAL;
    return pthread_setschedparam(thread, policy, param);
}

int scePthreadGetschedparam(pthread_t thread, int *policy, struct sched_param *param) {
    if (!policy || !param) return EINVAL;
    return pthread_getschedparam(thread, policy, param);
}

// TLS Keys

int scePthreadKeyCreate(pthread_key_t *key, void (*destructor)(void *), const char *name) {
    (void)name;
    if (!key) return EINVAL;
    return pthread_key_create(key, destructor);
}

int scePthreadSetspecific(pthread_key_t key, const void *value) {
    return pthread_setspecific(key, value);
}

void *scePthreadGetspecific(pthread_key_t key) {
    return pthread_getspecific(key);
}

// Affinity

int scePthreadSetaffinity(pthread_t thread, uint64_t cpumask) {
    (void)thread;
    (void)cpumask;
    return 0; // Could return ENOSYS here, but this should be alright too
}

/* ============================================================
 * Kernel & Hardware Shims
 * ============================================================ */

int sceKernelGettimeofday(struct timeval *tp) {
    if (!tp) return EINVAL;
    return gettimeofday(tp, NULL);
}

int sceKernelUsleep(unsigned microseconds) {
    return usleep(microseconds);
}

/* ============================================================
 * Audio Shims
 * ============================================================ */

static IggyAudioShimCallbacks g_audioCbs = {};

int iggyAudioShimRegisterCallbacks(const IggyAudioShimCallbacks* callbacks) {
    if (callbacks) {
        g_audioCbs = *callbacks;
    } else {
        IggyAudioShimCallbacks cbs = {};
        g_audioCbs = cbs;
    }
    return IGGY_AUDIO_OK;
}

int sceAudioOutOpen(int user_id, enum IggyAudioOutPort port_type, int index, unsigned length, unsigned sample_rate, IggyAudioOutParamExtendedInformation param_type) {
    if (g_audioCbs.iggyAudioOutOpen)
        return g_audioCbs.iggyAudioOutOpen(user_id, port_type, index, length, sample_rate, param_type);
    return IGGY_AUDIO_OK;
}

int sceAudioOutSetVolume(int handle, int flag, int* vol) {
    if (g_audioCbs.iggyAudioOutSetVolume)
        return g_audioCbs.iggyAudioOutSetVolume(handle, flag, vol);
    return IGGY_AUDIO_OK;
}

int sceAudioOutClose(int handle) {
    if (g_audioCbs.iggyAudioOutClose)
        return g_audioCbs.iggyAudioOutClose(handle);
    return IGGY_AUDIO_OK;
}

int sceAudioOutOutput(int handle, void* ptr) {
    if (g_audioCbs.iggyAudioOutOutput)
        return g_audioCbs.iggyAudioOutOutput(handle, ptr);
    return IGGY_AUDIO_OK;
}

/* ============================================================
 * Misc
 * ============================================================ */

void gdraw_ps4_wait(void) {
    usleep(1000);
}
