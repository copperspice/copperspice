add_library(sharedtools OBJECT "")

list(APPEND SHAREDTOOLS_INCLUDES
   ${CMAKE_BINARY_DIR}/privateinclude/QtCore/private
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

function_variable_fixup("${EXTRA_SHAREDTOOLS_CXXFLAGS}" EXTRA_SHAREDTOOLS_CXXFLAGS)
function_variable_fixup("${EXTRA_SHAREDTOOLS_LDFLAGS}"  EXTRA_SHAREDTOOLS_LDFLAGS)

target_compile_definitions(sharedtools PRIVATE ${SHAREDTOOLS_DEFINITIONS})
target_include_directories(sharedtools PRIVATE ${SHAREDTOOLS_INCLUDES})

target_link_libraries(sharedtools
   CsCore
   CsXml
)

target_compile_options(sharedtools
   PRIVATE
   $<$<COMPILE_LANGUAGE:CXX>:SHELL:${EXTRA_SHAREDTOOLS_CXXFLAGS}>
)

set_target_properties(sharedtools
   PROPERTIES
   LINK_FLAGS ${EXTRA_SHAREDTOOLS_LDFLAGS}
)
