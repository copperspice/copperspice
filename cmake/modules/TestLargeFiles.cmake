# ***********************************************************************
#
# Copyright (c) 2012-2024 Barbara Geller
# Copyright (c) 2012-2024 Ansel Sermersheim
#
# This file is part of CopperSpice.
#
# CopperSpice is free software. You can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public License
# version 2.1 as published by the Free Software Foundation.
#
# CopperSpice is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
#
# https://www.gnu.org/licenses/
#
# ***********************************************************************

#  Adapted from Gromacs project (http://www.gromacs.org/) by Julien Malik
#
#  Define macro to check large file support:  OPJ_TEST_LARGE_FILES(VARIABLE)
#
#  VARIABLE will be set to true if off_t is 64 bits, and fseeko/ftello present.
#  This macro will also defines the necessary variable enable large file support, for instance
#     _LARGE_FILES
#     _LARGEFILE_SOURCE
#     _FILE_OFFSET_BITS 64
#     HAVE_FSEEKO
#
#  user is responsible for setting up a config.h file which contains "#cmakedefine" for each
#  macro which is required for your project

macro(OPJ_TEST_LARGE_FILES VARIABLE)

   set(TestFileOffsetBits

   "#include <sys/types.h>

   int main(int argc, char **argv)
   {
     /* Cause a compile-time error if off_t is smaller than 64 bits */

     #define LARGE_OFF_T (((off_t) 1 << 62) - 1 + ((off_t) 1 << 62))
     int off_t_is_large[ (LARGE_OFF_T % 2147483629 == 721 && LARGE_OFF_T % 2147483647 == 1) ? 1 : -1 ];
     return 0;
   }"

   )

   if(${VARIABLE} MATCHES "^${VARIABLE}$")

        # On most platforms it is probably overkill to first test the flags for 64-bit off_t,
        # and then separately fseeko. However, in the future we might have 128-bit filesystems
        # (ZFS), so it might be dangerous to indiscriminately set e.g. _FILE_OFFSET_BITS=64.

        message(STATUS "Checking for 64-bit off_t")

        # First check without any special flags
        check_cxx_source_compiles("${TestFileOffsetBits}" FILE64_OK)
        if(FILE64_OK)
          message(STATUS "Checking for 64-bit off_t - present")
       	endif()

        if(NOT FILE64_OK)
            # Test with _FILE_OFFSET_BITS=64
            check_cxx_source_compiles("${TestFileOffsetBits}" FILE64_OK
                        COMPILE_DEFINITIONS "-D_FILE_OFFSET_BITS=64")
            if(FILE64_OK)
                message(STATUS "Checking for 64-bit off_t - present with _FILE_OFFSET_BITS=64")
                set(_FILE_OFFSET_BITS 64)
            endif()
        endif()

        if(NOT FILE64_OK)
            # Test with _LARGE_FILES
            check_cxx_source_compiles("${TestFileOffsetBits}" FILE64_OK
                        COMPILE_DEFINITIONS "-D_LARGE_FILES" )
            if(FILE64_OK)
                message(STATUS "Checking for 64-bit off_t - present with _LARGE_FILES")
                set(_LARGE_FILES 1)
            endif()
        endif()
	
        if(NOT FILE64_OK)
            # Test with _LARGEFILE_SOURCE
            check_cxx_source_compiles("${TestFileOffsetBits}" FILE64_OK
                        COMPILE_DEFINITIONS "-D_LARGEFILE_SOURCE" )
            if(FILE64_OK)
                message(STATUS "Checking for 64-bit off_t - present with _LARGEFILE_SOURCE")
                set(_LARGEFILE_SOURCE 1)
            endif()
        endif()


        #if(NOT FILE64_OK)
        #    # now check for Windows stuff
        #    try_compile(FILE64_OK "${PROJECT_BINARY_DIR}"
        #                "${PROJECT_SOURCE_DIR}/cmake/TestWindowsFSeek.c")
        #    if(FILE64_OK)
        #        message(STATUS "Checking for 64-bit off_t - present with _fseeki64")
        #        set(HAVE__FSEEKI64 1)
        #    endif()
        #endif()

        if(NOT FILE64_OK)
            message(STATUS "Checking for 64-bit off_t - not present")
        endif()

        set(_FILE_OFFSET_BITS ${_FILE_OFFSET_BITS} CACHE INTERNAL "Result of test for needed _FILE_OFFSET_BITS=64")
        set(_LARGE_FILES      ${_LARGE_FILES}      CACHE INTERNAL "Result of test for needed _LARGE_FILES")
        set(_LARGEFILE_SOURCE ${_LARGEFILE_SOURCE} CACHE INTERNAL "Result of test for needed _LARGEFILE_SOURCE")

        message(STATUS "Checking for fseeko/ftello")

        # Test if ftello/fseeko are	available
        check_cxx_source_compiles("${TestFileOffsetBits}" FSEEKO_COMPILE_OK)

        if(FSEEKO_COMPILE_OK)
            message(STATUS "Checking for fseeko/ftello - present")
        endif()

        if(NOT FSEEKO_COMPILE_OK)
                # glibc 2.2 needs _LARGEFILE_SOURCE for fseeko (but not for 64-bit off_t...)
                check_cxx_source_compiles("${TestFileOffsetBits}"
                            FSEEKO_COMPILE_OK
                            COMPILE_DEFINITIONS "-D_LARGEFILE_SOURCE" )

                if(FSEEKO_COMPILE_OK)
                    message(STATUS "Checking for fseeko/ftello - present with _LARGEFILE_SOURCE")
                    set(_LARGEFILE_SOURCE ${_LARGEFILE_SOURCE} CACHE INTERNAL "Result of test for needed _LARGEFILE_SOURCE")
                endif()
        endif()

	    if(FSEEKO_COMPILE_OK)
                set(HAVE_FSEEKO ON CACHE INTERNAL "Result of test for fseeko/ftello")
        else()
                message(STATUS "Checking for fseeko/ftello - not found")
                set(HAVE_FSEEKO OFF CACHE INTERNAL "Result of test for fseeko/ftello")
        endif()

	    if(FILE64_OK AND FSEEKO_COMPILE_OK)
                message(STATUS "Large File support - found")
                set(${VARIABLE} ON CACHE INTERNAL "Result of test for large file support")
        else()
                message(STATUS "Large File support - not found")
                set(${VARIABLE} OFF CACHE INTERNAL "Result of test for large file support")
        endif()

    endif()
endmacro()
