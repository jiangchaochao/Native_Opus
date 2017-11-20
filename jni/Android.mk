LOCAL_PATH :=$(call my-dir)
include $(CLEAR_VARS)
LOCAL_LDLIBS:=-L$(SYSROOT)/usr/lib -llog
LOCAL_MODULE := opus

include $(LOCAL_PATH)/celt_sources.mk
include $(LOCAL_PATH)/opus_sources.mk
include $(LOCAL_PATH)/silk_sources.mk

LOCAL_SRC_FILES := $(CELT_SOURCES) \
				   $(OPUS_SOURCES) \
				   $(OPUS_SOURCES_FLOAT) \
				   $(SILK_SOURCES) \
				   $(SILK_SOURCES_FIXED) \
				   $(LOCAL_PATH)/src/opus_native.c

LOCAL_C_INCLUDES := \
				   $(LOCAL_PATH)/include \
				   $(LOCAL_PATH)/src \
				   $(LOCAL_PATH)/celt \
				   $(LOCAL_PATH)/silk \
				   $(LOCAL_PATH)/silk/fixed \
				   $(LOCAL_PATH)/silk/float \
				   $(LOCAL_PATH)/silk/arm \
				   $(LOCAL_PATH)/silk/fixed/mips \
				   $(LOCAL_PATH)/silk/mips \
				   $(LOCAL_PATH)/silk/x86 \
				   $(LOCAL_PATH)/tests \
				   $(LOCAL_PATH)/win32 

LOCAL_CFLAGS := -DUSE_ALLOCA  -DOPUS_BUILD  -DFIXED_POINT #-DDISABLE_FLOAT_API
include $(BUILD_SHARED_LIBRARY)

