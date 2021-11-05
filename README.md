// build the binary for server and client
bazel build //exe:chat_server_main
bazel build //exe:chat_client_main

//start server and client
./bazel-bin/exe/chat_server_main localhost 10086
./bazel-bin/exe/chat_client_main localhost 10086