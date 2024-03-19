list(APPEND CORE_PUBLIC_INCLUDES
   QFactoryInterface
   QLibrary
   QPluginLoader
   QUuid
)

list(APPEND CORE_INCLUDES
   ${CMAKE_CURRENT_SOURCE_DIR}/plugin/qfactoryinterface.h
   ${CMAKE_CURRENT_SOURCE_DIR}/plugin/qlibrary.h
   ${CMAKE_CURRENT_SOURCE_DIR}/plugin/qplugin.h
   ${CMAKE_CURRENT_SOURCE_DIR}/plugin/qpluginloader.h
   ${CMAKE_CURRENT_SOURCE_DIR}/plugin/quuid.h
)

list(APPEND CORE_PRIVATE_INCLUDES
   ${CMAKE_CURRENT_SOURCE_DIR}/plugin/qfactoryloader_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/plugin/qlibrary_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/plugin/qsystemlibrary_p.h
)

target_sources(CsCore
   PRIVATE
   ${CMAKE_CURRENT_SOURCE_DIR}/plugin/qfactoryloader.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/plugin/qfactoryinterface.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/plugin/qlibrary.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/plugin/qpluginloader.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/plugin/quuid.cpp
)

if(CMAKE_SYSTEM_NAME MATCHES "Windows")
   target_sources(CsCore
      PRIVATE
      ${CMAKE_CURRENT_SOURCE_DIR}/plugin/qlibrary_win.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/plugin/qsystemlibrary.cpp
   )

elseif(CMAKE_SYSTEM_NAME MATCHES "(Linux|Darwin|OpenBSD|FreeBSD|NetBSD)")
   target_sources(CsCore
      PRIVATE
      ${CMAKE_CURRENT_SOURCE_DIR}/plugin/qlibrary_unix.cpp
  )
endif()

