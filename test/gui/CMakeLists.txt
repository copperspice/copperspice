include(CTest)

add_executable(CsGuiTest "")
set_target_properties(CsGuiTest
   PROPERTIES
   RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/test"
)

if (CS_INSTALL_MODE STREQUAL "Deploy" AND CMAKE_SYSTEM_NAME MATCHES "Windows")

   STRING(CONCAT GUI_BATCH_FILE
      "@SET PATH=..;%PATH% \n"
      "\n"
      "REM   @CsGuiTest\n"
      "\n"
      "@ECHO OFF           \n"
      "\n"
      "ECHO:               \n"
      "ECHO CopperSpice was built in Deploy Mode.\n"
      "ECHO    Running CsGuiTest may require some of the following libraries. \n"
      "ECHO    Either modify your PATH or copy these libraries to the current folder. \n"
      "ECHO:               \n"
      "ECHO CsSignal       \n"
      "ECHO CsString"
   )

   file(WRITE ${CMAKE_BINARY_DIR}/bin/test/run_guitest.bat
      ${GUI_BATCH_FILE}
   )

else()
   file(WRITE ${CMAKE_BINARY_DIR}/bin/test/run_guitest.bat
      "@SET PATH=..;%PATH%\n@CsGuiTest\n"
   )

endif()

target_link_libraries(CsGuiTest
   PUBLIC
   CsCore
   CsNetwork
   CsGui
)

include_directories(
   ${CMAKE_CURRENT_SOURCE_DIR}
   ${CMAKE_CURRENT_SOURCE_DIR}/..

   ${CMAKE_BINARY_DIR}/privateinclude/QtGui/private
)

target_sources(CsGuiTest
   PRIVATE
   ${CMAKE_CURRENT_SOURCE_DIR}/../catch2/catch.hpp
   ${CMAKE_CURRENT_SOURCE_DIR}/../cs_catch2.h

   ${CMAKE_CURRENT_SOURCE_DIR}/test_main.cpp

   ${CMAKE_CURRENT_SOURCE_DIR}/kernel/qvariant.cpp

   ${CMAKE_CURRENT_SOURCE_DIR}/text/qssparser.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/text/qtextobject.cpp
)

# sets working dir for CTest
set(CMAKE_CURRENT_BINARY_DIR "${CMAKE_BINARY_DIR}/bin/")

include(ParseAndAddCatchTests)
ParseAndAddCatchTests(CsGuiTest)
