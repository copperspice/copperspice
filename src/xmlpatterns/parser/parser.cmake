list(APPEND XMLPATTERNS_PRIVATE_INCLUDES
   ${CMAKE_CURRENT_SOURCE_DIR}/parser/qparsercontext_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/parser/qmaintainingreader_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/parser/qquerytransformparser_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/parser/qtokenizer_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/parser/qtokenrevealer_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/parser/qtokensource_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/parser/qxquerytokenizer_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/parser/qxslttokenizer_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/parser/qxslttokenlookup_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/parser/qmaintainingreader.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/parser/qtokenlookup.cpp
)

target_sources(CsXmlPatterns
   PRIVATE
   ${CMAKE_CURRENT_SOURCE_DIR}/parser/qquerytransformparser.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/parser/qparsercontext.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/parser/qtokenrevealer.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/parser/qtokensource.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/parser/qxquerytokenizer.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/parser/qxslttokenizer.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/parser/qxslttokenlookup.cpp
)
