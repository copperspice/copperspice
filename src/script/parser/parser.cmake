set(SCRIPT_PRIVATE_INCLUDES
    ${SCRIPT_PRIVATE_INCLUDES}
    ${CMAKE_CURRENT_SOURCE_DIR}/parser/qscriptastfwd_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/parser/qscriptast_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/parser/qscriptastvisitor_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/parser/qscriptgrammar_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/parser/qscriptsyntaxchecker_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/parser/qscriptlexer_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/parser/qscriptparser_p.h
)

set(SCRIPT_SOURCES
    ${SCRIPT_SOURCES}
    ${CMAKE_CURRENT_SOURCE_DIR}/parser/qscriptast.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/parser/qscriptastvisitor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/parser/qscriptgrammar.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/parser/qscriptsyntaxchecker.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/parser/qscriptlexer.cpp

#   ${CMAKE_CURRENT_SOURCE_DIR}/parser/qscriptparser.cpp
)
 
