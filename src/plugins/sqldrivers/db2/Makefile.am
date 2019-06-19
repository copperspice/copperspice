SQL_DIRS += src/plugins/sqldrivers/db2

SQL_DB2_PUBLIC_INCLUDES = \
	QDB2Driver \
	QDB2Result

SQL_DB2_INCLUDES = \
	qsql_db2.h \
	qdb2driver.h \
	qdb2result.h

# SQL_DB2_PRIVATE_INCLUDES =

SQL_PUBLIC_INCLUDES += $(SQL_DB2_PUBLIC_INCLUDES)
SQL_INCLUDES += $(SQL_DB2_INCLUDES)
SQL_PRIVATE_INCLUDES += $(SQL_DB2_PRIVATE_INCLUDES)

noinst_HEADERS+=$(patsubst %,src/plugins/sqldrivers/db2/%,$(SQL_DB2_INCLUDES)) \
		$(patsubst %,src/plugins/sqldrivers/db2/%,$(SQL_DB2_PRIVATE_INCLUDES))

# if BUILD_PSQL_PLUGIN
if FALSE

lib_LTLIBRARIES += lib/CsSqlDb2@BUILD_ABI@.la

lib_CsSqlDb2@BUILD_ABI@_la_CXXFLAGS = -Iinclude -Iprivateinclude \
   -Iinclude/QtCore  -Iprivateinclude/QtCore  \
   -Iinclude/QtSql -Iprivateinclude/QtSql -Iprivateinclude/QtSql/private \
   $(AM_CXXFLAGS) \
   $(patsubst %,-I%/include,$(DB2_DIR)) \
   -DBUILD_DATE=\"`date +%Y-%m-%d`\" \
   -DQT_PLUGIN

lib_CsSqlDb2@BUILD_ABI@_la_LDFLAGS = $(AM_LDFLAGS) \
   $(patsubst %,-L%/lib,$(DB2_DIR)) \
   -avoid-version -no-undefined -module

lib_CsSqlDb2@BUILD_ABI@_la_LIBADD=lib/libCsCore@BUILD_ABI@.la lib/libCsSql@BUILD_ABI@.la

lib_CsSqlDb2@BUILD_ABI@_la_SOURCES = \
	src/plugins/sqldrivers/db2/qsql_db2.cpp \
	src/plugins/sqldrivers/db2/psql/main.cpp


endif

