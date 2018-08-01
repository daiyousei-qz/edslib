
FILE(GLOB_RECURSE _SOURCE_FILES "*.cpp")

include_directories("${CMAKE_SOURCE_DIR}/edslib/include")
include_directories("${CMAKE_SOURCE_DIR}/Reference/Catch/single_include/catch2")

add_executable("edslib-test" ${_SOURCE_FILES})

target_link_libraries("edslib-test" edslib)