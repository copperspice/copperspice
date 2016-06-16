XMLPATTERNS_DIRS += src/xmlpatterns/iterators

#XMLPATTERNS_PUBLIC_INCLUDES +=

#XMLPATTERNS_ITERATORS_INCLUDES =

XMLPATTERNS_ITERATORS_PRIVATE_INCLUDES = \
	qcachingiterator_p.h          \
	qdeduplicateiterator_p.h      \
	qdistinctiterator_p.h         \
	qemptyiterator_p.h            \
	qexceptiterator_p.h           \
	qindexofiterator_p.h          \
	qinsertioniterator_p.h        \
	qintersectiterator_p.h        \
	qitemmappingiterator_p.h      \
	qrangeiterator_p.h            \
	qremovaliterator_p.h          \
	qsequencemappingiterator_p.h  \
	qsingletoniterator_p.h        \
	qsubsequenceiterator_p.h      \
	qtocodepointsiterator_p.h     \
	qunioniterator_p.h
XMLPATTERNS_INCLUDES += $(XMLPATTERNS_ITERATORS_INCLUDES)
XMLPATTERNS_PRIVATE_INCLUDES += $(XMLPATTERNS_ITERATORS_PRIVATE_INCLUDES)

noinst_HEADERS+=$(patsubst %,src/xmlpatterns/iterators/%,$(XMLPATTERNS_ITERATORS_INCLUDES)) \
		$(patsubst %,src/xmlpatterns/iterators/%,$(XMLPATTERNS_ITERATORS_PRIVATE_INCLUDES))

lib_libCsXmlPatterns@BUILD_ABI@_la_SOURCES += \
	src/xmlpatterns/iterators/qcachingiterator.cpp        \
	src/xmlpatterns/iterators/qdeduplicateiterator.cpp    \
	src/xmlpatterns/iterators/qdistinctiterator.cpp       \
	src/xmlpatterns/iterators/qexceptiterator.cpp         \
	src/xmlpatterns/iterators/qindexofiterator.cpp        \
	src/xmlpatterns/iterators/qinsertioniterator.cpp      \
	src/xmlpatterns/iterators/qintersectiterator.cpp      \
	src/xmlpatterns/iterators/qrangeiterator.cpp          \
	src/xmlpatterns/iterators/qremovaliterator.cpp        \
	src/xmlpatterns/iterators/qsubsequenceiterator.cpp    \
	src/xmlpatterns/iterators/qtocodepointsiterator.cpp   \
	src/xmlpatterns/iterators/qunioniterator.cpp
