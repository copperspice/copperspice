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
    # TODO: those should probably apply to a single object
    set(EXTRA_SQL_LDFLAGS
        ${EXTRA_SQL_LDFLAGS}
        -avoid-version
        -no-undefined
        # -module
    )

    set(EXTRA_SQL_LIBS
        ${EXTRA_SQL_LIBS}
        ${MYSQL_LIBRARIES}
        ${ZLIB_LIBRARIES}
    )

    set(SQL_SOURCES
        ${SQL_SOURCES}
        ${CMAKE_CURRENT_SOURCE_DIR}/drivers/mysql/qsql_mysql.cpp
        ${CMAKE_SOURCE_DIR}/src/plugins/sqldrivers/mysql/main.cpp
    )

    include_directories(${MYSQL_INCLUDE_DIR})
endif()
