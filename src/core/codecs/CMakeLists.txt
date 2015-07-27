set(CORE_PUBLIC_INCLUDES
    ${CORE_PUBLIC_INCLUDES}
    QTextCodec
    QTextCodecPlugin
    QTextDecoder
    QTextEncoder
    QTextCodecFactoryInterface
    PARENT_SCOPE
)

set(CORE_INCLUDES
    ${CORE_INCLUDES}
    ${CMAKE_CURRENT_SOURCE_DIR}/qtextcodec.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qtextdecoder.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qtextencoder.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qtextcodecfactoryinterface.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qtextcodecplugin.h
    PARENT_SCOPE
)

set(CORE_PRIVATE_INCLUDES
    ${CORE_PRIVATE_INCLUDES}
    ${CMAKE_CURRENT_SOURCE_DIR}/qisciicodec_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qlatincodec_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qsimplecodec_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qtextcodec_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qtsciicodec_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qutfcodec_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qiconvcodec_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qfontlaocodec_p.h
    PARENT_SCOPE
)

set(CORE_SOURCES
    ${CORE_SOURCES}
    ${CMAKE_CURRENT_SOURCE_DIR}/qisciicodec.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qlatincodec.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qsimplecodec.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qtextcodec.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qtextcodecplugin.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qtsciicodec.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qutfcodec.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qfontlaocodec.cpp
    PARENT_SCOPE
)

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(CORE_SOURCES
        ${CORE_SOURCES}
        ${CMAKE_CURRENT_SOURCE_DIR}/qiconvcodec.cpp
        PARENT_SCOPE
    )
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(CORE_SOURCES
        ${CORE_SOURCES}
        ${CMAKE_CURRENT_SOURCE_DIR}/qiconvcodec.cpp
        PARENT_SCOPE
    )
endif()

