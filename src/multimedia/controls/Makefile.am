MULTIMEDIA_DIRS += src/multimedia/controls

MULTIMEDIA_PUBLIC_INCLUDES += \
	QAudioRoleControl \
	QAudioInputSelectorControl \
	QAudioOutputSelectorControl \
	QMediaAvailabilityControl \
	QMediaNetworkAccessControl \
	QMediaPlayerControl \
	QMetaDataReaderControl \
	QVideoRendererControl \
	QVideoWindowControl

MULTIMEDIA_CONTROLS_INCLUDES = \
	qaudiodecodercontrol.h \
	qaudiorolecontrol.h \
	qaudioencodersettingscontrol.h \
	qaudioinputselectorcontrol.h \
	qaudiooutputselectorcontrol.h \
	qimageencodercontrol.h \
	qmediagaplessplaybackcontrol.h \
	qmediaaudioprobecontrol.h \
	qmediaavailabilitycontrol.h \
	qmedianetworkaccesscontrol.h \
	qmediaplayercontrol.h \
	qmediarecordercontrol.h \
	qmediastreamscontrol.h \
	qmetadatareadercontrol.h \
	qmetadatawritercontrol.h \
	qmediacontainercontrol.h \
	qmediavideoprobecontrol.h \
	qradiodatacontrol.h \
	qradiotunercontrol.h \
	qvideorenderercontrol.h \
	qvideowindowcontrol.h \
	qvideodeviceselectorcontrol.h \
	qvideoencodersettingscontrol.h

MULTIMEDIA_CONTROLS_PRIVATE_INCLUDES = \
	qmediaplaylistcontrol_p.h \
	qmediaplaylistsourcecontrol_p.h

MULTIMEDIA_INCLUDES += $(MULTIMEDIA_CONTROLS_INCLUDES)
MULTIMEDIA_PRIVATE_INCLUDES += $(MULTIMEDIA_CONTROLS_PRIVATE_INCLUDES)

noinst_HEADERS+=$(patsubst %,src/multimedia/controls/%,$(MULTIMEDIA_CONTROLS_INCLUDES)) \
		$(patsubst %,src/multimedia/controls/%,$(MULTIMEDIA_CONTROLS_PRIVATE_INCLUDES))

lib_libCsMultimedia@BUILD_ABI@_la_SOURCES += \
	src/multimedia/controls/qaudiodecodercontrol.cpp \
	src/multimedia/controls/qaudiorolecontrol.cpp \
	src/multimedia/controls/qaudioencodersettingscontrol.cpp \
	src/multimedia/controls/qaudioinputselectorcontrol.cpp \
	src/multimedia/controls/qimageencodercontrol.cpp \
	src/multimedia/controls/qmediagaplessplaybackcontrol.cpp \
	src/multimedia/controls/qaudiooutputselectorcontrol.cpp \
	src/multimedia/controls/qmediaaudioprobecontrol.cpp \
	src/multimedia/controls/qmediaavailabilitycontrol.cpp \
	src/multimedia/controls/qmedianetworkaccesscontrol.cpp \
	src/multimedia/controls/qmediaplayercontrol.cpp \
	src/multimedia/controls/qmediarecordercontrol.cpp \
	src/multimedia/controls/qmediastreamscontrol.cpp \
	src/multimedia/controls/qmediaplaylistcontrol.cpp \
	src/multimedia/controls/qmediaplaylistsourcecontrol.cpp \
	src/multimedia/controls/qmetadatareadercontrol.cpp \
	src/multimedia/controls/qmetadatawritercontrol.cpp \
	src/multimedia/controls/qmediacontainercontrol.cpp \
	src/multimedia/controls/qmediavideoprobecontrol.cpp \
	src/multimedia/controls/qradiodatacontrol.cpp \
	src/multimedia/controls/qradiotunercontrol.cpp \
	src/multimedia/controls/qvideorenderercontrol.cpp \
	src/multimedia/controls/qvideowindowcontrol.cpp \
	src/multimedia/controls/qvideodeviceselectorcontrol.cpp \
	src/multimedia/controls/qvideoencodersettingscontrol.cpp

