#include <grpcpp/grpcpp.h>

#include <iostream>
#include <memory>
#include <thread>

#include "api/chat.grpc.pb.h"
#include "src/async_client.h"

int main(int argc, char** argv) {
  std::string ip{argv[1]};
  std::string port{argv[2]};
  std::string endpoint = ip + ":" + port;
  std::shared_ptr<grpc::Channel> channel =
      grpc::CreateChannel(endpoint, grpc::InsecureChannelCredentials());
  AsyncClientStream<ChatService::Stub, ChatMessage, ChatMessage>
      async_client_stream(endpoint, ChatService::NewStub(channel));
  ChatMessage request;
  request.set_user("ming");
  async_client_stream.send(request);
}