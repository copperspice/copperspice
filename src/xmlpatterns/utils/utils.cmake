list(APPEND XMLPATTERNS_PRIVATE_INCLUDES
   ${CMAKE_CURRENT_SOURCE_DIR}/utils/qcommonnamespaces_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/utils/qcppcastinghelper_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/utils/qxmlpatterns_debug_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/utils/qgenericnamespaceresolver_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/utils/qpatternistlocale_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/utils/qnamepool_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/utils/qnamespaceresolver_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/utils/qoutputvalidator_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/utils/qxpathhelper_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/utils/qdelegatingnamespaceresolver_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/utils/qnodenamespaceresolver_p.h
)

target_sources(CsXmlPatterns
   PRIVATE
   ${CMAKE_CURRENT_SOURCE_DIR}/utils/qgenericnamespaceresolver.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/utils/qpatternistlocale.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/utils/qnamepool.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/utils/qnamespaceresolver.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/utils/qoutputvalidator.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/utils/qxpathhelper.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/utils/qdelegatingnamespaceresolver.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/utils/qnodenamespaceresolver.cpp
)
