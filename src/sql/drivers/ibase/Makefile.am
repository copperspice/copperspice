SQL_DIRS += src/sql/drivers/ibase

SQL_PUBLIC_INCLUDES += \
	QIBaseDriver \
	QIBaseResult


SQL_IBASE_INCLUDES = \
	qsql_ibase.h \
	qibasedriver.h \
	qibaseresult.h

#SQL_IBASE_PRIVATE_INCLUDES =

SQL_INCLUDES += $(SQL_IBASE_INCLUDES)
#SQL_PRIVATE_INCLUDES += $(SQL_IBASE_PRIVATE_INCLUDES)

noinst_HEADERS+=$(patsubst %,src/sql/drivers/ibase/%,$(SQL_IBASE_INCLUDES)) \
		$(patsubst %,src/sql/drivers/ibase/%,$(SQL_IBASE_PRIVATE_INCLUDES))

#lib_libCsSql@BUILD_ABI@_la_SOURCES += \
#	src/sql/drivers/ibase/qsql_ibase.cpp
