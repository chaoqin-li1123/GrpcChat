#include <memory>
#include <string>

#include "async_bank_server.h"
#include "sync_bank_server.h"

void runSyncBankServer(std::string const& endpoint) {
  SyncBankServiceImpl service;
  grpc::ServerBuilder server_builder;
  server_builder.AddListeningPort(endpoint, grpc::InsecureServerCredentials());
  server_builder.RegisterService(&service);
  std::unique_ptr<grpc::Server> server{server_builder.BuildAndStart()};
  server->Wait();
}

void runAsyncBankServer(std::string const& endpoint) {
  AsyncBankServiceImpl service(endpoint);
}

int main(int argc, char** argv) {
  std::string ip{argv[1]};
  std::string port{argv[2]};
  std::string endpoint = ip + ":" + port;
  runAsyncBankServer(endpoint);
}