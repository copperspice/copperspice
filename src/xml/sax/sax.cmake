list(APPEND XML_PUBLIC_INCLUDES
    QXmlAttributes
    QXmlContentHandler
    QXmlDTDHandler
    QXmlDeclHandler
    QXmlDefaultHandler
    QXmlEntityResolver
    QXmlErrorHandler
    QXmlInputSource
    QXmlLexicalHandler
    QXmlLocator
    QXmlNamespaceSupport
    QXmlParseException
    QXmlReader
    QXmlSimpleReader
)

list(APPEND XML_INCLUDES
    ${CMAKE_CURRENT_SOURCE_DIR}/sax/qxml.h
    ${CMAKE_CURRENT_SOURCE_DIR}/sax/qxmlattributes.h
    ${CMAKE_CURRENT_SOURCE_DIR}/sax/qxmlcontenthandler.h
    ${CMAKE_CURRENT_SOURCE_DIR}/sax/qxmldtdhandler.h
    ${CMAKE_CURRENT_SOURCE_DIR}/sax/qxmldeclhandler.h
    ${CMAKE_CURRENT_SOURCE_DIR}/sax/qxmldefaulthandler.h
    ${CMAKE_CURRENT_SOURCE_DIR}/sax/qxmlentityresolver.h
    ${CMAKE_CURRENT_SOURCE_DIR}/sax/qxmlerrorhandler.h
    ${CMAKE_CURRENT_SOURCE_DIR}/sax/qxmlinputsource.h
    ${CMAKE_CURRENT_SOURCE_DIR}/sax/qxmllexicalhandler.h
    ${CMAKE_CURRENT_SOURCE_DIR}/sax/qxmllocator.h
    ${CMAKE_CURRENT_SOURCE_DIR}/sax/qxmlnamespacesupport.h
    ${CMAKE_CURRENT_SOURCE_DIR}/sax/qxmlparseexception.h
    ${CMAKE_CURRENT_SOURCE_DIR}/sax/qxmlreader.h
    ${CMAKE_CURRENT_SOURCE_DIR}/sax/qxmlsimplereader.h
)

target_sources(CsXml
   PRIVATE
   ${CMAKE_CURRENT_SOURCE_DIR}/sax/qxml.cpp
)
