list(APPEND CORE_PUBLIC_INCLUDES
   QJsonArray
   QJsonDocument
   QJsonObject
   QJsonParseError
   QJsonValue
)

list(APPEND CORE_INCLUDES
   ${CMAKE_CURRENT_SOURCE_DIR}/json/qjson.h
   ${CMAKE_CURRENT_SOURCE_DIR}/json/qjsonarray.h
   ${CMAKE_CURRENT_SOURCE_DIR}/json/qjsondocument.h
   ${CMAKE_CURRENT_SOURCE_DIR}/json/qjsonobject.h
   ${CMAKE_CURRENT_SOURCE_DIR}/json/qjsonparseerror.h
   ${CMAKE_CURRENT_SOURCE_DIR}/json/qjsonvalue.h
)

list(APPEND CORE_PRIVATE_INCLUDES
   ${CMAKE_CURRENT_SOURCE_DIR}/json/qjsonparser_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/json/qjsonwriter_p.h
)

target_sources(CsCore
   PRIVATE
   ${CMAKE_CURRENT_SOURCE_DIR}/json/qjson.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/json/qjsonarray.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/json/qjsondocument.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/json/qjsonobject.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/json/qjsonparser.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/json/qjsonvalue.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/json/qjsonwriter.cpp
)
