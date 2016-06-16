XMLPATTERNS_DIRS += src/xmlpatterns/type

#XMLPATTERNS_PUBLIC_INCLUDES +=

#XMLPATTERNS_TYPE_INCLUDES =

XMLPATTERNS_TYPE_PRIVATE_INCLUDES = \
	qabstractnodetest_p.h              \
	qanyitemtype_p.h                   \
	qanynodetype_p.h                   \
	qanysimpletype_p.h                 \
	qanytype_p.h                       \
	qatomiccasterlocator_p.h           \
	qatomiccasterlocators_p.h          \
	qatomiccomparatorlocator_p.h       \
	qatomiccomparatorlocators_p.h      \
	qatomicmathematicianlocator_p.h    \
	qatomicmathematicianlocators_p.h   \
	qatomictypedispatch_p.h            \
	qatomictype_p.h                    \
	qbasictypesfactory_p.h             \
	qbuiltinatomictype_p.h             \
	qbuiltinatomictypes_p.h            \
	qbuiltinnodetype_p.h               \
	qbuiltintypes_p.h                  \
	qcardinality_p.h                   \
	qcommonsequencetypes_p.h           \
	qebvtype_p.h                       \
	qemptysequencetype_p.h             \
	qgenericsequencetype_p.h           \
	qitemtype_p.h                      \
	qlocalnametest_p.h                 \
	qmultiitemtype_p.h                 \
	qnamedschemacomponent_p.h          \
	qnamespacenametest_p.h             \
	qnonetype_p.h                      \
	qnumerictype_p.h                   \
	qprimitives_p.h                    \
	qqnametest_p.h                     \
	qschemacomponent_p.h               \
	qschematypefactory_p.h             \
	qschematype_p.h                    \
	qsequencetype_p.h                  \
	qtypechecker_p.h                   \
	quntyped_p.h                       \
	qxsltnodetest_p.h \
	qbuiltintypes.cpp \
	qbuiltinnodetype.cpp \
	qatomiccasterlocators.cpp \
	qatomiccomparatorlocators.cpp \
	qatomicmathematicianlocators.cpp

XMLPATTERNS_INCLUDES += $(XMLPATTERNS_TYPE_INCLUDES)
XMLPATTERNS_PRIVATE_INCLUDES += $(XMLPATTERNS_TYPE_PRIVATE_INCLUDES)

noinst_HEADERS+=$(patsubst %,src/xmlpatterns/type/%,$(XMLPATTERNS_TYPE_INCLUDES)) \
		$(patsubst %,src/xmlpatterns/type/%,$(XMLPATTERNS_TYPE_PRIVATE_INCLUDES))

lib_libCsXmlPatterns@BUILD_ABI@_la_SOURCES += \
	src/xmlpatterns/type/qabstractnodetest.cpp            \
	src/xmlpatterns/type/qanyitemtype.cpp                 \
	src/xmlpatterns/type/qanynodetype.cpp                 \
	src/xmlpatterns/type/qanysimpletype.cpp               \
	src/xmlpatterns/type/qanytype.cpp                     \
	src/xmlpatterns/type/qatomiccasterlocator.cpp         \
	src/xmlpatterns/type/qatomiccomparatorlocator.cpp     \
	src/xmlpatterns/type/qatomicmathematicianlocator.cpp  \
	src/xmlpatterns/type/qatomictype.cpp                  \
	src/xmlpatterns/type/qbasictypesfactory.cpp           \
	src/xmlpatterns/type/qbuiltinatomictype.cpp           \
	src/xmlpatterns/type/qbuiltinatomictypes.cpp          \
	src/xmlpatterns/type/qcardinality.cpp                 \
	src/xmlpatterns/type/qcommonsequencetypes.cpp         \
	src/xmlpatterns/type/qebvtype.cpp                     \
	src/xmlpatterns/type/qemptysequencetype.cpp           \
	src/xmlpatterns/type/qgenericsequencetype.cpp         \
	src/xmlpatterns/type/qitemtype.cpp                    \
	src/xmlpatterns/type/qlocalnametest.cpp               \
	src/xmlpatterns/type/qmultiitemtype.cpp               \
	src/xmlpatterns/type/qnamedschemacomponent.cpp        \
	src/xmlpatterns/type/qnamespacenametest.cpp           \
	src/xmlpatterns/type/qnonetype.cpp                    \
	src/xmlpatterns/type/qnumerictype.cpp                 \
	src/xmlpatterns/type/qqnametest.cpp                   \
	src/xmlpatterns/type/qschemacomponent.cpp             \
	src/xmlpatterns/type/qschematype.cpp                  \
	src/xmlpatterns/type/qschematypefactory.cpp           \
	src/xmlpatterns/type/qsequencetype.cpp                \
	src/xmlpatterns/type/qtypechecker.cpp                 \
	src/xmlpatterns/type/quntyped.cpp                     \
	src/xmlpatterns/type/qxsltnodetest.cpp
