#include <grpcpp/grpcpp.h>

#include <iostream>
#include <memory>
#include <thread>

#include "async_client.h"
#include "src/bank.grpc.pb.h"

int main(int argc, char** argv) {
  std::string ip{argv[1]};
  std::string port{argv[2]};
  std::string endpoint = ip + ":" + port;
  std::shared_ptr<grpc::Channel> channel =
      grpc::CreateChannel(endpoint, grpc::InsecureChannelCredentials());
  AsyncClientStream<Banking::Bank::Stub, Banking::DepositMoneyRequest,
                    Banking::DepositMoneyResponse>
      async_client_stream(endpoint, Banking::Bank::NewStub(channel));
  Banking::DepositMoneyRequest request;
  request.set_user("ming");
  async_client_stream.sendRequest(request);
}