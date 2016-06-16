XMLPATTERNS_DIRS += src/xmlpatterns/janitors

#XMLPATTERNS_PUBLIC_INCLUDES +=

#XMLPATTERNS_JANITORS_INCLUDES =

XMLPATTERNS_JANITORS_PRIVATE_INCLUDES = \
	qargumentconverter_p.h     \
	qatomizer_p.h              \
	qcardinalityverifier_p.h   \
	qebvextractor_p.h          \
	qitemverifier_p.h          \
	quntypedatomicconverter_p.h

XMLPATTERNS_INCLUDES += $(XMLPATTERNS_JANITORS_INCLUDES)
XMLPATTERNS_PRIVATE_INCLUDES += $(XMLPATTERNS_JANITORS_PRIVATE_INCLUDES)

noinst_HEADERS+=$(patsubst %,src/xmlpatterns/janitors/%,$(XMLPATTERNS_JANITORS_INCLUDES)) \
		$(patsubst %,src/xmlpatterns/janitors/%,$(XMLPATTERNS_JANITORS_PRIVATE_INCLUDES))

lib_libCsXmlPatterns@BUILD_ABI@_la_SOURCES += \
	src/xmlpatterns/janitors/qargumentconverter.cpp     \
	src/xmlpatterns/janitors/qatomizer.cpp              \
	src/xmlpatterns/janitors/qcardinalityverifier.cpp   \
	src/xmlpatterns/janitors/qebvextractor.cpp          \
	src/xmlpatterns/janitors/qitemverifier.cpp          \
	src/xmlpatterns/janitors/quntypedatomicconverter.cpp