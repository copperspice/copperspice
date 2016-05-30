set(EXTRA_MYSQL_LIBS CsCore${BUILD_ABI} CsSql${BUILD_ABI})

set(SQL_PUBLIC_INCLUDES
    ${SQL_PUBLIC_INCLUDES}
    QMYSQLDriver
    QMYSQLResult
)

set(SQL_INCLUDES
    ${SQL_INCLUDES}
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/mysql/qsql_mysql.h
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/mysql/qmysqldriver.h
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/mysql/qmysqlresult.h
)

if(WITH_MYSQL_PLUGIN AND MYSQL_FOUND)
    set(EXTRA_MYSQL_LIBS
        ${EXTRA_MYSQL_LIBS}
        ${MYSQL_LIBRARIES}
        ${ZLIB_LIBRARIES}
    )

    set(MYSQL_SOURCES
        ${MYSQL_SOURCES}
        ${CMAKE_CURRENT_SOURCE_DIR}/drivers/mysql/qsql_mysql.cpp
        ${CMAKE_SOURCE_DIR}/src/plugins/sqldrivers/mysql/main.cpp
    )

    include_directories(${MYSQL_INCLUDES})
    add_library(qsqlmysql4 MODULE ${MYSQL_SOURCES})
    target_link_libraries(qsqlmysql4 ${EXTRA_MYSQL_LIBS})

    install(TARGETS qsqlmysql4 DESTINATION ${CMAKE_INSTALL_LIBDIR})
endif()
