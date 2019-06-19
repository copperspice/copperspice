SQL_DIRS += src/plugins/sqldrivers/oci

SQL_OCI_PUBLIC_INCLUDES = \
	QOCIDriver \
	QOCIResult

SQL_OCI_INCLUDES = \
	qsql_oci.h \
	qocidriver.h \
	qociresult.h

# SQL_OCI_PRIVATE_INCLUDES =

SQL_PUBLIC_INCLUDES += $(SQL_OCI_PUBLIC_INCLUDES)
SQL_INCLUDES += $(SQL_OCI_INCLUDES)
SQL_PRIVATE_INCLUDES += $(SQL_OCI_PRIVATE_INCLUDES)

noinst_HEADERS+=$(patsubst %,src/plugins/sqldrivers/oci/%,$(SQL_OCI_INCLUDES)) \
		$(patsubst %,src/plugins/sqldrivers/oci/%,$(SQL_OCI_PRIVATE_INCLUDES))

if FALSE




#lib_libCsSqlOci@BUILD_ABI@_la_SOURCES += \
	src/plugins/sqldrivers/oci/qsql_oci.cpp \
	src/plugins/sqldrivers/oci/main.cpp

endif
