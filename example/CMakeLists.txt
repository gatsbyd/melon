###################################################################
#TEST CMAKELISTS
###################################################################

#--[Tests lists
file(GLOB test_srcs_temp ${PROJECT_SOURCE_DIR}/example/*.cpp)

set(test_srcs "")

foreach(test_src_temp ${test_srcs_temp} )
    string(REPLACE "//" "/" test_src ${test_src_temp})
    list(APPEND test_srcs ${test_src})
endforeach(test_src_temp ${test_srcs_temp})

####################################################################

# --[ Functionality Tests
foreach(test_src ${test_srcs} )
    # get test file name
    get_filename_component(test_bare_name ${test_src} NAME)
    string(REPLACE ".cpp" "" test_bare_name_without_extension ${test_bare_name})
    string(REPLACE "\"" "" test_name ${test_bare_name_without_extension})
	message(STATUS "test_name=" "${test_name}")

	# create executable
	add_executable(${test_name} EXCLUDE_FROM_ALL ${test_src})

    # link libraries
    target_link_libraries(${test_name} melon)
    target_link_libraries(${test_name} gd)


    # set target properties
    set_target_properties(${test_name}
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/example"
        COMMAND ${test_name}
    )
endforeach(test_src ${test_srcs})

add_subdirectory(rpc)
