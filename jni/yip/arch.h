#ifndef _YIPLOADER_ARCH_H_

#if defined(__arm__)
#define KN_ARCH_STRING "armeabi-v7a"
#elif defined(__aarch64__)
#define KN_ARCH_STRING "arm64-v8a"
#elif defined(__i386__)
#define KN_ARCH_STRING "x86"
#else
#define KN_ARCH_STRING "unknown"
#endif

#endif
