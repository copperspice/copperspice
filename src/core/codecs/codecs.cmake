set(CORE_PUBLIC_INCLUDES
    ${CORE_PUBLIC_INCLUDES}
    QTextCodec
    QTextCodecPlugin
    QTextDecoder
    QTextEncoder
    QTextCodecFactoryInterface
)

set(CORE_INCLUDES
    ${CORE_INCLUDES}
    ${CMAKE_CURRENT_SOURCE_DIR}/codecs/qtextcodec.h
    ${CMAKE_CURRENT_SOURCE_DIR}/codecs/qtextdecoder.h
    ${CMAKE_CURRENT_SOURCE_DIR}/codecs/qtextencoder.h
    ${CMAKE_CURRENT_SOURCE_DIR}/codecs/qtextcodecfactoryinterface.h
    ${CMAKE_CURRENT_SOURCE_DIR}/codecs/qtextcodecplugin.h
)

set(CORE_PRIVATE_INCLUDES
    ${CORE_PRIVATE_INCLUDES}
    ${CMAKE_CURRENT_SOURCE_DIR}/codecs/qisciicodec_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/codecs/qlatincodec_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/codecs/qsimplecodec_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/codecs/qtextcodec_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/codecs/qutfcodec_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/codecs/qiconvcodec_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/codecs/qfontlaocodec_p.h
)

set(CORE_SOURCES
    ${CORE_SOURCES}
    ${CMAKE_CURRENT_SOURCE_DIR}/codecs/qisciicodec.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/codecs/qlatincodec.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/codecs/qsimplecodec.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/codecs/qtextcodec.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/codecs/qtextcodecplugin.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/codecs/qutfcodec.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/codecs/qfontlaocodec.cpp
)

if(${CMAKE_SYSTEM_NAME} MATCHES "(Linux|Darwin|OpenBSD|FreeBSD|NetBSD)")
    set(CORE_SOURCES
        ${CORE_SOURCES}
        ${CMAKE_CURRENT_SOURCE_DIR}/codecs/qiconvcodec.cpp
    )
    if(NOT ${CMAKE_SYSTEM_NAME} MATCHES "Linux")
        set(EXTRA_CORE_LIBS
            ${EXTRA_CORE_LIBS}
            ${ICONV_LIBRARIES}
            ${CMAKE_THREAD_LIBS_INIT}
        )
        include_directories(${ICONV_INCLUDE_DIR})
    endif()
endif()
