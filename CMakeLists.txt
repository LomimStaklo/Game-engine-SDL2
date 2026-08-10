cmake_minimum_required(VERSION 3.10)
project(StreetKebabFighter C)

# Set C standard
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Compiler flags
if(NOT MSVC)
    add_compile_options(-Wall -Wextra -Wshadow -Wswitch-enum -Wconversion -Wsign-conversion
                      -Wdouble-promotion -Wundef -Wuninitialized -Wnull-dereference
                      -Wformat=2 -Wfloat-equal -Wcast-qual -Wcast-align
                      -Wmissing-prototypes -Wstrict-prototypes -Wstrict-aliasing=2
                      -Warray-bounds)
endif()

# Set optimization level
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(-g -O0)
else()
    add_compile_options(-O3)
endif()

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/src/include)

# Source files
set(SOURCES
    src/main.c
    src/game.c
)

# Create executable
add_executable(SKF ${SOURCES})

# Find SDL2 packages
if(WIN32)
    # On Windows, use bundled static libraries
    set(SDL2_LIBRARIES 
        ${CMAKE_SOURCE_DIR}/src/libs/libSDL2.a
        ${CMAKE_SOURCE_DIR}/src/libs/libSDL2main.a
        ${CMAKE_SOURCE_DIR}/src/libs/libSDL2_image.a
    )
else()
    # On Linux/Unix, use system SDL2
    find_package(PkgConfig QUIET)
    if(PkgConfig_FOUND)
        pkg_check_modules(SDL2 QUIET sdl2)
        pkg_check_modules(SDL2_IMAGE QUIET SDL2_image)
    endif()
    
    if(NOT SDL2_FOUND OR NOT SDL2_IMAGE_FOUND)
        # Try CMake's find_package for SDL2
        find_package(SDL2 QUIET)
        find_package(SDL2_image QUIET)
    endif()
    
    if(NOT SDL2_FOUND OR NOT SDL2_IMAGE_FOUND)
        message(FATAL_ERROR "SDL2 development libraries not found. "
                "Please install them using your package manager:\n"
                "  Ubuntu/Debian: sudo apt install libsdl2-dev libsdl2-image-dev\n"
                "  Fedora:        sudo dnf install SDL2-devel SDL2_image-devel\n"
                "  Arch:          sudo pacman -S sdl2 sdl2_image\n"
                "  openSUSE:      sudo zypper install libSDL2-devel libSDL2_image-devel")
    endif()
endif()

# Link libraries
if(WIN32)
    target_link_libraries(SKF ${SDL2_LIBRARIES} m)
else()
    target_link_libraries(SKF ${SDL2_LIBRARIES} ${SDL2_IMAGE_LIBRARIES} m)
    target_include_directories(SKF PRIVATE ${SDL2_INCLUDE_DIRS} ${SDL2_IMAGE_INCLUDE_DIRS})
endif()

# Set output directory
set_target_properties(SKF PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}
)

# Copy DLLs to output directory on Windows
if(WIN32)
    add_custom_command(TARGET SKF POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_SOURCE_DIR}/SDL2.dll
        ${CMAKE_SOURCE_DIR}
    )
    add_custom_command(TARGET SKF POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_SOURCE_DIR}/SDL2_image.dll
        ${CMAKE_SOURCE_DIR}
    )
endif()

# Optional: Add a target for running the game
add_custom_target(run
    COMMAND SKF
    DEPENDS SKF
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Running Street Kebab Fighter"
)