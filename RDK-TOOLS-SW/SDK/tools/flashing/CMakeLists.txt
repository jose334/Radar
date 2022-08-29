cmake_minimum_required(VERSION 3.0)
project(FlashToolRadarBaseboardMCU7)

find_package(Threads REQUIRED)

# bossa
file(GLOB bossa_src ../../3rd_party/libs/bossa/*.cpp ../../3rd_party/libs/bossa/*.h)
add_library(bossa STATIC ${bossa_src})
target_compile_definitions(bossa PUBLIC _CRT_SECURE_NO_WARNINGS)
target_include_directories(bossa PUBLIC ../../3rd_party/libs/bossa/)

# sources for the flashing library
set(flashlib_src
    "src/findComPort.cpp"
    "src/findComPort.h"
    "src/ActivateBootloader.cpp"
    "src/FlashRadar.cpp"
    "src/FlashRadar.h"
)

# shared library of flashing library
add_library(flash_rbb_mcu7 SHARED ${flashlib_src})
set_target_properties(flash_rbb_mcu7 PROPERTIES DEBUG_POSTFIX "d")
set_target_properties(flash_rbb_mcu7 PROPERTIES EXCLUDE_FROM_ALL 1)
target_include_directories(flash_rbb_mcu7 PRIVATE ../../external/strata/library/)
target_link_libraries(flash_rbb_mcu7 PRIVATE bossa)
if(WIN32 OR MINGW OR MSYS)
    target_link_libraries(flash_rbb_mcu7 PRIVATE setupapi ${CMAKE_THREAD_LIBS_INIT})
endif()

# static version of flashing library
add_library(flash_rbb_mcu7_static STATIC ${flashlib_src})
target_compile_definitions(flash_rbb_mcu7_static PUBLIC FLASHLIB_BUILD_STATIC=1)
target_include_directories(flash_rbb_mcu7_static PRIVATE ../../external/strata/library/)
target_link_libraries(flash_rbb_mcu7_static PRIVATE bossa)
if(WIN32 OR MINGW OR MSYS)
    target_link_libraries(flash_rbb_mcu7_static PRIVATE setupapi ${CMAKE_THREAD_LIBS_INIT})
endif()

# flashtool (link against static version of library to avoid dependency on shared library)
file(GLOB flashtool_rbb7_src src/flashtool.cpp src/flashtool.h)
add_executable(flashtool_rbb7 ${flashtool_rbb7_src})
target_link_libraries(flashtool_rbb7 flash_rbb_mcu7_static argparse)
