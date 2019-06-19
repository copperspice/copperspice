set(EXTRA_MYSQL_LIBS CsCore${BUILD_ABI} CsSql${BUILD_ABI})

set(SQL_PUBLIC_INCLUDES
    ${SQL_PUBLIC_INCLUDES}
    QMYSQLDriver
    QMYSQLResult
)

set(SQL_INCLUDES
    ${SQL_INCLUDES}
    ${CMAKE_SOURCE_DIR}/src/plugins/sqldrivers/mysql/qsql_mysql.h
    ${CMAKE_SOURCE_DIR}/src/plugins/sqldrivers/mysql/qmysqldriver.h
    ${CMAKE_SOURCE_DIR}/src/plugins/sqldrivers/mysql/qmysqlresult.h
)

if(WITH_MYSQL_PLUGIN AND MYSQL_FOUND)

    set(EXTRA_MYSQL_LIBS
        ${EXTRA_MYSQL_LIBS}
        ${MYSQL_LIBRARIES}
        ${ZLIB_LIBRARIES}
    )

    set(MYSQL_SOURCES
        ${MYSQL_SOURCES}
        ${CMAKE_SOURCE_DIR}/src/plugins/sqldrivers/mysql/qsql_mysql.cpp
        ${CMAKE_SOURCE_DIR}/src/plugins/sqldrivers/mysql/main.cpp
    )

    include_directories(${MYSQL_INCLUDES})
    add_library(CsSqlMySql${BUILD_ABI} MODULE ${MYSQL_SOURCES})
    target_link_libraries(CsSqlMySql${BUILD_ABI} ${EXTRA_MYSQL_LIBS})

    target_compile_definitions(CsSqlMySql${BUILD_ABI} PRIVATE -DIN_TRUE -DQT_PLUGIN)

    install(TARGETS CsSqlMySql${BUILD_ABI} DESTINATION ${CMAKE_INSTALL_LIBDIR})
endif()
