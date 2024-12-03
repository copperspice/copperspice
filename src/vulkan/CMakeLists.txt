add_library(CsVulkan SHARED "")
add_library(CopperSpice::CsVulkan ALIAS CsVulkan)

set_target_properties(CsVulkan PROPERTIES OUTPUT_NAME CsVulkan${BUILD_ABI})

target_compile_definitions(CsVulkan
   PRIVATE
   -DQT_BUILD_VULKAN_LIB
)

set(EXTRA_VULKAN_CXXFLAGS)
set(EXTRA_VULKAN_LDFLAGS)

target_include_directories(CsVulkan
   PUBLIC
   $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/include/QtVulkan>
   $<INSTALL_INTERFACE:include/QtVulkan>
)

include_directories(
   ${CMAKE_BINARY_DIR}/privateinclude/QtCore/private
   ${CMAKE_BINARY_DIR}/privateinclude/QtGui/private
   ${CMAKE_BINARY_DIR}/privateinclude/QtVulkan/private

   ${CMAKE_CURRENT_SOURCE_DIR}
)

list(APPEND VULKAN_PUBLIC_INCLUDES
   QVulkanDeviceFunctions
   QVulkanFunctions
   QVulkanInstance
   QVulkanWindow
   QVulkanWindowRenderer
)

list(APPEND VULKAN_INCLUDES
   ${CMAKE_CURRENT_SOURCE_DIR}/qvulkandevicefunctions.h
   ${CMAKE_CURRENT_SOURCE_DIR}/qvulkanfunctions.h
   ${CMAKE_CURRENT_SOURCE_DIR}/qvulkaninstance.h
   ${CMAKE_CURRENT_SOURCE_DIR}/qvulkanwindow.h
   ${CMAKE_CURRENT_SOURCE_DIR}/qvulkanwindowrenderer.h

   ${CMAKE_CURRENT_SOURCE_DIR}/qvulkan_device_functions.h
   ${CMAKE_CURRENT_SOURCE_DIR}/qvulkan_functions.h
   ${CMAKE_CURRENT_SOURCE_DIR}/qvulkan_instance.h
   ${CMAKE_CURRENT_SOURCE_DIR}/qvulkan_window.h
   ${CMAKE_CURRENT_SOURCE_DIR}/qvulkan_window_renderer.h
)

target_sources(CsVulkan
   PRIVATE
   ${CMAKE_CURRENT_SOURCE_DIR}/qvulkan_device_functions.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/qvulkan_functions.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/qvulkan_instance.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/qvulkan_window.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/qvulkan_window_renderer.cpp
)

macro_generate_public("${VULKAN_PUBLIC_INCLUDES}" QtVulkan)
macro_generate_private("${VULKAN_PRIVATE_INCLUDES}" QtVulkan)
macro_generate_misc("${VULKAN_INCLUDES}" QtVulkan)

function_variable_fixup("${EXTRA_VULKAN_CXXFLAGS}" EXTRA_VULKAN_CXXFLAGS)
function_variable_fixup("${EXTRA_VULKAN_LDFLAGS}"  EXTRA_VULKAN_LDFLAGS)

target_link_libraries(CsVulkan
   PUBLIC
   CsCore
   CsGui
   Vulkan::Vulkan
)

target_compile_options(CsVulkan
   PRIVATE
   $<$<COMPILE_LANGUAGE:CXX>:SHELL:${EXTRA_VULKAN_CXXFLAGS}>
)

set_target_properties(CsVulkan
   PROPERTIES
   LINK_FLAGS ${EXTRA_VULKAN_LDFLAGS}
)

if(CMAKE_SYSTEM_NAME MATCHES "(Linux|OpenBSD|FreeBSD|NetBSD|DragonFly)")
  target_compile_definitions(CsVulkan
    PUBLIC
    -DVK_USE_PLATFORM_XCB_KHR
  )

  target_link_libraries(CsVulkan
      PRIVATE
      ${XCB_LIB}
  )

else()
  target_compile_definitions(CsVulkan
     PUBLIC
     -DVK_USE_PLATFORM_WIN32_KHR
  )

endif()

function_generate_resources(CsVulkan)

macro_generate_package(
   CsVulkan
   QtVulkan
   ""
   "CsCore CsGui"
)

install(
   TARGETS CsVulkan
   EXPORT CopperSpiceLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS}
   RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
   LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
   ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(
   DIRECTORY ${CMAKE_BINARY_DIR}/include/QtVulkan
   DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
   COMPONENT Devel
)
