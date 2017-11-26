
CORE_DIRS += src/core/global

CORE_PUBLIC_INCLUDES += \
	Q_INT16 \
	Q_INT32 \
	Q_INT64 \
	Q_INT8 \
	Q_LLONG \
	Q_LONG \
	Q_UINT16 \
	Q_UINT32 \
	Q_UINT64 \
	Q_UINT8 \
	Q_ULLONG \
	Q_ULONG \
	QAssert \
	QCOORD \
	QFlag \
	QFlags \
	QGlobal \
	QGlobalStatic \
	QGlobalStaticDeleter \
	QIncompatibleFlag \
	QIntegerForSize \
	QInternal \
	QLibraryInfo \
	QLog \
	QNoImplicitBoolCast \
	QSysInfo \
	QTypeInfo \
	Qt \
	QtCore \
	QtEndian \
	QtGlobal \
	QtNamespace

CORE_GLOBAL_INCLUDES = \
	cs_build_info.h \
	q_int16.h \
	q_int32.h \
	q_int64.h \
	q_int8.h \
	q_llong.h \
	q_long.h \
	q_uint16.h \
	q_uint32.h \
	q_uint64.h \
	q_uint8.h \
	q_ullong.h \
	q_ulong.h \
	qassert.h \
	qcoord.h \
	qendian.h \
	qexport.h \
	qfeatures.h \
	qflag.h \
	qflags.h \
	qglobal.h \
	qglobalstatic.h \
	qglobalstaticdeleter.h \
	qincompatibleflag.h \
	qintegerforsize.h \
	qinternal.h \
	qlibraryinfo.h \
	qlog.h \
	qnamespace.h \
	qnoimplicitboolcast.h \
	qnumeric.h \
	qplatformdefs.h \
	qplatformposix.h \
	qsysinfo.h \
	qt.h \
	qtcore.h \
	qtendian.h \
	qtglobal.h \
	qtnamespace.h \
	qtypeinfo.h \
	qt_windows.h

CORE_GLOBAL_PRIVATE_INCLUDES = \
	qnumeric_p.h

CORE_INCLUDES += $(CORE_GLOBAL_INCLUDES)
CORE_PRIVATE_INCLUDES += $(CORE_GLOBAL_PRIVATE_INCLUDES)

noinst_HEADERS+=$(patsubst %,src/core/global/%,$(CORE_GLOBAL_INCLUDES)) \
		$(patsubst %,src/core/global/%,$(CORE_GLOBAL_PRIVATE_INCLUDES))

lib_libCsCore@BUILD_ABI@_la_SOURCES += \
	src/core/global/qassert.cpp \
	src/core/global/qexport.cpp \
	src/core/global/qglobal.cpp \
	src/core/global/qlibraryinfo.cpp \
	src/core/global/qlog.cpp \
	src/core/global/qmalloc.cpp \
	src/core/global/qnumeric.cpp \
	src/core/global/qnamespace.cpp



