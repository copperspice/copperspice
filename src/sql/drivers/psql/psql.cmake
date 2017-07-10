set(EXTRA_PSQL_LIBS CsCore${BUILD_ABI} CsSql${BUILD_ABI})

set(SQL_PUBLIC_INCLUDES
    ${SQL_PUBLIC_INCLUDES}
    QPSQLDriver
    QPSQLResult
)

set(SQL_INCLUDES
    ${SQL_INCLUDES}
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/psql/qsql_psql.h
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/psql/qpsqldriver.h
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/psql/qpsqlresult.h
)

if(WITH_PSQL_PLUGIN AND PostgreSQL_FOUND)

    set(EXTRA_PSQL_LIBS
        ${EXTRA_PSQL_LIBS}
        # NOTE: the CMake module documents PostgreSQL_LIBRARIES however in
        # practice that causes build failure on FreeBSD with CMake v3.2.3
        ${PostgreSQL_LIBRARY}
    )

    set(PSQL_SOURCES
        ${PSQL_SOURCES}
        ${CMAKE_CURRENT_SOURCE_DIR}/drivers/psql/qsql_psql.cpp
        ${CMAKE_SOURCE_DIR}/src/plugins/sqldrivers/psql/main.cpp
    )

    include_directories(${PostgreSQL_INCLUDE_DIRS})
    add_library(qsqlpsql${BUILD_ABI} MODULE ${PSQL_SOURCES})
    target_link_libraries(qsqlpsql${BUILD_ABI} ${EXTRA_PSQL_LIBS})

    target_compile_definitions(qsqlpsql${BUILD_ABI} PRIVATE -DIN_TRUE -DQT_PLUGIN)

    set(EXTRA_SQL_DRIVERS
        ${EXTRA_SQL_DRIVERS}
        qsqlpsql${BUILD_ABI}
    )

    install(TARGETS qsqlpsql${BUILD_ABI} DESTINATION ${CMAKE_INSTALL_LIBDIR})
endif()
