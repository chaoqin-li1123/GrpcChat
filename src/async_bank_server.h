#include <grpcpp/grpcpp.h>

#include <cassert>
#include <string>
#include <unordered_map>

#include "src/bank.grpc.pb.h"

class AsyncBankServiceImpl final : public Banking::Bank::AsyncService {
 public:
  AsyncBankServiceImpl(const std::string& endpoint) {
    server_builder_.AddListeningPort(endpoint,
                                     grpc::InsecureServerCredentials());
    server_builder_.RegisterService(this);
    completion_queue_ = server_builder_.AddCompletionQueue();
    server_ = server_builder_.BuildAndStart();
    run();
  }
  ~AsyncBankServiceImpl() {
    server_->Shutdown();
    completion_queue_->Shutdown();
  }
  enum class RequestType { DepositMoneyRequest, WithdrawMoneyRequest };
  void run() {
    for (int i = 0; i < 10; i++) {
      active_requests_.emplace_back(*this, RequestType::DepositMoneyRequest);
    }

    while (true) {
      void* tag;
      bool ok;
      completion_queue_->Next(&tag, &ok);
      assert(ok);
      static_cast<ActiveRequest*>(tag)->process();
    }
  }

  grpc::Status WithdrawMoneyStreaming(
      grpc::ServerContext* context,
      grpc::ServerReaderWriter<Banking::WithdrawMoneyResponse,
                               Banking::WithdrawMoneyRequest>* stream)
      override {
    return grpc::Status::OK;
  }

  grpc::Status DepositMoneyStreaming(
      grpc::ServerContext* context,
      grpc::ServerReaderWriter<Banking::DepositMoneyResponse,
                               Banking::DepositMoneyRequest>* stream) override {
    return grpc::Status::OK;
  }

 private:
  struct ActiveRequest {
    ActiveRequest(AsyncBankServiceImpl& server, RequestType request_type)
        : server_(server),
          responder_(&server_.server_context_),
          request_type_(request_type),
          state_(State::CREATE) {
      server_.RequestDepositMoney(&server_.server_context_, &request_,
                                  &responder_, server_.completion_queue_.get(),
                                  server_.completion_queue_.get(), this);
      state_ = State::PROCESS;
    }
    void process() {
      if (state_ == State::PROCESS) {
        switch (request_type_) {
          case RequestType::DepositMoneyRequest:
            response_.set_user("ming");
            response_.set_dollars(request_.dollars());
            break;
          case RequestType::WithdrawMoneyRequest:
            response_.set_user("ming");
            response_.set_dollars(request_.dollars());
            break;
        }
        state_ = State::FINISH;
        responder_.Finish(response_, grpc::Status::OK, this);
      } else {
        assert(state_ == State::FINISH);
      }
    }

   private:
    enum class State { CREATE, PROCESS, FINISH };
    AsyncBankServiceImpl& server_;
    Banking::DepositMoneyResponse response_;
    Banking::DepositMoneyRequest request_;
    grpc::ServerAsyncResponseWriter<Banking::DepositMoneyResponse> responder_;
    RequestType request_type_;
    State state_;
  };
  std::unordered_map<std::string, int> balance_;
  grpc::ServerBuilder server_builder_;
  grpc::ServerContext server_context_;
  std::unique_ptr<grpc::ServerCompletionQueue> completion_queue_;
  std::unique_ptr<grpc::Server> server_;
  std::list<ActiveRequest> active_requests_;
  friend class ActiveRequest;
};