#pragma once
#include <stdint.h>

#ifdef __ANDROID__
#include <jni.h>

// Forward declare JNI context
struct HXO_JNI_Context;

#endif

struct HXOParam {
    const char *hxo_version; //The hxo version string
    int32_t PID;             //PID of current process
    char *baseName;          //Containing the elf executable name
    //or, In case of Android: The APP ID

    char *basePath;          //elf executable path
    char *moduleName;        //The hxo module's file name
    char *modulePath;        //the hxo module's absolute path

#ifdef __ANDROID__
    struct HXO_JNI_Context *jni_ctx; //JNI context (NULL if JVM not available)
#endif
};

//Function APIs and types
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define HXO_THREAD_FUNCTION void *(*) (void*)

static inline int hxo_cmpversion(const char *current_version, const char *needed_version) {
    int v1_major, v1_minor, v1_patch;
    int v2_major, v2_minor, v2_patch;

    // Parse the version strings
    sscanf(current_version, "%d.%d.%d", &v1_major, &v1_minor, &v1_patch);
    sscanf(needed_version, "%d.%d.%d", &v2_major, &v2_minor, &v2_patch);

    // Compare major version
    if (v1_major > v2_major) return 1;
    if (v1_major < v2_major) return -1;

    // Compare minor version
    if (v1_minor > v2_minor) return 1;
    if (v1_minor < v2_minor) return -1;

    // Compare patch version
    if (v1_patch > v2_patch) return 1;
    if (v1_patch < v2_patch) return -1;

    // Versions are equal
    return 0;
}

static inline int hxo_createthread(void *(*start_routine) (void *), void *arg)
{
    pthread_t thr = 0;
    return pthread_create(&thr, 0, start_routine, arg);
}

#ifdef __ANDROID__
// Helper macros for JNI usage in modules
#define HXO_JNI_AVAILABLE(param) ((param)->jni_ctx != NULL && (param)->jni_ctx->is_available)

#define HXO_JNI_ATTACH(param) \
    (HXO_JNI_AVAILABLE(param) ? (param)->jni_ctx->attach_thread() : NULL)

#define HXO_JNI_DETACH(param) \
    do { if (HXO_JNI_AVAILABLE(param)) (param)->jni_ctx->detach_thread(); } while(0)


#endif // __ANDROID__