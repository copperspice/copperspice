SQL_DIRS += src/sql/drivers/oci

SQL_PUBLIC_INCLUDES += \
	QOCIDriver \
	QOCIResult


SQL_OCI_INCLUDES = \
	qsql_oci.h \
	qocidriver.h \
	qociresult.h

#SQL_OCI_PRIVATE_INCLUDES =

SQL_INCLUDES += $(SQL_OCI_INCLUDES)
#SQL_PRIVATE_INCLUDES += $(SQL_OCI_PRIVATE_INCLUDES)

noinst_HEADERS+=$(patsubst %,src/sql/drivers/oci/%,$(SQL_OCI_INCLUDES)) \
		$(patsubst %,src/sql/drivers/oci/%,$(SQL_OCI_PRIVATE_INCLUDES))

#lib_libCsSql@BUILD_ABI@_la_SOURCES += \
#	src/sql/drivers/oci/qsql_oci.cpp
