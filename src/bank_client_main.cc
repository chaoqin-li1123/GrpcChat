#include <grpcpp/grpcpp.h>

#include <iostream>
#include <memory>
#include <thread>

#include "src/bank.grpc.pb.h"

int main(int argc, char** argv) {
  std::string ip{argv[1]};
  std::string port{argv[2]};
  std::string endpoint = ip + ":" + port;

  grpc::ClientContext context;
  std::shared_ptr<grpc::Channel> channel =
      grpc::CreateChannel(endpoint, grpc::InsecureChannelCredentials());
  std::unique_ptr<Banking::Bank::Stub> stub{Banking::Bank::NewStub(channel)};
  /*
    // send request synchronously.
    grpc::Status status = stub->DepositMoney(&context, request, &response);
    if (!status.ok()) {
      std::cout << status.error_code() << ": " << status.error_message()
                << std::endl;
    } else {
      std::cout << response.user() << std::endl;
    }
  */
  Banking::DepositMoneyResponse response;

  std::shared_ptr<grpc::ClientReaderWriter<Banking::DepositMoneyRequest,
                                           Banking::DepositMoneyResponse>>
      stream = stub->DepositMoneyStreaming(&context);
  std::thread writer([stream]() {
    Banking::DepositMoneyRequest request;
    request.set_user("qq");
    request.set_dollars(100);
    stream->Write(request);
    stream->WritesDone();
    std::cout << "already make grpc call." << std::endl;
  });
  while (stream->Read(&response)) {
    std::cout << response.user() << std::endl;
  }
  writer.join();
  grpc::Status status = stream->Finish();
  if (!status.ok()) {
    std::cout << status.error_code() << ": " << status.error_message()
              << std::endl;
  }
  std::cout << "finish grpc call." << std::endl;
}