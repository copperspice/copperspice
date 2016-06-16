XMLPATTERNS_DIRS += src/xmlpatterns/acceltree

#XMLPATTERNS_PUBLIC_INCLUDES +=

#XMLPATTERNS_ACCELTREE_INCLUDES =

XMLPATTERNS_ACCELTREE_PRIVATE_INCLUDES = \
	qacceltreebuilder_p.h \
	qacceltree_p.h \
	qacceltreeresourceloader_p.h \
	qacceliterators_p.h \
	qcompressedwhitespace_p.h \
	qacceltreebuilder.cpp

#XMLPATTERNS_INCLUDES += $(XMLPATTERNS_ACCELTREE_INCLUDES)
XMLPATTERNS_PRIVATE_INCLUDES += $(XMLPATTERNS_ACCELTREE_PRIVATE_INCLUDES)

noinst_HEADERS+=$(patsubst %,src/xmlpatterns/acceltree/%,$(XMLPATTERNS_ACCELTREE_INCLUDES)) \
		$(patsubst %,src/xmlpatterns/acceltree/%,$(XMLPATTERNS_ACCELTREE_PRIVATE_INCLUDES))

lib_libCsXmlPatterns@BUILD_ABI@_la_SOURCES += \
	src/xmlpatterns/acceltree/qacceltree.cpp \
	src/xmlpatterns/acceltree/qacceltreeresourceloader.cpp \
	src/xmlpatterns/acceltree/qacceliterators.cpp \
	src/xmlpatterns/acceltree/qcompressedwhitespace.cpp
