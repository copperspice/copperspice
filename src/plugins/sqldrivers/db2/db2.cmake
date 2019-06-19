set(EXTRA_DB2_LIBS CsCore${BUILD_ABI} CsSql${BUILD_ABI})

set(SQL_PUBLIC_INCLUDES
    ${SQL_PUBLIC_INCLUDES}
    QDB2Driver
    QDB2Result
)

set(SQL_INCLUDES
    ${SQL_INCLUDES}
    ${CMAKE_SOURCE_DIR}/src/plugins/sqldrivers/db2/qsql_db2.h
    ${CMAKE_SOURCE_DIR}/src/plugins/sqldrivers/db2/qdb2driver.h
    ${CMAKE_SOURCE_DIR}/src/plugins/sqldrivers/db2/qdb2result.h
)

#  if(WITH_DB2_PLUGIN AND DB2_FOUND)

if (FALSE)

    set(EXTRA_DB2_LIBS
        ${EXTRA_DB2_LIBS}
        ${DB2_LIBRARY}
    )

   set(SQL_SOURCES
       ${DB2_SOURCES}
       ${CMAKE_SOURCE_DIR}/src/plugins/sqldrivers/db2/qsql_db2.cpp
   )

    include_directories(${DB2_INCLUDE_DIRS})
    add_library(CsSqlDb2${BUILD_ABI} MODULE ${DB2_SOURCES})
    target_link_libraries(CsSqlDb2l${BUILD_ABI} ${EXTRA_DB2_LIBS})

    target_compile_definitions(CsSqlDb2${BUILD_ABI} PRIVATE -DIN_TRUE -DQT_PLUGIN)

    install(TARGETS CsSqlDb2${BUILD_ABI} DESTINATION ${CMAKE_INSTALL_LIBDIR})
endif()

