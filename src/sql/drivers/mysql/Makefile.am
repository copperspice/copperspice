SQL_DIRS += src/sql/drivers/mysql

SQL_PUBLIC_INCLUDES += \
	QMYSQLDriver \
	QMYSQLResult


SQL_MYSQL_INCLUDES = \
	qsql_mysql.h \
	qmysqldriver.h \
	qmysqlresult.h

#SQL_MYSQL_PRIVATE_INCLUDES =

SQL_INCLUDES += $(SQL_MYSQL_INCLUDES)
#SQL_PRIVATE_INCLUDES += $(SQL_MYSQL_PRIVATE_INCLUDES)

noinst_HEADERS+=$(patsubst %,src/sql/drivers/mysql/%,$(SQL_MYSQL_INCLUDES)) \
		$(patsubst %,src/sql/drivers/mysql/%,$(SQL_MYSQL_PRIVATE_INCLUDES))

if BUILD_MYSQL_PLUGIN
lib_qsqlmysql4_la_CXXFLAGS = $(AM_CXXFLAGS) -Iinclude  -Iinclude/QtSql -Iprivateinclude/QtSql -Iinclude/QtCore -Iprivateinclude/QtCore -Iprivateinclude -Iprivateinclude/QtSql/private -DBUILD_DATE=\"`date +%Y-%m-%d`\" -I$(top_builddir)/src/sql/drivers/mysql

lib_LTLIBRARIES += lib/qsqlmysql4.la

lib_qsqlmysql4_la_LDFLAGS = $(AM_LDFLAGS) -avoid-version -no-undefined -module -lmysqlclient -lz 

lib_qsqlmysql4_la_LIBADD=lib/libCsCore@BUILD_ABI@.la lib/libCsSql@BUILD_ABI@.la

lib_qsqlmysql4_la_SOURCES = \
	src/sql/drivers/mysql/qsql_mysql.cpp \
	src/plugins/sqldrivers/mysql/main.cpp


endif
