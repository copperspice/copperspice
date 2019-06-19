SQL_DIRS += src/plugins/sqldrivers/psql

SQL_PSQL_PUBLIC_INCLUDES = \
	QPSQLDriver \
	QPSQLResult

SQL_PSQL_INCLUDES = \
	qsql_psql.h \
	qpsqldriver.h \
	qpsqlresult.h

# SQL_PSQL_PRIVATE_INCLUDES =

SQL_PUBLIC_INCLUDES += $(SQL_PSQL_PUBLIC_INCLUDES)
SQL_INCLUDES += $(SQL_PSQL_INCLUDES)
SQL_PRIVATE_INCLUDES += $(SQL_PSQL_PRIVATE_INCLUDES)

noinst_HEADERS+=$(patsubst %,src/plugins/sqldrivers/psql/%,$(SQL_PSQL_INCLUDES)) \
	$(patsubst %,src/plugins/sqldrivers/psql/%,$(SQL_PSQL_PRIVATE_INCLUDES))

if BUILD_PSQL_PLUGIN

lib_LTLIBRARIES += lib/CsSqlPsql@BUILD_ABI@.la

lib_CsSqlPsql@BUILD_ABI@_la_CXXFLAGS = -Iinclude -Iprivateinclude \
   -Iinclude/QtCore  -Iprivateinclude/QtCore  \
   -Iinclude/QtSql -Iprivateinclude/QtSql -Iprivateinclude/QtSql/private \
   $(AM_CXXFLAGS) \
   $(patsubst %,-I%/include,$(PSQL_DIR)) \
   -DBUILD_DATE=\"`date +%Y-%m-%d`\" \
   -DQT_PLUGIN

lib_CsSqlPsql@BUILD_ABI@_la_LDFLAGS = $(AM_LDFLAGS) \
   $(patsubst %,-L%/lib,$(PSQL_DIR)) \
   -avoid-version -no-undefined -module -lpq

lib_CsSqlPsql@BUILD_ABI@_la_LIBADD=lib/libCsCore@BUILD_ABI@.la lib/libCsSql@BUILD_ABI@.la

lib_CsSqlPsql@BUILD_ABI@_la_SOURCES = \
	src/plugins/sqldrivers/psql/qsql_psql.cpp \
	src/plugins/sqldrivers/psql/main.cpp

endif
