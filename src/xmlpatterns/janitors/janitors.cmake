list(APPEND XMLPATTERNS_PRIVATE_INCLUDES
   ${CMAKE_CURRENT_SOURCE_DIR}/janitors/qargumentconverter_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/janitors/qatomizer_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/janitors/qcardinalityverifier_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/janitors/qebvextractor_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/janitors/qitemverifier_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/janitors/quntypedatomicconverter_p.h
)

target_sources(CsXmlPatterns
   PRIVATE
   ${CMAKE_CURRENT_SOURCE_DIR}/janitors/qargumentconverter.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/janitors/qatomizer.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/janitors/qcardinalityverifier.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/janitors/qebvextractor.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/janitors/qitemverifier.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/janitors/quntypedatomicconverter.cpp
)
