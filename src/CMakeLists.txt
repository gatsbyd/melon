###################################################################
#SRC CMAKELISTS
###################################################################

file(GLOB_RECURSE srcs ${PROJECT_SOURCE_DIR}/src/*.cpp ${PROJECT_SOURCE_DIR}/src/*.c)

add_library(melon STATIC ${srcs})
target_link_libraries(melon ${CMAKE_DL_LIBS})
