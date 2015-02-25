cmake_minimum_required (VERSION 2.8.11)
project (png_to_jpeg)

include_directories(..)

find_package(PNG REQUIRED)
include_directories(${PNG_INCLUDE_DIR})

find_package(JPEG REQUIRED)
include_directories(${JPEG_INCLUDE_DIR})

add_executable(png_to_jpeg main.c ../simple_png_to_jpeg.c ../simple_png_to_jpeg.h)
target_link_libraries(png_to_jpeg ${PNG_LIBRARY} ${JPEG_LIBRARY})


