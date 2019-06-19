set(EXTRA_PSQL_LIBS CsCore${BUILD_ABI} CsSql${BUILD_ABI})

set(SQL_PUBLIC_INCLUDES
    ${SQL_PUBLIC_INCLUDES}
    QPSQLDriver
    QPSQLResult
)

set(SQL_INCLUDES
    ${SQL_INCLUDES}
    ${CMAKE_SOURCE_DIR}/src/plugins/sqldrivers/psql/qsql_psql.h
    ${CMAKE_SOURCE_DIR}/src/plugins/sqldrivers/psql/qpsqldriver.h
    ${CMAKE_SOURCE_DIR}/src/plugins/sqldrivers/psql/qpsqlresult.h
)

if(WITH_PSQL_PLUGIN AND PostgreSQL_FOUND)

    set(EXTRA_PSQL_LIBS
        ${EXTRA_PSQL_LIBS}
        ${PostgreSQL_LIBRARY}
    )

    set(PSQL_SOURCES
        ${PSQL_SOURCES}
        ${CMAKE_SOURCE_DIR}/src/plugins/sqldrivers/psql/qsql_psql.cpp
        ${CMAKE_SOURCE_DIR}/src/plugins/sqldrivers/psql/main.cpp
    )

    include_directories(${PostgreSQL_INCLUDE_DIRS})
    add_library(CsSqlPsql${BUILD_ABI} MODULE ${PSQL_SOURCES})
    target_link_libraries(CsSqlPsql${BUILD_ABI} ${EXTRA_PSQL_LIBS})

    target_compile_definitions(CsSqlPsql${BUILD_ABI} PRIVATE -DIN_TRUE -DQT_PLUGIN)

    install(TARGETS CsSqlPsql${BUILD_ABI} DESTINATION ${CMAKE_INSTALL_LIBDIR})
endif()
