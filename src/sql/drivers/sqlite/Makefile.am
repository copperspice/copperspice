SQL_DIRS += src/sql/drivers/sqlite

SQL_PUBLIC_INCLUDES += \
	QSQLiteResult \
	QSQLiteDriver


SQL_SQLITE_INCLUDES = \
	qsql_sqlite.h \
	qsqliteresult.h \
	qsqlitedriver.h

#SQL_SQLITE_PRIVATE_INCLUDES =

SQL_INCLUDES += $(SQL_SQLITE_INCLUDES)
#SQL_PRIVATE_INCLUDES += $(SQL_SQLITE_PRIVATE_INCLUDES)

noinst_HEADERS+=$(patsubst %,src/sql/drivers/sqlite/%,$(SQL_SQLITE_INCLUDES)) \
		$(patsubst %,src/sql/drivers/sqlite/%,$(SQL_SQLITE_PRIVATE_INCLUDES))

lib_libCsSql@BUILD_ABI@_la_SOURCES += \
	src/3rdparty/sqlite/sqlite3.c  \
	src/sql/drivers/sqlite/qsql_sqlite.cpp

noinst_HEADERS+= src/3rdparty/sqlite/sqlite3.h

lib_libCsSql@BUILD_ABI@_la_CXXFLAGS += -I$(top_srcdir)/src/3rdparty/sqlite
lib_libCsSql@BUILD_ABI@_la_CFLAGS += -I$(top_srcdir)/src/3rdparty/sqlite -Iinclude/QtCore
