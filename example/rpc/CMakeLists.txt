set(rpc_server "rpc_srver")
add_executable(rpc_server EXCLUDE_FROM_ALL "rpc_server.cpp" "echo.pb.cc" "args.pb.cc")
target_link_libraries(rpc_server melon)

set_target_properties(rpc_server
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/example"
        COMMAND rpc_server)


set(rpc_client "rpc_client")
add_executable(rpc_client EXCLUDE_FROM_ALL "rpc_client.cpp" "echo.pb.cc" "args.pb.cc")
target_link_libraries(rpc_client melon)

set_target_properties(rpc_client
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/example"
        COMMAND rpc_client)
