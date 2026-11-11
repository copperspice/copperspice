list(APPEND SQL_PUBLIC_INCLUDES
   QIBaseDriver
   QIBaseResult
)

list(APPEND SQL_INCLUDES
   ${PROJECT_SOURCE_DIR}/src/plugins/sqldrivers/ibase/qsql_ibase.h
   ${PROJECT_SOURCE_DIR}/src/plugins/sqldrivers/ibase/qibasedriver.h
   ${PROJECT_SOURCE_DIR}/src/plugins/sqldrivers/ibase/qibaseresult.h
)

# if(WITH_IBASE_PLUGIN AND IBASE_FOUND), unsupported at this time
if (FALSE)

   add_library(CsSqlIBase MODULE "")
   add_library(CopperSpice::CsSqlIBase  ALIAS CsSqlIBase )

   set_target_properties(CsSqlIBase  PROPERTIES OUTPUT_NAME CsSqlIBase ${BUILD_ABI} PREFIX "")

   include_directories(${IBase_INCLUDE_DIRS})

   target_sources(CsSqlPsql
      PRIVATE
      ${PROJECT_SOURCE_DIR}/src/plugins/sqldrivers/ibase/qsql_ibase.cpp
      ${PROJECT_SOURCE_DIR}/src/plugins/sqldrivers/ibase/main.cpp
   )

   target_link_libraries(CsSqlIBase
      CsCore
      CsSql
      ${IBase_LIBRARY}
   )

   target_compile_definitions(CsSqlIBase
      PRIVATE
      -DIN_TRUE
      -DQT_PLUGIN
   )

   install(TARGETS CsSqlIBase  DESTINATION ${CMAKE_INSTALL_LIBDIR})
endif()

