list(APPEND MULTIMEDIA_PUBLIC_INCLUDES
   QMediaBindableInterface
   QMediaControl
   QMediaObject
   QMediaMetaData
   QMediaService
   QMediaService_Provider_Plugin
   QMediaTimeRange
   QMultimedia
   QtMultimedia
)

list(APPEND MULTIMEDIA_INCLUDES
   ${CMAKE_CURRENT_SOURCE_DIR}/kernel/qmediabindableinterface.h
   ${CMAKE_CURRENT_SOURCE_DIR}/kernel/qmediacontrol.h
   ${CMAKE_CURRENT_SOURCE_DIR}/kernel/qmediaobject.h
   ${CMAKE_CURRENT_SOURCE_DIR}/kernel/qmediametadata.h
   ${CMAKE_CURRENT_SOURCE_DIR}/kernel/qmediaservice.h
   ${CMAKE_CURRENT_SOURCE_DIR}/kernel/qmediaservice_provider_plugin.h
   ${CMAKE_CURRENT_SOURCE_DIR}/kernel/qmediatimerange.h
   ${CMAKE_CURRENT_SOURCE_DIR}/kernel/qmultimedia.h
   ${CMAKE_CURRENT_SOURCE_DIR}/kernel/qtmultimedia.h
)

list(APPEND MULTIMEDIA_PRIVATE_INCLUDES
   ${CMAKE_CURRENT_SOURCE_DIR}/kernel/qmediacontrol_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/kernel/qmediaobject_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/kernel/qmediaopenglhelper_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/kernel/qmediaresourceset_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/kernel/qmediaservice_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/kernel/qmediaserviceprovider_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/kernel/qmediastoragelocation_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/kernel/qmediaresourcepolicy_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/kernel/qmediaresourcepolicyplugin_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/kernel/qmultimediautils_p.h
)

target_sources(CsMultimedia
   PRIVATE
   ${CMAKE_CURRENT_SOURCE_DIR}/kernel/qmediabindableinterface.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/kernel/qmediacontrol.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/kernel/qmediaobject.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/kernel/qmediametadata.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/kernel/qmediaresourceset.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/kernel/qmediaservice.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/kernel/qmediaserviceprovider.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/kernel/qmediatimerange.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/kernel/qmediastoragelocation.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/kernel/qmediaresourcepolicy.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/kernel/qmediaresourcepolicyplugin.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/kernel/qmultimediautils.cpp
)


