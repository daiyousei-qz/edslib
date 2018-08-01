
FILE(GLOB_RECURSE _HEADER_FILES "include/*.h")
FILE(GLOB_RECURSE _SOURCE_FILES "*.cpp")


include_directories("include")
add_library("edslib" STATIC ${_SOURCE_FILES} ${_HEADER_FILES})