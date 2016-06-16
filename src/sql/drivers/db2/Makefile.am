SQL_DIRS += src/sql/drivers/db2

SQL_PUBLIC_INCLUDES += \
	QDB2Driver \
	QDB2Result


SQL_DB2_INCLUDES = \
	qsql_db2.h \
	qdb2driver.h \
	qdb2result.h

#SQL_DB2_PRIVATE_INCLUDES =

SQL_INCLUDES += $(SQL_DB2_INCLUDES)
#SQL_PRIVATE_INCLUDES += $(SQL_DB2_PRIVATE_INCLUDES)

noinst_HEADERS+=$(patsubst %,src/sql/drivers/db2/%,$(SQL_DB2_INCLUDES)) \
		$(patsubst %,src/sql/drivers/db2/%,$(SQL_DB2_PRIVATE_INCLUDES))

#lib_libCsSql@BUILD_ABI@_la_SOURCES += \
#	src/sql/drivers/db2/qsql_db2.cpp
