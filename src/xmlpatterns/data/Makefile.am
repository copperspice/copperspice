XMLPATTERNS_DIRS += src/xmlpatterns/data

# XMLPATTERNS_PUBLIC_INCLUDES +=

#XMLPATTERNS_DATA_INCLUDES =


XMLPATTERNS_DATA_PRIVATE_INCLUDES = \
	qabstractdatetime_p.h              \
	qabstractduration_p.h              \
	qabstractfloat_p.h                 \
	qabstractfloatcasters_p.h          \
	qabstractfloatmathematician_p.h    \
	qanyuri_p.h                        \
	qatomiccaster_p.h                  \
	qatomiccasters_p.h                 \
	qatomiccomparator_p.h              \
	qatomiccomparators_p.h             \
	qatomicmathematician_p.h           \
	qatomicmathematicians_p.h          \
	qatomicstring_p.h                  \
	qbase64binary_p.h                  \
	qboolean_p.h                       \
	qcommonvalues_p.h                  \
	qcomparisonfactory_p.h             \
	qdate_p.h                          \
	qdaytimeduration_p.h               \
	qdecimal_p.h                       \
	qderivedinteger_p.h                \
	qderivedstring_p.h                 \
	qduration_p.h                      \
	qgday_p.h                          \
	qgmonth_p.h                        \
	qgmonthday_p.h                     \
	qgyear_p.h                         \
	qgyearmonth_p.h                    \
	qhexbinary_p.h                     \
	qinteger_p.h                       \
	qitem_p.h                          \
	qnodebuilder_p.h                   \
	qqnamevalue_p.h                    \
	qresourceloader_p.h                \
	qschemadatetime_p.h                \
	qschemanumeric_p.h                 \
	qschematime_p.h                    \
	quntypedatomic_p.h                 \
	qvalidationerror_p.h               \
	qvaluefactory_p.h                  \
	qyearmonthduration_p.h \
	qabstractfloat.cpp \
	qsorttuple_p.h \
	qabstractfloatcasters.cpp \
	qabstractfloatmathematician.cpp

XMLPATTERNS_INCLUDES += $(XMLPATTERNS_DATA_INCLUDES)
XMLPATTERNS_PRIVATE_INCLUDES += $(XMLPATTERNS_DATA_PRIVATE_INCLUDES)

noinst_HEADERS+=$(patsubst %,src/xmlpatterns/data/%,$(XMLPATTERNS_DATA_INCLUDES)) \
		$(patsubst %,src/xmlpatterns/data/%,$(XMLPATTERNS_DATA_PRIVATE_INCLUDES))

lib_libCsXmlPatterns@BUILD_ABI@_la_SOURCES += \
	src/xmlpatterns/data/qabstractdatetime.cpp        \
	src/xmlpatterns/data/qabstractduration.cpp        \
	src/xmlpatterns/data/qanyuri.cpp                  \
	src/xmlpatterns/data/qatomiccaster.cpp            \
	src/xmlpatterns/data/qatomiccasters.cpp           \
	src/xmlpatterns/data/qatomiccomparator.cpp        \
	src/xmlpatterns/data/qatomiccomparators.cpp       \
	src/xmlpatterns/data/qatomicmathematician.cpp     \
	src/xmlpatterns/data/qatomicmathematicians.cpp    \
	src/xmlpatterns/data/qatomicstring.cpp            \
	src/xmlpatterns/data/qatomicvalue.cpp             \
	src/xmlpatterns/data/qbase64binary.cpp            \
	src/xmlpatterns/data/qboolean.cpp                 \
	src/xmlpatterns/data/qcommonvalues.cpp            \
	src/xmlpatterns/data/qcomparisonfactory.cpp       \
	src/xmlpatterns/data/qdate.cpp                    \
	src/xmlpatterns/data/qdaytimeduration.cpp         \
	src/xmlpatterns/data/qdecimal.cpp                 \
	src/xmlpatterns/data/qduration.cpp                \
	src/xmlpatterns/data/qgday.cpp                    \
	src/xmlpatterns/data/qgmonth.cpp                  \
	src/xmlpatterns/data/qgmonthday.cpp               \
	src/xmlpatterns/data/qgyear.cpp                   \
	src/xmlpatterns/data/qgyearmonth.cpp              \
	src/xmlpatterns/data/qhexbinary.cpp               \
	src/xmlpatterns/data/qinteger.cpp                 \
	src/xmlpatterns/data/qitem.cpp                    \
	src/xmlpatterns/data/qnodebuilder.cpp             \
	src/xmlpatterns/data/qnodemodel.cpp               \
	src/xmlpatterns/data/qqnamevalue.cpp              \
	src/xmlpatterns/data/qresourceloader.cpp          \
	src/xmlpatterns/data/qschemadatetime.cpp          \
	src/xmlpatterns/data/qschemanumeric.cpp           \
	src/xmlpatterns/data/qschematime.cpp              \
	src/xmlpatterns/data/qsorttuple.cpp               \
	src/xmlpatterns/data/quntypedatomic.cpp           \
	src/xmlpatterns/data/qvalidationerror.cpp         \
	src/xmlpatterns/data/qvaluefactory.cpp            \
	src/xmlpatterns/data/qyearmonthduration.cpp