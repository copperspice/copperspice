include(CTest)

add_executable(CsNetworkTest "")
set_target_properties(CsNetworkTest
   PROPERTIES
   RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/test"
)

if (${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.12.0")
   if (MSVC)
      # CopperSpice must be compiled with C++20 in order to run the unit test for C++20

   else()
      set_target_properties(CsCoreTest
         PROPERTIES

         # only a request for C++20
         CXX_STANDARD_REQUIRED OFF
         CXX_STANDARD 20
      )

   endif()
endif()

file(WRITE ${CMAKE_BINARY_DIR}/bin/test/run_networktest.bat
   "@SET PATH=..;%PATH%\n@CsNetworkTest\n"
)

target_link_libraries(CsNetworkTest
   PUBLIC
   CsCore
   CsNetwork
)

include_directories(
   ${CMAKE_BINARY_DIR}/include
   ${CMAKE_BINARY_DIR}/privateinclude

   ${CMAKE_BINARY_DIR}/include/QtCore
   ${CMAKE_BINARY_DIR}/privateinclude/QtCore/private

   ${CMAKE_BINARY_DIR}/include/QtNetwork
   ${CMAKE_BINARY_DIR}/privateinclude/QtNetwork/private

   ${CMAKE_CURRENT_SOURCE_DIR}
   ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_sources(CsNetworkTest
   PRIVATE
   ${CMAKE_CURRENT_SOURCE_DIR}/../catch2/catch.hpp
   ${CMAKE_CURRENT_SOURCE_DIR}/../cs_catch2.h

   ${CMAKE_CURRENT_SOURCE_DIR}/test_main.cpp

   ${CMAKE_CURRENT_SOURCE_DIR}/kernel/qhostinfo.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/kernel/qhostaddress.cpp

   ${CMAKE_CURRENT_SOURCE_DIR}/socket/qtcpserver.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/socket/qtcpsocket.cpp
)

include(ParseAndAddCatchTests)
ParseAndAddCatchTests(CsNetworkTest)
