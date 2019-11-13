###################################################################
#SRC CMAKELISTS
###################################################################

file(GLOB_RECURSE srcs ${PROJECT_SOURCE_DIR}/src/*.cpp ${PROJECT_SOURCE_DIR}/src/*.c)
file(GLOB_RECURSE headers ${PROJECT_SOURCE_DIR}/src/*.h)

add_library(melon STATIC ${srcs})
target_link_libraries(melon ${CMAKE_DL_LIBS}) 	#dlsym
target_link_libraries(melon ${PROTOBUF_LIBRARIES}) #protobuf
target_link_libraries(melon ${ZLIB_LIBRARIES})	#zlib


install(TARGETS melon
        ARCHIVE DESTINATION /usr/lib
        LIBRARY DESTINATION /usr/lib)
install(FILES ${headers}
        DESTINATION /usr/local/include/melon)
