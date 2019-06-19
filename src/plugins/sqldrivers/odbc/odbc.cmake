set(EXTRA_ODBC_LIBS CsCore${BUILD_ABI} CsSql${BUILD_ABI})

set(SQL_PUBLIC_INCLUDES
    ${SQL_PUBLIC_INCLUDES}
    QODBCDriver
    QODBCResult
)

set(SQL_INCLUDES
    ${SQL_INCLUDES}
    ${CMAKE_SOURCE_DIR}/src/plugins/sqldrivers/odbc/qsql_odbc.h
    ${CMAKE_SOURCE_DIR}/src/plugins/sqldrivers/odbc/qodbcdriver.h
    ${CMAKE_SOURCE_DIR}/src/plugins/sqldrivers/odbc/qodbcresult.h
)

#   if(WITH_ODBC_PLUGIN AND ODBC_FOUND)

if (FALSE)

    set(EXTRA_ODBC_LIBS
        ${EXTRA_ODBC_LIBS}
        ${ODBC_LIBRARY}
    )

   set(SQL_SOURCES
       ${SQL_SOURCES}
       ${CMAKE_SOURCE_DIR}/src/plugins/sqldrivers/odbc/main.cpp
       ${CMAKE_SOURCE_DIR}/src/plugins/sqldrivers/odbc/qsql_odbc.cpp
   )

    include_directories(${ODBC_INCLUDE_DIRS})
    add_library(CsSqlDb2${BUILD_ABI} MODULE ${ODBC_SOURCES})
    target_link_libraries(CsSqlOdbc${BUILD_ABI} ${EXTRA_ODBC_LIBS})

    target_compile_definitions(CsSqlOdbc${BUILD_ABI} PRIVATE -DIN_TRUE -DQT_PLUGIN)

    install(TARGETS CsSqlOdbc${BUILD_ABI} DESTINATION ${CMAKE_INSTALL_LIBDIR})

endif()
