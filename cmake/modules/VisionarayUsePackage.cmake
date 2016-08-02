#---------------------------------------------------------------------------------------------------
# visionaray_use_package(name)
#

function(visionaray_use_package name)
    string(TOUPPER ${name} upper_name)

    if(NOT ${name}_FOUND AND NOT ${upper_name}_FOUND)
        return()
    endif()

    #
    # search for cmake variables in the following order:
    #  name_INCLUDE_DIR, NAME_INCLUDE_DIR, name_INCLUDE_DIRS, NAME_INCLUDE_DIRS
    if(NOT pkg_INCDIRS)
        if(${name}_INCLUDE_DIR)
            set(pkg_INCDIRS ${${name}_INCLUDE_DIR})
        elseif(${upper_name}_INCLUDE_DIR)
            set(pkg_INCDIRS ${${upper_name}_INCLUDE_DIR})
        elseif(${name}_INCLUDE_DIRS)
            set(pkg_INCDIRS ${${name}_INCLUDE_DIRS})
        elseif(${upper_name}_INCLUDE_DIRS)
            set(pkg_INCDIRS ${${upper_name}_INCLUDE_DIRS})
        endif()
    endif()

    include_directories(SYSTEM ${pkg_INCDIRS})

    #
    # search for cmake variables in the following order:
    #  name_LIBRARIES, NAME_LIBRARIES, name_LIBRARY, NAME_LIBRARY
    #
    if(NOT pkg_LIBS)
        if(${name}_LIBRARIES)
            set(pkg_LIBS ${${name}_LIBRARIES})
        elseif(${upper_name}_LIBRARIES})
            set(pkg_LIBS ${${upper_name}_LIBRARIES})
        elseif(${name}_LIBRARY)
            set(pkg_LIBS ${${name}_LIBRARY})
        elseif(${upper_name}_LIBRARY)
            set(pkg_LIBS ${${upper_name}_LIBRARY})
        endif()
    endif()

    #
    # some special treatment for CMake Threads meta package..
    #
    if(${upper_name} STREQUAL "THREADS")
        set(pkg_LIBS ${CMAKE_THREAD_LIBS_INIT} ${pkg_LIBS})
    endif()

    set(__VSNRAY_LINK_LIBRARIES ${__VSNRAY_LINK_LIBRARIES} ${pkg_LIBS} PARENT_SCOPE)

    add_definitions(-DVSNRAY_HAVE_${upper_name}=1)
endfunction()
