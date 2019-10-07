list(APPEND CORE_PUBLIC_INCLUDES
   QTextCodec
   QTextCodecPlugin
   QTextDecoder
   QTextEncoder
   QTextCodecFactoryInterface
)

list(APPEND CORE_INCLUDES
   ${CMAKE_CURRENT_SOURCE_DIR}/codecs/qtextcodec.h
   ${CMAKE_CURRENT_SOURCE_DIR}/codecs/qtextdecoder.h
   ${CMAKE_CURRENT_SOURCE_DIR}/codecs/qtextencoder.h
   ${CMAKE_CURRENT_SOURCE_DIR}/codecs/qtextcodecfactoryinterface.h
   ${CMAKE_CURRENT_SOURCE_DIR}/codecs/qtextcodecplugin.h
   )

list(APPEND CORE_PRIVATE_INCLUDES
   ${CMAKE_CURRENT_SOURCE_DIR}/codecs/qisciicodec_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/codecs/qlatincodec_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/codecs/qsimplecodec_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/codecs/qtextcodec_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/codecs/qutfcodec_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/codecs/qiconvcodec_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/codecs/qfontlaocodec_p.h
)

target_sources(CsCore
   PRIVATE
   ${CMAKE_CURRENT_SOURCE_DIR}/codecs/qisciicodec.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/codecs/qlatincodec.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/codecs/qsimplecodec.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/codecs/qtextcodec.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/codecs/qtextcodecplugin.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/codecs/qutfcodec.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/codecs/qfontlaocodec.cpp
)

if(CMAKE_SYSTEM_NAME MATCHES "(Linux|Darwin|OpenBSD|FreeBSD|NetBSD)")
   target_sources(CsCore
      PRIVATE
      ${CMAKE_CURRENT_SOURCE_DIR}/codecs/qiconvcodec.cpp
   )

   if(NOT CMAKE_SYSTEM_NAME MATCHES "Linux")
      target_link_libraries(CsCore
         PRIVATE
         ${ICONV_LIBRARIES}
         ${CMAKE_THREAD_LIBS_INIT}
      )

      target_include_directories(CsCore
         PRIVATE
         ${ICONV_INCLUDE_DIR}
      )
    endif()
endif()
