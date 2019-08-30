MULTIMEDIA_DIRS += src/multimedia/audio

MULTIMEDIA_PUBLIC_INCLUDES += \
	QAbstractAudioDeviceInfo \
	QAbstractAudioInput \
	QAbstractAudioOutput \
	QAudio \
	QAudioFormat \
	QAudioInput \
	QAudioOutput \
	QAudioDeviceInfo \
	QAudioEngineFactoryInterface \
	QSound \
	QSoundEffect

MULTIMEDIA_AUDIO_INCLUDES = \
	qabstractaudiodeviceinfo.h \
	qabstractaudioinput.h \
	qabstractaudiooutput.h \
	qaudio.h \
	qaudiobuffer.h \
	qaudiodecoder.h \
	qaudiodeviceinfo.h \
	qaudioformat.h \
	qaudioinput.h \
	qaudiooutput.h \
	qaudioprobe.h \
	qaudioenginefactoryinterface.h \
	qaudiosystem.h \
	qaudiosystemplugin.h \
	qsound.h \
	qsoundeffect.h

MULTIMEDIA_AUDIO_PRIVATE_INCLUDES = \
	qaudiobuffer_p.h \
	qaudiodevicefactory_p.h \
	qaudiohelpers_p.h \
	qsamplecache_p.h \
	qsoundeffect_qaudio_p.h \
	qsoundeffect_pulse_p.h \
	qwavedecoder_p.h

MULTIMEDIA_INCLUDES += $(MULTIMEDIA_AUDIO_INCLUDES)
MULTIMEDIA_PRIVATE_INCLUDES += $(MULTIMEDIA_AUDIO_PRIVATE_INCLUDES)

noinst_HEADERS+=$(patsubst %,src/multimedia/audio/%,$(MULTIMEDIA_AUDIO_INCLUDES)) \
		$(patsubst %,src/multimedia/audio/%,$(MULTIMEDIA_AUDIO_PRIVATE_INCLUDES))

lib_libCsMultimedia@BUILD_ABI@_la_SOURCES += \
	src/multimedia/audio/qaudio.cpp \
	src/multimedia/audio/qaudiobuffer.cpp \
	src/multimedia/audio/qaudiodecoder.cpp \
	src/multimedia/audio/qaudiodevicefactory.cpp \
	src/multimedia/audio/qaudiodeviceinfo.cpp \
	src/multimedia/audio/qaudioformat.cpp \
	src/multimedia/audio/qaudiohelpers.cpp \
	src/multimedia/audio/qaudioinput.cpp \
	src/multimedia/audio/qaudiooutput.cpp \
	src/multimedia/audio/qaudioprobe.cpp \
	src/multimedia/audio/qaudiosystem.cpp \
	src/multimedia/audio/qaudiosystemplugin.cpp \
	src/multimedia/audio/qsamplecache_p.cpp \
	src/multimedia/audio/qsound.cpp \
	src/multimedia/audio/qsoundeffect.cpp \
	src/multimedia/audio/qwavedecoder_p.cpp


if PULSEAUDIO

lib_libCsMultimedia@BUILD_ABI@_la_CXXFLAGS +=\
   -DQT_MULTIMEDIA_PULSEAUDIO \
   -DQTM_PULSEAUDIO_DEFAULTBUFFER

lib_libCsMultimedia@BUILD_ABI@_la_LIBADD += ${PULSEAUDIO_LIBS}

lib_libCsMultimedia@BUILD_ABI@_la_SOURCES += \
	src/multimedia/audio/qsoundeffect_pulse_p.cpp

else

lib_libCsMultimedia@BUILD_ABI@_la_CXXFLAGS +=\
   -DQT_MULTIMEDIA_QAUDIO

lib_libCsMultimedia@BUILD_ABI@_la_SOURCES += \
	src/multimedia/audio/qsoundeffect_qaudio_p.cpp

endif


if OSTYPE_WIN
lib_libCsMultimedia@BUILD_ABI@_la_LIBADD+= -lwinmm
endif

if OSTYPE_DARWIN
lib_libCsMultimedia@BUILD_ABI@_la_LDFLAGS += \
   -framework AudioUnit \
   -framework CoreAudio \
   -framework AudioToolbox
endif




