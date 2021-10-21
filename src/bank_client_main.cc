#include <grpcpp/grpcpp.h>

#include <iostream>
#include <memory>

#include "src/bank.grpc.pb.h"

int main(int argc, char** argv) {
  std::string ip{argv[1]};
  std::string port{argv[2]};
  std::string endpoint = ip + ":" + port;

  grpc::ClientContext context;
  std::shared_ptr<grpc::Channel> channel =
      grpc::CreateChannel(endpoint, grpc::InsecureChannelCredentials());
  std::unique_ptr<Banking::Bank::Stub> stub{Banking::Bank::NewStub(channel)};

  Banking::DepositMoneyRequest request;
  request.set_user("qq");
  request.set_dollars(100);
  Banking::DepositMoneyResponse response;
  std::cout << "making grpc call." << std::endl;
  /*
  std::shared_ptr<grpc::ClientReaderWriter<Banking::DepositMoneyRequest,
                                           Banking::DepositMoneyResponse>>
      stream = stub->DepositMoneyStreaming(&context);
      */
  grpc::Status status = stub->DepositMoney(&context, request, &response);
  if (!status.ok()) {
    std::cout << status.error_code() << ": " << status.error_message()
              << std::endl;
  } else {
    std::cout << response.user() << std::endl;
  }

  std::cout << "finish grpc call." << std::endl;
}