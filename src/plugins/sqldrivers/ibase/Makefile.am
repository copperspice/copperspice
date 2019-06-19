SQL_DIRS += src/plugins/sqldrivers/ibase

SQL_IBSE_PUBLIC_INCLUDES = \
	QIBaseDriver \
	QIBaseResult

SQL_IBASE_INCLUDES = \
	qsql_ibase.h \
	qibasedriver.h \
	qibaseresult.h

# SQL_IBASE_PRIVATE_INCLUDES =

SQL_PUBLIC_INCLUDES += $(SQL_IBASE_PUBLIC_INCLUDES)
SQL_INCLUDES += $(SQL_IBASE_INCLUDES)
SQL_PRIVATE_INCLUDES += $(SQL_IBASE_PRIVATE_INCLUDES)

noinst_HEADERS+=$(patsubst %,src/plugins/sqldrivers/ibase/%,$(SQL_IBASE_INCLUDES)) \
		$(patsubst %,src/plugins/sqldrivers/ibase/%,$(SQL_IBASE_PRIVATE_INCLUDES))

if FALSE



#lib_libCsSqlIBase@BUILD_ABI@_la_SOURCES += \
	src/plugins/sqldrivers/ibase/qsql_ibase.cpp \
	src/plugins/sqldrivers/ibase/main.cpp

endif