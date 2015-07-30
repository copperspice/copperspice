# XXX: as bad as this can be it is neccessary for building properly when Qt4 is
# installed on the system because they have the same headers names with the
# difference being that CopperSpice does not install non-fancy headers into
# <prefix>/include.
set(CMAKE_INCLUDE_DIRECTORIES_BEFORE ON)

add_subdirectory(core)
# TODO: add_subdirectory(declarative)
add_subdirectory(gui)
add_subdirectory(multimedia)
add_subdirectory(network)
add_subdirectory(opengl)
add_subdirectory(phonon)
add_subdirectory(script)
# TODO: add_subdirectory(scripttools)
# TODO: add_subdirectory(sql)
add_subdirectory(svg)
# TODO: add_subdirectory(tools)
add_subdirectory(tools/rcc)
add_subdirectory(tools/uic)
if(WITH_WEBKIT)
    add_subdirectory(webkit)
endif()
add_subdirectory(xml)
add_subdirectory(xmlpatterns)