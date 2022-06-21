project(WebXR)

# Generate a dummy .cpp file for the library whenever library_webxr.js changes
add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/webxr.cpp
    COMMAND ${CMAKE_COMMAND} -E touch ${CMAKE_CURRENT_BINARY_DIR}/webxr.cpp
    DEPENDS library_webxr.js)

# Add an empty library with the generted dummy file. This will cause anything
# that links to it to relink when library_webxr.js changes.
# Also adding library_webxr.js and webxr.h here so that they show up in IDEs
add_library(webxr STATIC webxr.h library_webxr.js
    ${CMAKE_CURRENT_BINARY_DIR}/webxr.cpp)
target_include_directories(webxr PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# "link" the webxr javascript library when linking to webxr
# Next release of CMake will contain "INTERFACE_LINK_OPTIONS"!
if(CMAKE_VERSION VERSION_GREATER_EQUAL 3.13.0)
    target_link_options(webxr PUBLIC
        "SHELL:--js-library ${CMAKE_CURRENT_SOURCE_DIR}/library_webxr.js")

elseif(CMAKE_VERSION VERSION_GREATER_EQUAL 3.12.0)
    set_property(TARGET webxr APPEND_STRING PROPERTY
        INTERFACE_LINK_OPTIONS
            " --js-library ${CMAKE_CURRENT_SOURCE_DIR}/library_webxr.js")
else()
    # *UGLY HACK* to get --js-library into the link parameters. May only work if webxr
    # target is linked last, so be aware of that or use cmake >= 3.12.0
    # Also, note that there is no whitespace prefix as in the above.
    set_property(TARGET webxr APPEND_STRING PROPERTY
        INTERFACE_LINK_LIBRARIES
            " --js-library ${CMAKE_CURRENT_SOURCE_DIR}/library_webxr.js")
endif()
