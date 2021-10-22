#include <grpcpp/grpcpp.h>

#include <cassert>
#include <iostream>
#include <string>
#include <unordered_map>

#include "src/bank.grpc.pb.h"

class AsyncBankServiceImpl final : public Banking::Bank::AsyncService {
 public:
  AsyncBankServiceImpl(const std::string& endpoint);
  ~AsyncBankServiceImpl();
  void run();

 private:
  struct ActiveRequest {
    ActiveRequest(AsyncBankServiceImpl& server)
        : server_(server), responder_(&server_context_), state_(State::INIT) {
      server_.RequestDepositMoneyStreaming(
          &server_context_, &responder_, server_.completion_queue_.get(),
          server_.completion_queue_.get(), this);
    }
    void process();

    void renew();

   private:
    enum class State { INIT, PROCESS, FINISH };
    grpc::ServerContext server_context_;
    AsyncBankServiceImpl& server_;
    Banking::DepositMoneyResponse response_;
    Banking::DepositMoneyRequest request_;
    grpc::ServerAsyncReaderWriter<Banking::DepositMoneyResponse,
                                  Banking::DepositMoneyRequest>
        responder_;
    State state_;
  };
  std::unordered_map<std::string, int> balance_;
  grpc::ServerBuilder server_builder_;
  std::unique_ptr<grpc::ServerCompletionQueue> completion_queue_;
  std::unique_ptr<grpc::Server> server_;
  std::list<ActiveRequest> active_requests_;
  static constexpr size_t REQUESTS_PER_QUEUE = 3;
  friend class ActiveRequest;
};