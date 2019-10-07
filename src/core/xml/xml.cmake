list(APPEND CORE_PUBLIC_INCLUDES
   QXmlStreamAttribute
   QXmlStreamAttributes
   QXmlStreamEntityDeclaration
   QXmlStreamEntityDeclarations
   QXmlStreamEntityResolver
   QXmlStreamNamespaceDeclaration
   QXmlStreamNamespaceDeclarations
   QXmlStreamNotationDeclaration
   QXmlStreamNotationDeclarations
   QXmlStreamReader
   QXmlStreamWriter
   QXmlStream
)

list(APPEND CORE_INCLUDES
   ${CMAKE_CURRENT_SOURCE_DIR}/xml/qxmlstreamattribute.h
   ${CMAKE_CURRENT_SOURCE_DIR}/xml/qxmlstreamattributes.h
   ${CMAKE_CURRENT_SOURCE_DIR}/xml/qxmlstreamentitydeclaration.h
   ${CMAKE_CURRENT_SOURCE_DIR}/xml/qxmlstreamentitydeclarations.h
   ${CMAKE_CURRENT_SOURCE_DIR}/xml/qxmlstreamentityresolver.h
   ${CMAKE_CURRENT_SOURCE_DIR}/xml/qxmlstreamnamespacedeclaration.h
   ${CMAKE_CURRENT_SOURCE_DIR}/xml/qxmlstreamnamespacedeclarations.h
   ${CMAKE_CURRENT_SOURCE_DIR}/xml/qxmlstreamnotationdeclaration.h
   ${CMAKE_CURRENT_SOURCE_DIR}/xml/qxmlstreamnotationdeclarations.h
   ${CMAKE_CURRENT_SOURCE_DIR}/xml/qxmlstreamreader.h
   ${CMAKE_CURRENT_SOURCE_DIR}/xml/qxmlstreamwriter.h
   ${CMAKE_CURRENT_SOURCE_DIR}/xml/qxmlstream.h
)

list(APPEND CORE_PRIVATE_INCLUDES
   ${CMAKE_CURRENT_SOURCE_DIR}/xml/qxmlstream_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/xml/qxmlutils_p.h
)

target_sources(CsCore
   PRIVATE
   ${CMAKE_CURRENT_SOURCE_DIR}/xml/qxmlstream.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/xml/qxmlutils.cpp
)
