set(MULTIMEDIA_PUBLIC_INCLUDES
    ${MULTIMEDIA_PUBLIC_INCLUDES}
    QtMultimedia
)

set(MULTIMEDIA_INCLUDES
    ${MULTIMEDIA_INCLUDES}
    ${CMAKE_CURRENT_SOURCE_DIR}/kernel/qmultimedia.h
    ${CMAKE_CURRENT_SOURCE_DIR}/kernel/qmediaservice_provider_plugin.h
    ${CMAKE_CURRENT_SOURCE_DIR}/kernel/qtmultimedia.h
)

set(MULTIMEDIA_PRIVATE_INCLUDES
    ${MULTIMEDIA_PRIVATE_INCLUDES}
    ${CMAKE_CURRENT_SOURCE_DIR}/kernel/qmediapluginloader_p.h
)

set(MULTIMEDIA_SOURCES
   ${MULTIMEDIA_SOURCES}
   ${CMAKE_CURRENT_SOURCE_DIR}/kernel/qmediapluginloader.cpp
)


