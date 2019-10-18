list(APPEND SQL_PUBLIC_INCLUDES
   QSqlQueryModel
   QSqlTableModel
   QSqlRelationalDelegate
   QSqlRelationalTableModel
   QSqlRelation
)

list(APPEND SQL_PRIVATE_INCLUDES
   ${CMAKE_CURRENT_SOURCE_DIR}/models/qsqlquerymodel_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/models/qsqltablemodel_p.h
)

list(APPEND SQL_INCLUDES
   ${CMAKE_CURRENT_SOURCE_DIR}/models/qsqlquerymodel.h
   ${CMAKE_CURRENT_SOURCE_DIR}/models/qsqltablemodel.h
   ${CMAKE_CURRENT_SOURCE_DIR}/models/qsqlrelationaldelegate.h
   ${CMAKE_CURRENT_SOURCE_DIR}/models/qsqlrelationaltablemodel.h
   ${CMAKE_CURRENT_SOURCE_DIR}/models/qsqlrelation.h
)

target_sources(CsSql
   PRIVATE
   ${CMAKE_CURRENT_SOURCE_DIR}/models/qsqlquerymodel.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/models/qsqltablemodel.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/models/qsqlrelationaldelegate.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/models/qsqlrelationaltablemodel.cpp
)
