MULTIMEDIA_DIRS += src/multimedia/radio

MULTIMEDIA_PUBLIC_INCLUDES +=

MULTIMEDIA_RADIO_INCLUDES = \
	qradiodata.h \
	qradiotuner.h

MULTIMEDIA_RADIO_PRIVATE_INCLUDES =	

MULTIMEDIA_INCLUDES += $(MULTIMEDIA_RADIO_INCLUDES)
MULTIMEDIA_PRIVATE_INCLUDES += $(MULTIMEDIA_RADIO_PRIVATE_INCLUDES)

noinst_HEADERS+=$(patsubst %,src/multimedia/radio/%,$(MULTIMEDIA_RADIO_INCLUDES)) \
		$(patsubst %,src/multimedia/radio/%,$(MULTIMEDIA_RADIO_PRIVATE_INCLUDES))

lib_libCsMultimedia@BUILD_ABI@_la_SOURCES += \
	src/multimedia/radio/qradiodata.cpp \
	src/multimedia/radio/qradiotuner.cpp