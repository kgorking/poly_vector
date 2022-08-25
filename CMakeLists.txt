cmake_minimum_required (VERSION 3.15)

project (poly_vector CXX)

# creates a library 'poly_vector' which is an interface (header files only)
add_library(poly_vector INTERFACE)

install(
    DIRECTORY include/poly_vector
    DESTINATION include
)

# determine whether this is a standalone project or included by other projects
set(PV_STANDALONE_PROJECT OFF)
if (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    set(PV_STANDALONE_PROJECT ON)
endif ()

if (MSVC)
    # warning level 4
    target_compile_options(poly_vector INTERFACE
        /std:c++20

        # enable lots of warnings
        /W4

        # -- Dump compilation info ---
        #/Bt                     # total time spent in frontend (d1) and backend (d2)
        #/d1templateStats        # show info regarding template use
        #/d1reportTimeSummary    # show frontend compile time summary
        #/d1reportTime           # show (very) detailed frontend compile time info
        #/d2cgsummary            # show backend compile time summary

		# Shows the layout of the struct name 'chunk'
		#/d1reportSingleClassLayoutchunk

        # Enable address sanitizer
        #/fsanitize=address
    )

    # enable profiling
    #target_compile_options(poly_vector INTERFACE /Zi)
    #target_link_options(poly_vector INTERFACE /PROFILE)

    # Disable C++ exceptions.
    string(REGEX REPLACE "/EH[a-z]+" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHs-c-")
    add_definitions(-D_HAS_EXCEPTIONS=0)

    # Disable RTTI.
    string(REGEX REPLACE "/GR" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /GR-")
else()
    # lots of warnings
    target_compile_options(poly_vector INTERFACE
        -std=c++2a
        -Wall
        -Wextra
        -Wpedantic
        -Wshadow
        #-Wsign-conversion
        #-ftime-trace  # produce json flamegraph files. open with chrome://tracing or edge://tracing
	)
    target_link_options(poly_vector INTERFACE -pthread)

    # Disable C++ exceptions.
    # - fails to compile on gcc 10
    if (Clang)
        string(REGEX REPLACE "-fexceptions" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-exceptions")
    endif()

    # Disable RTTI.
    # - clang's std::function::target needs rtti -_-
    if (GNU)
        string(REGEX REPLACE "-frtti" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti")
    endif()
endif()

# Project headers
# add include folders to the library and targets that consume it
# the SYSTEM keyword suppresses warnings for users of the library
if(PV_STANDALONE_PROJECT)
    target_include_directories(poly_vector INTERFACE
        $<BUILD_INTERFACE:
            ${CMAKE_CURRENT_SOURCE_DIR}/include
        >
    )
else()
    target_include_directories(poly_vector SYSTEM INTERFACE
        $<BUILD_INTERFACE:
            ${CMAKE_CURRENT_SOURCE_DIR}/include
        >
    )
endif()

if (PV_STANDALONE_PROJECT)
    # Examples
    add_subdirectory ("examples/example")

    # Unittests
    enable_testing()
    add_subdirectory ("unittest")
endif ()
