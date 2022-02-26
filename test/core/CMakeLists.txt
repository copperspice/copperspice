include(Load_Catch2)
include(CTest)

add_executable(CsCoreTest "")
set_target_properties(CsCoreTest
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

file(WRITE ${CMAKE_BINARY_DIR}/bin/test/run_coretest.bat
   "@SET PATH=..;%PATH%\n@CsCoreTest\n"
)

target_link_libraries(CsCoreTest
   PUBLIC
   CsCore
   Catch2::Catch2
)

include_directories(
   ${CMAKE_BINARY_DIR}/include
   ${CMAKE_BINARY_DIR}/privateinclude

   ${CMAKE_BINARY_DIR}/include/QtCore
   ${CMAKE_BINARY_DIR}/privateinclude/QtCore/private

   ${CMAKE_CURRENT_SOURCE_DIR}
   ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_sources(CsCoreTest
   PRIVATE
   ${CMAKE_CURRENT_SOURCE_DIR}/../cs_catch2.h

   ${CMAKE_CURRENT_SOURCE_DIR}/test_main.cpp

   ${CMAKE_CURRENT_SOURCE_DIR}/containers/qcache.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/containers/qflatmap.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/containers/qhash.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/containers/qlinkedlist.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/containers/qlist.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/containers/qmap.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/containers/qmultimap.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/containers/qmultihash.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/containers/qqueue.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/containers/qset.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/containers/qstack.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/containers/qstringlist.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/containers/qvector.cpp

   ${CMAKE_CURRENT_SOURCE_DIR}/datetime/qdate.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/datetime/qdatetime.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/datetime/qtime.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/datetime/qtimezone.cpp

   ${CMAKE_CURRENT_SOURCE_DIR}/io/qdatastream.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/io/qdebug.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/io/qdir.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/io/qfile.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/io/qfileinfo.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/io/qlockfile.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/io/qresource.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/io/qsettings.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/io/qtemporaryfile.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/io/qurl.cpp

   ${CMAKE_CURRENT_SOURCE_DIR}/json/qjsonarray.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/json/qjsondocument.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/json/qjsonobject.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/json/qjsonvalue.cpp

   ${CMAKE_CURRENT_SOURCE_DIR}/kernel/qmetaenum.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/kernel/qmetamethod.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/kernel/qmetaobject.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/kernel/qmetaproperty.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/kernel/qobject.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/kernel/qtranslator.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/kernel/qvariant.cpp

   ${CMAKE_CURRENT_SOURCE_DIR}/locale/qlocale.cpp

   ${CMAKE_CURRENT_SOURCE_DIR}/pointer/qpointer.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/pointer/qscopedarraypointer.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/pointer/qscopedpointer.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/pointer/qsharedpointer.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/pointer/qweakpointer.cpp

   ${CMAKE_CURRENT_SOURCE_DIR}/string/cs_string.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/string/qchar.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/string/qreqularexpression.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/string/qstring8.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/string/qstring16.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/string/qstringparser.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/string/qstringview.cpp

   ${CMAKE_CURRENT_SOURCE_DIR}/thread/qatomicint.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/thread/qatomicpointer.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/thread/qmutex.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/thread/qreadwritelock.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/thread/qthread.cpp

   ${CMAKE_CURRENT_SOURCE_DIR}/tools/qbitarray.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/tools/qbytearray.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/tools/qcommandlineoption.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/tools/qcommandlineparser.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/tools/qelapsedtimer.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/tools/qline.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/tools/qlinef.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/tools/qmargins.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/tools/qmarginsf.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/tools/qpair.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/tools/qpoint.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/tools/qpointf.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/tools/qrect.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/tools/qrectf.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/tools/qsize.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/tools/qsizef.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/tools/qtextboundaryfinder.cpp

   ${CMAKE_CURRENT_SOURCE_DIR}/kernel/test_de.ts

   ${CMAKE_CURRENT_SOURCE_DIR}/test.qrc
   ${CMAKE_CURRENT_BINARY_DIR}/qrc_test.cpp
)

function_generate_resources(CsCoreTest)

# generate new qrc if required (adds to existinig cusom command)
set(resource_out ${CMAKE_CURRENT_BINARY_DIR}/qrc_test.cpp)

add_custom_command(
   OUTPUT ${resource_out}
   DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/test_de.qm
   APPEND
)

# sets working dir for CTest, positioned after qrc generation
set(CMAKE_CURRENT_BINARY_DIR "${CMAKE_BINARY_DIR}/bin/")

include(ParseAndAddCatchTests)
ParseAndAddCatchTests(CsCoreTest)
