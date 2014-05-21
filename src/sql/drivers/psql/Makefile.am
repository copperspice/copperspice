SQL_DIRS += src/sql/drivers/psql

SQL_PUBLIC_INCLUDES += \
	QPSQLDriver \
	QPSQLResult


SQL_PSQL_INCLUDES = \
	qsql_psql.h \
	qpsqldriver.h \
	qpsqlresult.h

#SQL_PSQL_PRIVATE_INCLUDES =

SQL_INCLUDES += $(SQL_PSQL_INCLUDES)
#SQL_PRIVATE_INCLUDES += $(SQL_PSQL_PRIVATE_INCLUDES)

noinst_HEADERS+=$(patsubst %,src/sql/drivers/psql/%,$(SQL_PSQL_INCLUDES)) \
		$(patsubst %,src/sql/drivers/psql/%,$(SQL_PSQL_PRIVATE_INCLUDES))


if BUILD_PSQL_PLUGIN
lib_LTLIBRARIES += lib/qsqlpsql4.la

lib_qsqlpsql4_la_CXXFLAGS = $(AM_CXXFLAGS) -Iinclude  -Iinclude/QtSql -Iprivateinclude/QtSql -Iinclude/QtCore -Iprivateinclude/QtCore -Iprivateinclude -Iprivateinclude/QtSql/private -DBUILD_DATE=\"`date +%Y-%m-%d`\" -DIN_TRUE

lib_qsqlpsql4_la_LDFLAGS = $(AM_LDFLAGS) -avoid-version -no-undefined -module -lpq

lib_qsqlpsql4_la_LIBADD=lib/libCsCore1.la lib/libCsSql1.la

lib_qsqlpsql4_la_SOURCES = \
	src/sql/drivers/psql/qsql_psql.cpp \
	src/plugins/sqldrivers/psql/main.cpp
endif
