cmake_minimum_required(VERSION 3.13)
find_package(PkgConfig REQUIRED)

project(vipser C)
project(sniffer C)

set(CMAKE_C_COMPILER_VERSION C11)

set(VIPSER_SOURCES
        src/vipser.c
        src/io.c
        src/sniffing.c
        src/vips_glue.c
        src/commands.c
        src/sandbox.c src/sandbox.h)

set(SNIFFER_SOURCES
        src/sniffer.c
        src/sniffing.c
        src/io.c
        src/sandbox.c src/sandbox.h)

pkg_check_modules(GLIB REQUIRED glib-2.0)
pkg_check_modules(VIPS REQUIRED vips)

add_executable(vipser ${VIPSER_SOURCES})
add_executable(sniffer ${SNIFFER_SOURCES})

set(INC_DIRS ${GLIB_INCLUDE_DIRS} ${VIPS_INCLUDE_DIRS} /usr/local/include)
set(LIB_DIRS ${GLIB_LIBRARY_DIRS} ${VIPS_LIBRARY_DIRS} /usr/local/lib /usr/local/lib)
set(LIBS ${GLIB_LIBRARIES} ${VIPS_LIBRARIES} magic)
set(OPTS ${GLIB_CFLAGS} ${VIPS_CFLAGS} ${GLIB_CFLAGS_OTHER} ${VIPS_CFLAGS_OTHER} -O2 -Werror -Wall -Wextra)

find_program(CPPCHECK NAMES cppcheck)
set(CMAKE_C_CPPCHECK  ${CPPCHECK})
find_program(CLANG_TIDY NAMES clang-tidy)
set(CMAKE_C_CLANG_TIDY "${CLANG_TIDY}")

target_include_directories(vipser PUBLIC ${INC_DIRS})
target_compile_options(vipser PUBLIC ${OPTS})
target_link_directories(vipser PUBLIC ${LIB_DIRS})
target_link_libraries(vipser PUBLIC ${LIBS})

target_include_directories(sniffer PUBLIC ${INC_DIRS})
target_compile_options(sniffer PUBLIC ${OPTS})
target_link_directories(sniffer PUBLIC ${LIB_DIRS})
target_link_libraries(sniffer PUBLIC ${LIBS})
