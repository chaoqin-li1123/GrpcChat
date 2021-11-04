#include <csignal>
#include <memory>
#include <string>
#include <thread>

#include "async_server.h"

ChatService::AsyncService service;
AsyncServer<ChatService::AsyncService, ChatMessage, ChatMessage>* async_server;

static void signalHandler(int signal_number) {
  async_server->shutdown();
  std::cerr << "server shutdown\n";
}

void runAsyncChatServer(std::string const& endpoint) {
  async_server =
      new AsyncServer<ChatService::AsyncService, ChatMessage, ChatMessage>(
          endpoint, service);
  async_server->run();
}

int main(int argc, char** argv) {
  signal(SIGTERM, signalHandler);
  signal(SIGINT, signalHandler);
  std::string ip{argv[1]};
  std::string port{argv[2]};
  std::string endpoint = ip + ":" + port;

  runAsyncChatServer(endpoint);
}