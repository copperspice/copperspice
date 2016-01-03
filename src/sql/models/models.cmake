set(SQL_PUBLIC_INCLUDES
    ${SQL_PUBLIC_INCLUDES}
    QSqlQueryModel
    QSqlTableModel
    QSqlRelationalDelegate
    QSqlRelationalTableModel
    QSqlRelation
)

set(SQL_PRIVATE_INCLUDES
    ${SQL_PRIVATE_INCLUDES}
    ${CMAKE_CURRENT_SOURCE_DIR}/models/qsqlquerymodel_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/models/qsqltablemodel_p.h
)

set(SQL_INCLUDES
    ${SQL_INCLUDES}
    ${CMAKE_CURRENT_SOURCE_DIR}/models/qsqlquerymodel.h
    ${CMAKE_CURRENT_SOURCE_DIR}/models/qsqltablemodel.h
    ${CMAKE_CURRENT_SOURCE_DIR}/models/qsqlrelationaldelegate.h
    ${CMAKE_CURRENT_SOURCE_DIR}/models/qsqlrelationaltablemodel.h
    ${CMAKE_CURRENT_SOURCE_DIR}/models/qsqlrelation.h
)

set(SQL_SOURCES
    ${SQL_SOURCES}
    ${CMAKE_CURRENT_SOURCE_DIR}/models/qsqlquerymodel.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/models/qsqltablemodel.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/models/qsqlrelationaldelegate.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/models/qsqlrelationaltablemodel.cpp
)
