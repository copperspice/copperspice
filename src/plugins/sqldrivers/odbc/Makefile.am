SQL_DIRS += src/plugins/sqldrivers/odbc

SQL_ODBC_PUBLIC_INCLUDES = \
	QODBCDriver \
	QODBCResult

SQL_ODBC_INCLUDES = \
	qsql_odbc.h \
	qodbcdriver.h \
	qodbcresult.h

# SQL_ODBC_PRIVATE_INCLUDES =

SQL_PUBLIC_INCLUDES += $(SQL_ODBC_PUBLIC_INCLUDES)
SQL_INCLUDES += $(SQL_ODBC_INCLUDES)
SQL_PRIVATE_INCLUDES += $(SQL_ODBC_PRIVATE_INCLUDES)

noinst_HEADERS+=$(patsubst %,src/plugins/sqldrivers/odbc/%,$(SQL_ODBC_INCLUDES)) \
		$(patsubst %,src/plugins/sqldrivers/odbc/%,$(SQL_ODBC_PRIVATE_INCLUDES))

if BUILD_ODBC_PLUGIN

lib_LTLIBRARIES += lib/CsSqlOdbc@BUILD_ABI@.la

lib_CsSqlOdbc@BUILD_ABI@_la_CXXFLAGS = -Iinclude -Iprivateinclude \
   -Iinclude/QtCore  -Iprivateinclude/QtCore  \
   -Iinclude/QtSql -Iprivateinclude/QtSql -Iprivateinclude/QtSql/private \
   $(AM_CXXFLAGS) \
   -DBUILD_DATE=\"`date +%Y-%m-%d`\" \
   -DQT_PLUGIN

lib_CsSqlOdbc@BUILD_ABI@_la_LDFLAGS = $(AM_LDFLAGS) \
   $(patsubst %,-L%/lib,$(ODBC_DIR)) \
   -avoid-version -no-undefined -module

lib_CsSqlOdbc@BUILD_ABI@_la_LIBADD=lib/libCsCore@BUILD_ABI@.la lib/libCsSql@BUILD_ABI@.la

lib_CsSqlOdbc@BUILD_ABI@_la_SOURCES = \
	src/plugins/sqldrivers/odbc/main.cpp \
	src/plugins/sqldrivers/odbc/qsql_odbc.cpp

endif
