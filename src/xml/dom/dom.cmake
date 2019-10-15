list(APPEND XML_PUBLIC_INCLUDES
   QDomAttr
   QDomCDATASection
   QDomCharacterData
   QDomComment
   QDomDocument
   QDomDocumentFragment
   QDomDocumentType
   QDomElement
   QDomEntity
   QDomEntityReference
   QDomImplementation
   QDomNamedNodeMap
   QDomNode
   QDomNodeList
   QDomNotation
   QDomProcessingInstruction
   QDomText
)

list(APPEND XML_INCLUDES
   ${CMAKE_CURRENT_SOURCE_DIR}/dom/qdom.h
   ${CMAKE_CURRENT_SOURCE_DIR}/dom/qdomattr.h
   ${CMAKE_CURRENT_SOURCE_DIR}/dom/qdomcdatasection.h
   ${CMAKE_CURRENT_SOURCE_DIR}/dom/qdomcharacterdata.h
   ${CMAKE_CURRENT_SOURCE_DIR}/dom/qdomcomment.h
   ${CMAKE_CURRENT_SOURCE_DIR}/dom/qdomdocument.h
   ${CMAKE_CURRENT_SOURCE_DIR}/dom/qdomdocumentfragment.h
   ${CMAKE_CURRENT_SOURCE_DIR}/dom/qdomdocumenttype.h
   ${CMAKE_CURRENT_SOURCE_DIR}/dom/qdomelement.h
   ${CMAKE_CURRENT_SOURCE_DIR}/dom/qdomentity.h
   ${CMAKE_CURRENT_SOURCE_DIR}/dom/qdomentityreference.h
   ${CMAKE_CURRENT_SOURCE_DIR}/dom/qdomimplementation.h
   ${CMAKE_CURRENT_SOURCE_DIR}/dom/qdomnamednodemap.h
   ${CMAKE_CURRENT_SOURCE_DIR}/dom/qdomnode.h
   ${CMAKE_CURRENT_SOURCE_DIR}/dom/qdomnodelist.h
   ${CMAKE_CURRENT_SOURCE_DIR}/dom/qdomnotation.h
   ${CMAKE_CURRENT_SOURCE_DIR}/dom/qdomprocessinginstruction.h
   ${CMAKE_CURRENT_SOURCE_DIR}/dom/qdomtext.h
)

target_sources(CsXml
   PRIVATE
   ${CMAKE_CURRENT_SOURCE_DIR}/dom/qdom.cpp
)
