MULTIMEDIA_DIRS += src/multimedia/kernel

MULTIMEDIA_PUBLIC_INCLUDES += \
   QMediaBindableInterface \
   QMediaControl \
   QMediaObject \
   QMediaMetaData \
   QMediaService \
   QMediaTimeRange \
   QMultimedia \
   QtMultimedia

MULTIMEDIA_KERNEL_INCLUDES = \
	qmediabindableinterface.h \
	qmediacontrol.h \
	qmediaenumdebug.h \
	qmediaobject.h \
	qmultimedia.h \
	qmediametadata.h \
	qmediaservice.h \
	qmediaservice_provider_plugin.h \
	qmediatimerange.h \
	qtmultimedia.h

MULTIMEDIA_KERNEL_PRIVATE_INCLUDES = \
	qmediacontrol_p.h \
	qmediaobject_p.h \
	qmediaopenglhelper_p.h \
	qmediaresourceset_p.h \
	qmediaservice_p.h \
	qmediaserviceprovider_p.h \
	qmediaresourcepolicy_p.h \
	qmediaresourcepolicyplugin_p.h \
	qmultimediautils_p.h

MULTIMEDIA_INCLUDES += $(MULTIMEDIA_KERNEL_INCLUDES)
MULTIMEDIA_PRIVATE_INCLUDES += $(MULTIMEDIA_KERNEL_PRIVATE_INCLUDES)

noinst_HEADERS+=$(patsubst %,src/multimedia/kernel/%,$(MULTIMEDIA_KERNEL_INCLUDES)) \
		$(patsubst %,src/multimedia/kernel/%,$(MULTIMEDIA_KERNEL_PRIVATE_INCLUDES))

lib_libCsMultimedia@BUILD_ABI@_la_SOURCES += \
	src/multimedia/kernel/qmediabindableinterface.cpp \
	src/multimedia/kernel/qmediacontrol.cpp \
	src/multimedia/kernel/qmediaobject.cpp \
	src/multimedia/kernel/qmediametadata.cpp \
	src/multimedia/kernel/qmediaresourceset.cpp \
	src/multimedia/kernel/qmediaservice.cpp \
	src/multimedia/kernel/qmediaserviceprovider.cpp \
	src/multimedia/kernel/qmediatimerange.cpp \
	src/multimedia/kernel/qmediaresourcepolicy.cpp \
	src/multimedia/kernel/qmediaresourcepolicyplugin.cpp \
	src/multimedia/kernel/qmultimediautils.cpp


