set(SQL_PUBLIC_INCLUDES
    ${SQL_PUBLIC_INCLUDES}
    QOCIDriver
    QOCIResult
)

set(SQL_INCLUDES
    ${SQL_INCLUDES}
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/oci/qsql_oci.h
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/oci/qocidriver.h
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/oci/qociresult.h
)

set(SQL_SOURCES
    ${SQL_SOURCES}
    # ${CMAKE_CURRENT_SOURCE_DIR}/drivers/oci/qsql_oci.cpp
)
