SQL_DIRS += src/plugins/sqldrivers/mysql

SQL_MYSQL_PUBLIC_INCLUDES = \
	QMYSQLDriver \
	QMYSQLResult

SQL_MYSQL_INCLUDES = \
	qsql_mysql.h \
	qmysqldriver.h \
	qmysqlresult.h

# SQL_MYSQL_PRIVATE_INCLUDES =

SQL_PUBLIC_INCLUDES += $(SQL_MYSQL_PUBLIC_INCLUDES)
SQL_INCLUDES += $(SQL_MYSQL_INCLUDES)
SQL_PRIVATE_INCLUDES += $(SQL_MYSQL_PRIVATE_INCLUDES)

noinst_HEADERS+=$(patsubst %,src/plugins/sqldrivers/mysql/%,$(SQL_MYSQL_INCLUDES)) \
		$(patsubst %,src/plugins/sqldrivers/mysqll/%,$(SQL_MYSQL_PRIVATE_INCLUDES))

if BUILD_MYSQL_PLUGIN

lib_LTLIBRARIES += lib/CsSqlMySql@BUILD_ABI@.la

lib_CsSqlMySql@BUILD_ABI@_la_CXXFLAGS =-Iinclude -Iprivateinclude \
   -Iinclude/QtCore  -Iprivateinclude/QtCore  \
   -Iinclude/QtSql -Iprivateinclude/QtSql -Iprivateinclude/QtSql/private \
   $(AM_CXXFLAGS) \
   -DBUILD_DATE=\"`date +%Y-%m-%d`\" \
   -DQT_PLUGIN

lib_CsSqlMySql@BUILD_ABI@_la_LDFLAGS = $(AM_LDFLAGS) \
   -avoid-version -no-undefined -module -lmysqlclient -lz

lib_CsSqlMySql@BUILD_ABI@_la_LIBADD=lib/libCsCore@BUILD_ABI@.la lib/libCsSql@BUILD_ABI@.la

lib_CsSqlMySql@BUILD_ABI@_la_SOURCES = \
	src/plugins/sqldrivers/mysql/qsql_mysql.cpp \
	src/plugins/sqldrivers/mysql/main.cpp

endif
