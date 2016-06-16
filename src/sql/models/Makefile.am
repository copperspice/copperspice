SQL_DIRS += src/sql/models

SQL_PUBLIC_INCLUDES += \
	QSqlQueryModel \
	QSqlTableModel \
	QSqlRelationalDelegate \
	QSqlRelationalTableModel \
	QSqlRelation

SQL_MODELS_INCLUDES = \
	qsqlquerymodel.h \
	qsqltablemodel.h \
	qsqlrelationaldelegate.h \
	qsqlrelationaltablemodel.h \
	qsqlrelation.h


SQL_MODELS_PRIVATE_INCLUDES = \
	qsqlquerymodel_p.h \
	qsqltablemodel_p.h

SQL_INCLUDES += $(SQL_MODELS_INCLUDES)
SQL_PRIVATE_INCLUDES += $(SQL_MODELS_PRIVATE_INCLUDES)

noinst_HEADERS+=$(patsubst %,src/sql/models/%,$(SQL_MODELS_INCLUDES)) \
		$(patsubst %,src/sql/models/%,$(SQL_MODELS_PRIVATE_INCLUDES))

lib_libCsSql@BUILD_ABI@_la_SOURCES += \
	src/sql/models/qsqlquerymodel.cpp \
	src/sql/models/qsqltablemodel.cpp \
	src/sql/models/qsqlrelationaldelegate.cpp \
	src/sql/models/qsqlrelationaltablemodel.cpp
