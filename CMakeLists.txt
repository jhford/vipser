cmake_minimum_required(VERSION 3.13)
project(vipser C)

set(CMAKE_C_COMPILER_VERSION C99)

set(SOURCE_FILES src/vipser.c src/types.h src/io.c src/io.h src/sniffing.h src/sniffing.c src/vips_glue.c src/vips_glue.h src/commands.c src/commands.h)
set(SNIFFER_SOURCE_FILES src/sniffer.c src/types.h src/sniffing.c src/io.c src/io.h src/sniffing.h)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GLIB REQUIRED glib-2.0)
pkg_check_modules(VIPS REQUIRED vips)

add_executable(vipser ${SOURCE_FILES})
add_executable(vipser-efence ${SOURCE_FILES})
add_executable(sniffer ${SNIFFER_SOURCE_FILES})

target_include_directories(vipser PUBLIC ${GLIB_INCLUDE_DIRS} ${VIPS_INCLUDE_DIRS} /usr/local/include)
target_link_directories(vipser PUBLIC /usr/local/lib)
target_link_libraries(vipser PUBLIC ${GLIB_LINK_LIBRARIES} ${VIPS_LINK_LIBRARIES} magic)
target_compile_options(vipser PUBLIC ${GLIB_CFLAGS_OTHER} ${GLIB_CFLAGS_OTHER})

target_include_directories(vipser-efence PUBLIC ${GLIB_INCLUDE_DIRS} ${VIPS_INCLUDE_DIRS} /usr/local/include)
target_link_directories(vipser-efence PUBLIC /usr/local/lib)
target_link_libraries(vipser-efence PUBLIC ${GLIB_LINK_LIBRARIES} ${VIPS_LINK_LIBRARIES} magic efence)
target_compile_options(vipser-efence PUBLIC ${GLIB_CFLAGS_OTHER} ${GLIB_CFLAGS_OTHER})

target_include_directories(sniffer PUBLIC /usr/local/include ${GLIB_INCLUDE_DIRS} ${VIPS_INCLUDE_DIRS})
target_link_directories(sniffer PUBLIC /usr/local/lib ${GLIB_LINK_LIBRARIES} ${VIPS_LINK_LIBRARIES})
target_link_libraries(sniffer PUBLIC magic ${GLIB_LINK_LIBRARIES} ${VIPS_LINK_LIBRARIES})
target_compile_options(sniffer PUBLIC ${GLIB_CFLAGS_OTHER} ${GLIB_CFLAGS_OTHER})
