MULTIMEDIA_DIRS += src/multimedia/recording

MULTIMEDIA_PUBLIC_INCLUDES += \
	QAudioRecorder \
	QMediaRecorder

MULTIMEDIA_RECORDING_INCLUDES = \
	qaudiorecorder.h \
	qmediaencodersettings.h \
	qmediarecorder.h

MULTIMEDIA_RECORDING_PRIVATE_INCLUDES = \
	qmediarecorder_p.h

MULTIMEDIA_INCLUDES += $(MULTIMEDIA_RECORDING_INCLUDES)
MULTIMEDIA_PRIVATE_INCLUDES += $(MULTIMEDIA_RECORDING_PRIVATE_INCLUDES)

noinst_HEADERS+=$(patsubst %,src/multimedia/recording/%,$(MULTIMEDIA_RECORDING_INCLUDES)) \
		$(patsubst %,src/multimedia/recording/%,$(MULTIMEDIA_RECORDING_PRIVATE_INCLUDES))

lib_libCsMultimedia@BUILD_ABI@_la_SOURCES += \
	src/multimedia/recording/qaudiorecorder.cpp \
	src/multimedia/recording/qmediaencodersettings.cpp \
	src/multimedia/recording/qmediarecorder.cpp