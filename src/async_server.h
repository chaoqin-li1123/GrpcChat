#ifndef ASYNC_SERVER_H
#define ASYNC_SERVER_H
#include <grpcpp/grpcpp.h>

#include <cassert>
#include <functional>
#include <iostream>
#include <string>
#include <unordered_map>

#include "async_stream.h"
#include "src/bank.grpc.pb.h"

template <typename Service, typename Request, typename Response>
class AsyncServer {
 public:
  struct AsyncServerStream : public AsyncStream {
    AsyncServerStream(
        AsyncServer& server,
        std::function<void(grpc::ServerContext*,
                           grpc::ServerAsyncReaderWriter<Response, Request>*,
                           grpc::ServerCompletionQueue*, void*)>
            init_callback)
        : server_(server), rw_(&server_context_) {
      init_callback(&server_context_, &rw_, server_.completion_queue_.get(),
                    &init_tag_);
    }

    ~AsyncServerStream() {}
    void onReadComplete() override {
      std::cout << "read complete" << std::endl;
      std::cout << request_.user() << std::endl;
    }

    void onInitComplete() override {
      started_ = true;
      std::cout << "init complete" << std::endl;
      response_.set_user("server");
      response_.set_dollars(request_.dollars());
      rw_.Read(&request_, &read_tag_);
      rw_.Write(response_, &write_tag_);
    }

    void onWriteComplete() override {
      std::cout << "write complete" << std::endl;
    }

    void onFinishComplete() override { deleteSelf(); }

    void localClose() {
      shutdown_ = true;
      if (!shutdown_) {
        rw_.Finish(grpc::Status::OK, &finish_tag_);
      }
    }

   private:
    void deleteSelf() {
      for (auto it = server_.active_requests_.begin();
           it != server_.active_requests_.end(); it++) {
        if (&(*it) == this) {
          server_.active_requests_.erase(it);
          break;
        }
      }
    }
    Response response_;
    Request request_;
    grpc::ServerContext server_context_;
    AsyncServer& server_;
    grpc::ServerAsyncReaderWriter<Response, Request> rw_;
    bool started_{false};
    bool shutdown_{false};
    ActionTag<AsyncServerStream> init_tag_{this, Action::INIT};
    ActionTag<AsyncServerStream> read_tag_{this, Action::READ};
    ActionTag<AsyncServerStream> write_tag_{this, Action::WRITE};
    ActionTag<AsyncServerStream> finish_tag_{this, Action::FINISH};
  };
  AsyncServer(const std::string& endpoint, Service& service)
      : service_(service) {
    server_builder_.AddListeningPort(endpoint,
                                     grpc::InsecureServerCredentials());
    server_builder_.RegisterService(&service_);
    completion_queue_ = server_builder_.AddCompletionQueue();
    server_ = server_builder_.BuildAndStart();
    init_callback_ = [&](grpc::ServerContext* context,
                         grpc::ServerAsyncReaderWriter<Response, Request>* rw,
                         grpc::ServerCompletionQueue* cq, void* tag) {
      service_.RequestDepositMoneyStreaming(context, rw, cq, cq, tag);
    };
  }

  ~AsyncServer() {
    server_->Shutdown();
    completion_queue_->Shutdown();
    void* tag;
    bool ok = false;
    std::cerr << "shutdown successfully" << std::endl;
    while (completion_queue_->Next(&tag, &ok)) {
    }
  }

  void run() {
    for (size_t i = 0; i < REQUESTS_PER_QUEUE; i++) {
      active_requests_.emplace_back(*this, init_callback_);
    }

    while (!active_requests_.empty()) {
      if (shutdown_) {
        for (auto& stream : active_requests_) {
          stream.localClose();
        }
        break;
      }
      void* tag;
      bool ok = false;
      bool has_new_event = completion_queue_->AsyncNext(
          &tag, &ok, gpr_time_0(GPR_CLOCK_REALTIME));
      if (ok && has_new_event) {
        (static_cast<ActionTag<AsyncServerStream>*>(tag))->act();
      }
    }
    std::cerr << "exit main loop" << std::endl;

    delete this;
  }

  void shutdown() { shutdown_ = true; }

 private:
  Service& service_;
  grpc::ServerBuilder server_builder_;
  std::unique_ptr<grpc::ServerCompletionQueue> completion_queue_;
  std::unique_ptr<grpc::Server> server_;
  std::list<AsyncServerStream> active_requests_;
  bool shutdown_{false};
  std::function<void(grpc::ServerContext*,
                     grpc::ServerAsyncReaderWriter<Response, Request>*,
                     grpc::ServerCompletionQueue*, void*)>
      init_callback_;
  static constexpr size_t REQUESTS_PER_QUEUE = 3;
  friend class AsyncServerStream;
};

#endif