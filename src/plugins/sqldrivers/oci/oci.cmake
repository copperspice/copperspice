list(APPEND SQL_PUBLIC_INCLUDES
   QOCIDriver
   QOCIResult
)

list(APPEND SQL_INCLUDES
   ${CMAKE_SOURCE_DIR}/src/plugins/sqldrivers/oci/qsql_oci.h
   ${CMAKE_SOURCE_DIR}/src/plugins/sqldrivers/oci/qocidriver.h
   ${CMAKE_SOURCE_DIR}/src/plugins/sqldrivers/oci/qociresult.h
)

# if(WITH_OCI_PLUGIN AND OCI_FOUND), unsupported at this time
if (FALSE)

   add_library(CsSqlOci MODULE "")
   add_library(CopperSpice::CsSqlOci ALIAS CsSqlOci)

   set_target_properties(CsSqlOci PROPERTIES OUTPUT_NAME CsSqlOci${BUILD_ABI} PREFIX "")

   include_directories(${OCI_INCLUDE_DIRS})

   target_sources(CsSqlOci
      PRIVATE
      ${CMAKE_SOURCE_DIR}/src/plugins/sqldrivers/oci/qsql_oci.cpp
      ${CMAKE_SOURCE_DIR}/src/plugins/sqldrivers/oci/main.cpp
   )

   target_link_libraries(CsSqlOci
      CsCore
      CsSql
      ${OCI_LIBRARY}
   )

   target_compile_definitions(CsSqlOci
      PRIVATE
      -DIN_TRUE
      -DQT_PLUGIN
   )

   if(BUILDING_RPM)
      install(TARGETS CsSqlOci DESTINATION ${CMAKE_INSTALL_LIBDIR}/copperspice/plugins/sqldrivers)
   else()
      install(TARGETS CsSqlOci DESTINATION ${CMAKE_INSTALL_LIBDIR})
   endif()
endif()
