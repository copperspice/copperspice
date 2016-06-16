SQL_DIRS += src/sql/drivers/odbc

SQL_PUBLIC_INCLUDES += \
	QODBCDriver \
	QODBCResult


SQL_ODBC_INCLUDES = \
	qsql_odbc.h \
	qodbcdriver.h \
	qodbcresult.h

#SQL_ODBC_PRIVATE_INCLUDES =

SQL_INCLUDES += $(SQL_ODBC_INCLUDES)
#SQL_PRIVATE_INCLUDES += $(SQL_ODBC_PRIVATE_INCLUDES)

noinst_HEADERS+=$(patsubst %,src/sql/drivers/odbc/%,$(SQL_ODBC_INCLUDES)) \
		$(patsubst %,src/sql/drivers/odbc/%,$(SQL_ODBC_PRIVATE_INCLUDES))

#lib_libCsSql@BUILD_ABI@_la_SOURCES += \
#	src/sql/drivers/odbc/qsql_odbc.cpp
