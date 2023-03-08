list(APPEND IMAGEFORMATS_SVG_PRIVATE_INCLUDES
   ${CMAKE_SOURCE_DIR}/src/plugins/imageformats/svg/qsvgiohandler.h
)

if(WITH_SVG)
   add_library(CsImageFormatsSvg MODULE "")
   add_library(CopperSpice::CsImageFormatsSvg ALIAS CsImageFormatsSvg)

   set_target_properties(CsImageFormatsSvg PROPERTIES OUTPUT_NAME CsImageFormatsSvg${BUILD_ABI} PREFIX "")

   target_sources(CsImageFormatsSvg
      PRIVATE
      ${CMAKE_SOURCE_DIR}/src/plugins/imageformats/svg/main.cpp
      ${CMAKE_SOURCE_DIR}/src/plugins/imageformats/svg/qsvgiohandler.cpp
   )

   target_link_libraries(CsImageFormatsSvg
      CsCore
      CsGui
      CsSvg
   )

   target_compile_definitions(CsImageFormatsSvg
      PRIVATE
      -DQT_PLUGIN
   )

   function_generate_resources(CsImageFormatsSvg)


   if(BUILDING_RPM)
      install(TARGETS CsImageFormatsSvg DESTINATION ${CMAKE_INSTALL_LIBDIR}/copperspice/plugins/imageformats)
   else()
      install(TARGETS CsImageFormatsSvg DESTINATION ${CMAKE_INSTALL_LIBDIR})
   endif()
endif()

