LOCAL_PATH := $(call my-dir)

# KnShim
include $(CLEAR_VARS)

LOCAL_ARM_MODE  := arm
LOCAL_MODULE    := YipLoader
LOCAL_SRC_FILES := yip/main.c \
	yip/util.c \
	yip/knshim.c \
	yip/loader.c \
	yip/jnistuff.c \
	yip/apkiter.c
LOCAL_LDLIBS     := -ldl -llog -landroid
LOCAL_STATIC_LIBRARIES := android_native_app_glue
LOCAL_C_INCLUDES := yip/extern
LOCAL_CFLAGS     := -DDUMMY

include $(BUILD_SHARED_LIBRARY)

$(call import-module,android/native_app_glue)
