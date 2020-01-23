list(APPEND SCRIPT_PUBLIC_INCLUDES
   QScriptable
   QScriptClass
   QScriptClassPropertyIterator
   QScriptContext
   QScriptContextInfo
   QScriptEngine
   QScriptEngineAgent
   QScriptExtensionInterface
   QScriptExtensionPlugin
   QScriptProgram
   QScriptString
   QScriptValue
   QScriptValueIterator
   QScriptContextInfoList
   QScriptSyntaxCheckResult
   QtScript
)

list(APPEND SCRIPT_INCLUDES
   ${CMAKE_CURRENT_SOURCE_DIR}/api/qscriptable.h
   ${CMAKE_CURRENT_SOURCE_DIR}/api/qscriptclass.h
   ${CMAKE_CURRENT_SOURCE_DIR}/api/qscriptclasspropertyiterator.h
   ${CMAKE_CURRENT_SOURCE_DIR}/api/qscriptcontext.h
   ${CMAKE_CURRENT_SOURCE_DIR}/api/qscriptcontextinfo.h
   ${CMAKE_CURRENT_SOURCE_DIR}/api/qscriptcontextinfolist.h
   ${CMAKE_CURRENT_SOURCE_DIR}/api/qscriptengine.h
   ${CMAKE_CURRENT_SOURCE_DIR}/api/qscriptengineagent.h
   ${CMAKE_CURRENT_SOURCE_DIR}/api/qscriptextensioninterface.h
   ${CMAKE_CURRENT_SOURCE_DIR}/api/qscriptextensionplugin.h
   ${CMAKE_CURRENT_SOURCE_DIR}/api/qscriptprogram.h
   ${CMAKE_CURRENT_SOURCE_DIR}/api/qscriptstring.h
   ${CMAKE_CURRENT_SOURCE_DIR}/api/qscriptsyntaxcheckresult.h
   ${CMAKE_CURRENT_SOURCE_DIR}/api/qscriptvalue.h
   ${CMAKE_CURRENT_SOURCE_DIR}/api/qscriptvalueiterator.h
   ${CMAKE_CURRENT_SOURCE_DIR}/api/qtscript.h
)

list(APPEND SCRIPT_PRIVATE_INCLUDES
   ${CMAKE_CURRENT_SOURCE_DIR}/api/qscriptable_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/api/qscriptcontext_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/api/qscriptengine_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/api/qscriptengineagent_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/api/qscriptprogram_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/api/qscriptstring_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/api/qscriptvalue_p.h
)

target_sources(CsScript
   PRIVATE
   ${CMAKE_CURRENT_SOURCE_DIR}/api/qscriptclass.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/api/qscriptclasspropertyiterator.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/api/qscriptcontext.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/api/qscriptcontextinfo.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/api/qscriptengine.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/api/qscriptengineagent.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/api/qscriptextensionplugin.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/api/qscriptprogram.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/api/qscriptstring.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/api/qscriptvalue.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/api/qscriptvalueiterator.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/api/qscriptable.cpp
)
