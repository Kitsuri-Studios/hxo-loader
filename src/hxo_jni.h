/*-----------(START OF LICENSE NOTICE)-----------*/
/*
 * This file is part of HXO-loader.
 *
 * HXO-loader is licensed under the GNU General Public License v3.0
 * (GPL-3.0). You may copy, modify, and distribute it under the terms of the
 * GPL-3.0. This software is distributed WITHOUT ANY WARRANTY; see the GPL-3.0
 * for more details. You should have received a copy of the GPL-3.0 along
 * with this software. If not, see <https://www.gnu.org/licenses/>.
 *
 * Copyright (C) 2024 bitware
*/
/*-----------(END OF LICENSE NOTICE)-----------*/


#pragma once

#ifdef __ANDROID__

#include <jni.h>
#include <pthread.h>
#include <stdbool.h>

// Global JVM pointer (set during JNI_OnLoad)
static JavaVM *g_jvm = NULL;
static pthread_mutex_t jvm_mutex = PTHREAD_MUTEX_INITIALIZER;

// Structure to pass JVM info to modules
struct HXO_JNI_Context {
    JavaVM *jvm;                    // The JavaVM pointer
    jint jni_version;               // JNI version (e.g., JNI_VERSION_1_6)
    bool is_available;              // Whether JVM is available
    JNIEnv* (*attach_thread)(void); // Function to attach current thread
    void (*detach_thread)(void);    // Function to detach current thread
};

// Function to attach current thread to JVM
static inline JNIEnv* hxo_jni_attach_thread(void) {
    if (g_jvm == NULL) {
        return NULL;
    }

    JNIEnv *env = NULL;
    jint result = (*g_jvm)->GetEnv(g_jvm, (void**)&env, JNI_VERSION_1_6);

    if (result == JNI_EDETACHED) {
        // Thread not attached, attach it
        result = (*g_jvm)->AttachCurrentThread(g_jvm, &env, NULL);
        if (result != JNI_OK) {
            fprintf(stderr, "[!] Failed to attach thread to JVM\n");
            return NULL;
        }
    } else if (result != JNI_OK) {
        fprintf(stderr, "[!] Failed to get JNI environment\n");
        return NULL;
    }

    return env;
}

// Function to detach current thread from JVM
static inline void hxo_jni_detach_thread(void) {
    if (g_jvm == NULL) {
        return;
    }

    (*g_jvm)->DetachCurrentThread(g_jvm);
}

// Get JNI context for modules
static inline struct HXO_JNI_Context* hxo_jni_get_context(void) {
    static struct HXO_JNI_Context ctx;

    pthread_mutex_lock(&jvm_mutex);

    ctx.jvm = g_jvm;
    ctx.jni_version = (g_jvm != NULL) ? JNI_VERSION_1_6 : 0;
    ctx.is_available = (g_jvm != NULL);
    ctx.attach_thread = hxo_jni_attach_thread;
    ctx.detach_thread = hxo_jni_detach_thread;

    pthread_mutex_unlock(&jvm_mutex);

    return &ctx;
}

// Set JVM pointer for modules
static inline void hxo_jni_set_jvm(JavaVM *jvm) {
    pthread_mutex_lock(&jvm_mutex);
    g_jvm = jvm;
    pthread_mutex_unlock(&jvm_mutex);
}


JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *jvm, void *reserved) {
    (void)reserved; // Unused parameter

    fprintf(stdout, "[+] HXO JVM LOADED\n");

    // Save JVM pointer
    hxo_jni_set_jvm(jvm);

    JNIEnv *env = NULL;
    if ((*jvm)->GetEnv(jvm, (void**)&env, JNI_VERSION_1_6) != JNI_OK) {
        fprintf(stderr, "[!] Failed to get JNI environment in JNI_OnLoad\n");
        return JNI_ERR;
    }

    fprintf(stdout, "[+] HXO JVM pointer saved successfully\n");

    // Start the HXO loader in a separate thread
    pthread_attr_t ptattr;
    size_t stacksize = 4 * 1024 * 1024; // 4MB stack size

    pthread_attr_init(&ptattr);
    pthread_attr_setstacksize(&ptattr, stacksize);

    pthread_t loader_thread;
    int result = pthread_create(&loader_thread, &ptattr, (void*(*)(void*))hxo_loader, NULL);

    if (result != 0) {
        fprintf(stderr, "[!] Failed to create HXO loader thread\n");
        pthread_attr_destroy(&ptattr);
        return JNI_ERR;
    }

    pthread_attr_destroy(&ptattr);
    pthread_detach(loader_thread); // Detach so it cleans up automatically

    return JNI_VERSION_1_6;
}


JNIEXPORT void JNICALL JNI_OnUnload(JavaVM *jvm, void *reserved) {
    (void)jvm;
    (void)reserved;

    fprintf(stdout, "[+] HXO JVM UNLOADED\n");

    pthread_mutex_lock(&jvm_mutex);
    g_jvm = NULL;
    pthread_mutex_unlock(&jvm_mutex);
}

#endif // __ANDROID__