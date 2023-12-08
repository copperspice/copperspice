include(CTest)

add_executable(CsNetworkTest "")
set_target_properties(CsNetworkTest
   PROPERTIES
   RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/test"
)

if (MSVC)
   # CopperSpice must be compiled with C++20 in order to run the unit test for C++20

else()
   set_target_properties(CsNetworkTest
      PROPERTIES

      # only a request for C++20
      CXX_STANDARD_REQUIRED OFF
      CXX_STANDARD 20
   )

endif()

if (CS_INSTALL_MODE STREQUAL "Deploy" AND CMAKE_SYSTEM_NAME MATCHES "Windows")

   STRING(CONCAT NETWORK_BATCH_FILE
      "@SET PATH=..;%PATH% \n"
      "\n"
      "REM   @CsNetworkTest\n"
      "\n"
      "@ECHO OFF           \n"
      "\n"
      "ECHO:               \n"
      "ECHO CopperSpice was built in Deploy Mode.\n"
      "ECHO    Running CsNetworkTest may require some of the following libraries. \n"
      "ECHO    Either modify your PATH or copy these libraries to the current folder. \n"
      "ECHO:               \n"
      "ECHO CsSignal       \n"
      "ECHO CsString"
   )

   file(WRITE ${CMAKE_BINARY_DIR}/bin/test/run_networktest.bat
      ${NETWORK_BATCH_FILE}
   )

else()
   file(WRITE ${CMAKE_BINARY_DIR}/bin/test/run_networktest.bat
      "@SET PATH=..;%PATH%\n@CsNetworkTest\n"
   )

endif()

target_link_libraries(CsNetworkTest
   PUBLIC
   CsCore
   CsNetwork
)

include_directories(
   ${CMAKE_CURRENT_SOURCE_DIR}
   ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_sources(CsNetworkTest
   PRIVATE
   ${CMAKE_CURRENT_SOURCE_DIR}/../catch2/catch.hpp
   ${CMAKE_CURRENT_SOURCE_DIR}/../cs_catch2.h

   ${CMAKE_CURRENT_SOURCE_DIR}/test_main.cpp

   ${CMAKE_CURRENT_SOURCE_DIR}/access/qnetwork_reply.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/access/qnetwork_request.cpp

   ${CMAKE_CURRENT_SOURCE_DIR}/kernel/qhostinfo.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/kernel/qhostaddress.cpp

   ${CMAKE_CURRENT_SOURCE_DIR}/socket/qtcpserver.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/socket/qtcpsocket.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/socket/qudpsocket.cpp
)

# sets working dir for CTest
set(CMAKE_CURRENT_BINARY_DIR "${CMAKE_BINARY_DIR}/bin/")

include(ParseAndAddCatchTests)
ParseAndAddCatchTests(CsNetworkTest)
