list(APPEND CORE_PUBLIC_INCLUDES
   QChar
   QChar32
   QString
   QString8
   QString16
   QStringParser
   QStringView
   QRegularExpression
)

if (CsString_FOUND)
   # use system headers

   if (CS_INSTALL_MODE STREQUAL "Package")
      # package mode, do not copy install headers

      target_link_libraries(CsCore
         PUBLIC
         CsString::CsString
      )

   elseif (CS_INSTALL_MODE STREQUAL "Deploy")

      if (CMAKE_SYSTEM_NAME MATCHES "Darwin")
         target_link_libraries(CsCore
            PUBLIC
            $<BUILD_INTERFACE:CsString::CsString>
            $<INSTALL_INTERFACE:${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}/libCsString.dylib>
         )

      elseif (CMAKE_SYSTEM_NAME MATCHES "(Linux|OpenBSD|FreeBSD|NetBSD|DragonFly)")
         target_link_libraries(CsCore
            PUBLIC
            $<BUILD_INTERFACE:CsString::CsString>
            $<INSTALL_INTERFACE:${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}/libCsString.so>
         )

      elseif (CMAKE_SYSTEM_NAME MATCHES "Windows")
         if (MSVC)
            target_link_libraries(CsCore
               PUBLIC
               $<BUILD_INTERFACE:CsString::CsString>

               # link with import library in CS install lib directory
               $<INSTALL_INTERFACE:${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}/libCsString.lib>
            )

         else()
            target_link_libraries(CsCore
               PUBLIC
               $<BUILD_INTERFACE:CsString::CsString>

               # link with import library in CS install lib directory
               $<INSTALL_INTERFACE:${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}/libCsString.dll.a>
            )
         endif()

      endif()

      list(APPEND CORE_INCLUDES
         ${CsString_INCLUDE_DIR}/cs_string.h
         ${CsString_INCLUDE_DIR}/cs_string_iterator.h
         ${CsString_INCLUDE_DIR}/cs_encoding.h
         ${CsString_INCLUDE_DIR}/cs_char.h
         ${CsString_INCLUDE_DIR}/cs_string_view.h
      )
   endif()

else()
   # use annex headers
   target_include_directories(CsCore
      PUBLIC
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/annex/cs_string>
   )

   list(APPEND CORE_INCLUDES
      ${CMAKE_CURRENT_SOURCE_DIR}/../annex/cs_string/cs_string.h
      ${CMAKE_CURRENT_SOURCE_DIR}/../annex/cs_string/cs_string_iterator.h
      ${CMAKE_CURRENT_SOURCE_DIR}/../annex/cs_string/cs_encoding.h
      ${CMAKE_CURRENT_SOURCE_DIR}/../annex/cs_string/cs_char.h
      ${CMAKE_CURRENT_SOURCE_DIR}/../annex/cs_string/cs_string_view.h
   )
endif()

list(APPEND CORE_INCLUDES
   ${CMAKE_CURRENT_SOURCE_DIR}/string/qchar.h
   ${CMAKE_CURRENT_SOURCE_DIR}/string/qchar32.h
   ${CMAKE_CURRENT_SOURCE_DIR}/string/qstring.h
   ${CMAKE_CURRENT_SOURCE_DIR}/string/qstring8.h
   ${CMAKE_CURRENT_SOURCE_DIR}/string/qstring16.h
   ${CMAKE_CURRENT_SOURCE_DIR}/string/qstringparser.h
   ${CMAKE_CURRENT_SOURCE_DIR}/string/qstringview.h
   ${CMAKE_CURRENT_SOURCE_DIR}/string/qregularexpression.h
   ${CMAKE_CURRENT_SOURCE_DIR}/string/qstringfwd.h
)

list(APPEND CORE_REGEX_INCLUDES
   ${CMAKE_CURRENT_SOURCE_DIR}/string/regex/basic_regex.h
   ${CMAKE_CURRENT_SOURCE_DIR}/string/regex/basic_regex_creator.h
   ${CMAKE_CURRENT_SOURCE_DIR}/string/regex/basic_regex_parser.h
   ${CMAKE_CURRENT_SOURCE_DIR}/string/regex/iterator_category.h
   ${CMAKE_CURRENT_SOURCE_DIR}/string/regex/match_flags.h
   ${CMAKE_CURRENT_SOURCE_DIR}/string/regex/match_results.h
   ${CMAKE_CURRENT_SOURCE_DIR}/string/regex/perl_matcher.h
   ${CMAKE_CURRENT_SOURCE_DIR}/string/regex/perl_matcher_common.h
   ${CMAKE_CURRENT_SOURCE_DIR}/string/regex/perl_matcher_non_recursive.h
   ${CMAKE_CURRENT_SOURCE_DIR}/string/regex/regex.h
   ${CMAKE_CURRENT_SOURCE_DIR}/string/regex/regex_constants.h
   ${CMAKE_CURRENT_SOURCE_DIR}/string/regex/regex_fwd.h
   ${CMAKE_CURRENT_SOURCE_DIR}/string/regex/regex_iterator.h
   ${CMAKE_CURRENT_SOURCE_DIR}/string/regex/regex_match.h
   ${CMAKE_CURRENT_SOURCE_DIR}/string/regex/regex_merge.h
   ${CMAKE_CURRENT_SOURCE_DIR}/string/regex/regex_raw_buffer.h
   ${CMAKE_CURRENT_SOURCE_DIR}/string/regex/regex_search.h
   ${CMAKE_CURRENT_SOURCE_DIR}/string/regex/regex_split.h
   ${CMAKE_CURRENT_SOURCE_DIR}/string/regex/regex_sub_match.h
   ${CMAKE_CURRENT_SOURCE_DIR}/string/regex/regex_token_iterator.h
   ${CMAKE_CURRENT_SOURCE_DIR}/string/regex/regex_traits.h
   ${CMAKE_CURRENT_SOURCE_DIR}/string/regex/regex_traits_defaults.h
   ${CMAKE_CURRENT_SOURCE_DIR}/string/regex/r_config.h
   ${CMAKE_CURRENT_SOURCE_DIR}/string/regex/r_error_type.h
   ${CMAKE_CURRENT_SOURCE_DIR}/string/regex/r_object_cache.h
   ${CMAKE_CURRENT_SOURCE_DIR}/string/regex/r_pattern_except.h
   ${CMAKE_CURRENT_SOURCE_DIR}/string/regex/r_protected_call.h
   ${CMAKE_CURRENT_SOURCE_DIR}/string/regex/r_states.h
)

target_sources(CsCore
   PRIVATE
   ${CMAKE_CURRENT_SOURCE_DIR}/string/qchar32.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/string/qstring8.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/string/qstring16.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/string/qstringparser.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/string/qstringview.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/string/regex/regex_raw_buffer.cpp
)

if(CMAKE_SYSTEM_NAME MATCHES "Darwin")
   target_sources(CsCore
      PRIVATE
      ${CMAKE_CURRENT_SOURCE_DIR}/string/qstring_mac.mm
   )
endif()
