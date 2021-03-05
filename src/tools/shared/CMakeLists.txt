add_library(sharedtools OBJECT "")

list(APPEND SHAREDTOOLS_INCLUDES
   ${CMAKE_BINARY_DIR}/include
   ${CMAKE_BINARY_DIR}/privateinclude

   ${CMAKE_BINARY_DIR}/include/QtCore
   ${CMAKE_BINARY_DIR}/privateinclude/QtCore/private

   ${CMAKE_BINARY_DIR}/include/QtXml
   ${CMAKE_BINARY_DIR}/privateinclude/QtXml/private

   ${CMAKE_SOURCE_DIR}/src/tools/shared
)

list(APPEND SHAREDTOOLS_HEADERS
    ${CMAKE_SOURCE_DIR}/src/tools/shared/translator.h
    ${CMAKE_SOURCE_DIR}/src/tools/shared/translatormessage.h
)

target_sources(sharedtools
   PRIVATE
   ${CMAKE_SOURCE_DIR}/src/tools/shared/numerus.cpp
   ${CMAKE_SOURCE_DIR}/src/tools/shared/translator.cpp
   ${CMAKE_SOURCE_DIR}/src/tools/shared/translatormessage.cpp

   ${CMAKE_SOURCE_DIR}/src/tools/shared/qm.cpp
   ${CMAKE_SOURCE_DIR}/src/tools/shared/ts.cpp
)

function_variable_fixup("${SHAREDTOOLS_EXTRA_CXXFLAGS}" SHAREDTOOLS_EXTRA_CXXFLAGS)
function_variable_fixup("${SHAREDTOOLS_EXTRA_LDFLAGS}"  SHAREDTOOLS_EXTRA_LDFLAGS)

target_compile_definitions(sharedtools PRIVATE ${SHAREDTOOLS_DEFINITIONS})
target_include_directories(sharedtools PRIVATE ${SHAREDTOOLS_INCLUDES})

set_target_properties(sharedtools
   PROPERTIES
   COMPILE_FLAGS ${SHAREDTOOLS_EXTRA_CXXFLAGS}
   LINK_FLAGS ${SHAREDTOOLS_EXTRA_LDFLAGS}
)
