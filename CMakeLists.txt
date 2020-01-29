cmake_minimum_required(VERSION 3.13)
project(vipser)

set(CMAKE_C_COMPILER_VERSION C99)

set(SOURCE_FILES vipser.c)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GLIB REQUIRED glib-2.0)
pkg_check_modules(VIPS REQUIRED vips)

add_executable(vipser ${SOURCE_FILES})

target_link_libraries(vipser PUBLIC ${GLIB_LINK_LIBRARIES} ${VIPS_LINK_LIBRARIES})
target_include_directories(vipser PUBLIC ${GLIB_INCLUDE_DIRS} ${VIPS_INCLUDE_DIRS})
target_compile_options(vipser PUBLIC ${GLIB_CFLAGS_OTHER} ${GLIB_CFLAGS_OTHER})