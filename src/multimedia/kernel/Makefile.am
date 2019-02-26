MULTIMEDIA_DIRS += src/multimedia/kernel

MULTIMEDIA_PUBLIC_INCLUDES += \
	QtMultimedia

MULTIMEDIA_KERNEL_INCLUDES = \
	qtmultimedia.h \
	qmultimedia.h \
	qmediaservice_provider_plugin.h

MULTIMEDIA_KERNEL_PRIVATE_INCLUDES = \
	qmediapluginloader_p.h

MULTIMEDIA_INCLUDES += $(MULTIMEDIA_KERNEL_INCLUDES)
MULTIMEDIA_PRIVATE_INCLUDES += $(MULTIMEDIA_KERNEL_PRIVATE_INCLUDES)

noinst_HEADERS+=$(patsubst %,src/multimedia/kernel/%,$(MULTIMEDIA_KERNEL_INCLUDES)) \
		$(patsubst %,src/multimedia/kernel/%,$(MULTIMEDIA_KERNEL_PRIVATE_INCLUDES))

lib_libCsMultimedia@BUILD_ABI@_la_SOURCES += \
	src/multimedia/kernel/qmediapluginloader.cpp

