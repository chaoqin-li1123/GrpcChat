// build the binary for server and client <br />
bazel build //exe:chat_server_main <br />
bazel build //exe:chat_client_main <br />
<br />
//start server and client <br />
./bazel-bin/exe/chat_server_main localhost 10086 <br />
./bazel-bin/exe/chat_client_main localhost 10086