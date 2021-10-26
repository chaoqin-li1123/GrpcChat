#include <csignal>
#include <memory>
#include <string>
#include <thread>

#include "async_server.h"

Banking::Bank::AsyncService service;
AsyncServer<Banking::Bank::AsyncService, Banking::DepositMoneyRequest,
            Banking::DepositMoneyResponse>* async_server;

static void signalHandler(int signal_number) {
  async_server->shutdown();
  std::cerr << "server shutdown\n";
}

void runAsyncBankServer(std::string const& endpoint) {
  async_server =
      new AsyncServer<Banking::Bank::AsyncService, Banking::DepositMoneyRequest,
                      Banking::DepositMoneyResponse>(endpoint, service);
  async_server->run();
}

int main(int argc, char** argv) {
  signal(SIGTERM, signalHandler);
  signal(SIGINT, signalHandler);
  std::string ip{argv[1]};
  std::string port{argv[2]};
  std::string endpoint = ip + ":" + port;

  runAsyncBankServer(endpoint);
}