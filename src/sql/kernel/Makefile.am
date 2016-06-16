SQL_DIRS += src/sql/kernel

SQL_PUBLIC_INCLUDES += \
	QSqlDatabase \
	QSqlDriver \
	QSqlDriverPlugin \
	QSqlError \
	QSqlField \
	QSqlIndex \
	QSqlQuery \
	QSqlRecord \
	QSqlResult \
	QSqlDriverCreator \
	QSqlDriverCreatorBase \
	QSqlDriverFactoryInterface \
	QtSql


SQL_KERNEL_INCLUDES = \
	qsql.h \
	qsqldatabase.h \
	qsqldriver.h \
	qsqldriverplugin.h \
	qsqlerror.h \
	qsqlfield.h \
	qsqlindex.h \
	qsqlquery.h \
	qsqlrecord.h \
	qsqlresult.h \
	qsqldrivercreator.h \
	qsqldrivercreatorbase.h \
	qsqldriverfactoryinterface.h \
	qtsql.h

SQL_KERNEL_PRIVATE_INCLUDES = \
	qsqlnulldriver_p.h \
	qsqlcachedresult_p.h

SQL_INCLUDES += $(SQL_KERNEL_INCLUDES)
SQL_PRIVATE_INCLUDES += $(SQL_KERNEL_PRIVATE_INCLUDES)

noinst_HEADERS+=$(patsubst %,src/sql/kernel/%,$(SQL_KERNEL_INCLUDES)) \
		$(patsubst %,src/sql/kernel/%,$(SQL_KERNEL_PRIVATE_INCLUDES))

lib_libCsSql@BUILD_ABI@_la_SOURCES += \
	src/sql/kernel/qsqlquery.cpp \
	src/sql/kernel/qsqldatabase.cpp \
	src/sql/kernel/qsqlfield.cpp \
	src/sql/kernel/qsqlrecord.cpp \
	src/sql/kernel/qsqldriver.cpp \
	src/sql/kernel/qsqldriverplugin.cpp \
	src/sql/kernel/qsqlerror.cpp \
	src/sql/kernel/qsqlresult.cpp \
	src/sql/kernel/qsqlindex.cpp \
	src/sql/kernel/qsqlcachedresult.cpp
