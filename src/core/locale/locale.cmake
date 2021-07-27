list(APPEND CORE_PUBLIC_INCLUDES
   QLocale
   QSystemLocale
)

list(APPEND CORE_INCLUDES
   ${CMAKE_CURRENT_SOURCE_DIR}/locale/qlocale.h
   ${CMAKE_CURRENT_SOURCE_DIR}/locale/qsystemlocale.h
)

list(APPEND CORE_PRIVATE_INCLUDES
   ${CMAKE_CURRENT_SOURCE_DIR}/locale/qlocale_data_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/locale/qlocale_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/locale/qlocale_tools_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/locale/qunicodetables_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/locale/qunicodetools_p.h
)

target_sources(CsCore
   PRIVATE
   ${CMAKE_CURRENT_SOURCE_DIR}/locale/qlocale.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/locale/qlocale_tools.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/locale/qunicodetools.cpp
)

if(CMAKE_SYSTEM_NAME MATCHES "Darwin")
   target_sources(CsCore
      PRIVATE
      ${CMAKE_CURRENT_SOURCE_DIR}/locale/qlocale_mac.mm
    )

elseif(CMAKE_SYSTEM_NAME MATCHES "(Linux|OpenBSD|FreeBSD|NetBSD)")
   target_sources(CsCore
      PRIVATE
      ${CMAKE_CURRENT_SOURCE_DIR}/locale/qlocale_unix.cpp
   )

elseif(CMAKE_SYSTEM_NAME MATCHES "Windows")
   target_sources(CsCore
      PRIVATE
      ${CMAKE_CURRENT_SOURCE_DIR}/locale/qlocale_win.cpp
   )

endif()
