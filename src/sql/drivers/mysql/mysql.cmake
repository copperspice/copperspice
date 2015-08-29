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
    set(EXTRA_MYSQL_LDFLAGS
        ${EXTRA_MYSQL_LDFLAGS}
        -avoid-version
        -no-undefined
    )

    set(EXTRA_SQL_LIBS
        ${EXTRA_SQL_LIBS}
        ${MYSQL_LIBRARIES}
        ${ZLIB_LIBRARIES}
    )

    set(MYSQL_SOURCES
        ${MYSQL_SOURCES}
        ${CMAKE_CURRENT_SOURCE_DIR}/drivers/mysql/qsql_mysql.cpp
        ${CMAKE_SOURCE_DIR}/src/plugins/sqldrivers/mysql/main.cpp
    )

    function_variable_fixup("${EXTRA_MYSQL_LDFLAGS}" EXTRA_MYSQL_LDFLAGS)

    include_directories(${MYSQL_INCLUDE_DIR})
    add_library(qsqlmysql4 MODULE ${MYSQL_SOURCES})
    set_target_properties(qsqlmysql4 PROPERTIES
        LINK_FLAGS ${EXTRA_MYSQL_LDFLAGS}
    )

    install(TARGETS qsqlmysql4 DESTINATION ${CMAKE_INSTALL_LIBDIR})
endif()

