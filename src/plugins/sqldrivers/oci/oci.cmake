set(EXTRA_OCI_LIBS CsCore${BUILD_ABI} CsSql${BUILD_ABI})

set(SQL_PUBLIC_INCLUDES
    ${SQL_PUBLIC_INCLUDES}
    QOCIDriver
    QOCIResult
)

set(SQL_INCLUDES
    ${SQL_INCLUDES}
    ${CMAKE_SOURCE_DIR}/src/plugins/sqldrivers/oci/qsql_oci.h
    ${CMAKE_SOURCE_DIR}/src/plugins/sqldrivers/oci/qocidriver.h
    ${CMAKE_SOURCE_DIR}/src/plugins/sqldrivers/oci/qociresult.h
)


if (FALSE)


   set(OCI_SOURCES
    ${OCI_SOURCES}
    ${CMAKE_SOURCE_DIR}/src/plugins/sqldrivers/oci/qsql_oci.cpp
    ${CMAKE_SOURCE_DIR}/src/plugins/sqldrivers/oci/main.cpp

   )

endif()