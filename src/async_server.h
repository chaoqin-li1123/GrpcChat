#ifndef ASYNC_SERVER_H
#define ASYNC_SERVER_H
#include <grpcpp/grpcpp.h>

#include <cassert>
#include <functional>
#include <iostream>
#include <list>
#include <memory>
#include <string>
#include <unordered_map>

#include "api/chat.grpc.pb.h"
#include "async_stream.h"

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

    void onReadComplete(bool ok) override {
      std::cout << "read complete" << std::endl;
      if (!ok) localClose();
      if (!shutdown_) {
        received_requests_.push_back(request_);
        // Issue a new read after read complete.
        rw_.Read(&request_, &read_tag_);
        inflight_tags_++;
      }
      onCompleteOps();
    }

    void onInitComplete(bool ok) override {
      started_ = true;
      if (!ok) localClose();
      std::cout << "init complete" << std::endl;
      // Start reading.
      if (!shutdown_) {
        rw_.Read(&request_, &read_tag_);
        inflight_tags_++;
      }
      onCompleteOps();
    }

    void onWriteComplete(bool ok) override {
      pending_responses_.pop_front();
      if (!ok) localClose();
      // send pending response.
      if (!shutdown_) {
        if (!pending_responses_.empty()) {
          rw_.Write(pending_responses_.front(), &write_tag_);
          inflight_tags_++;
        }
      }
      onCompleteOps();
      std::cout << "write complete" << std::endl;
    }

    void onFinishComplete(bool ok) override { onCompleteOps(); }

    void localClose() {
      std::cerr << "local close\n";
      shutdown_ = true;
      if (!shutdown_) {
        rw_.Finish(grpc::Status::OK, &finish_tag_);
        inflight_tags_++;
      }
    }

    void send(Response& response) {
      pending_responses_.push_back(response);
      // Send immediately if don't have inflight response.
      if (pending_responses_.size() == 1) {
        rw_.Write(pending_responses_.front(), &write_tag_);
        inflight_tags_++;
      }
    }

    std::unique_ptr<Request> receive() {
      if (received_requests_.empty()) {
        return nullptr;
      }
      Request request = received_requests_.front();
      received_requests_.pop_front();
      return std::make_unique<Request>(move(request));
    }

   private:
    void deleteSelf() {
      for (auto it = server_.active_streams_.begin();
           it != server_.active_streams_.end(); it++) {
        if (&(*it) == this) {
          server_.active_streams_.erase(it);
          break;
        }
      }
    }

    void onCompleteOps() {
      inflight_tags_--;
      if (inflight_tags_ == 0 && shutdown_) deleteSelf();
    }

    std::list<Request> received_requests_;
    std::list<Response> pending_responses_;
    Request request_;
    grpc::ServerContext server_context_;
    AsyncServer& server_;
    grpc::ServerAsyncReaderWriter<Response, Request> rw_;
    bool started_{false};
    bool write_pending_{false};
    bool shutdown_{false};
    ActionTag<AsyncServerStream> init_tag_{this, Action::INIT};
    ActionTag<AsyncServerStream> read_tag_{this, Action::READ};
    ActionTag<AsyncServerStream> write_tag_{this, Action::WRITE};
    ActionTag<AsyncServerStream> finish_tag_{this, Action::FINISH};
    int inflight_tags_{0};
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
      service_.RequestChatStreaming(context, rw, cq, cq, tag);
    };
  }

  ~AsyncServer() {
    server_->Shutdown(gpr_time_0(GPR_CLOCK_REALTIME));
    completion_queue_->Shutdown();
    void* tag;
    bool ok = false;
    std::cerr << "shutdown successfully" << std::endl;
    while (completion_queue_->Next(&tag, &ok)) {
    }
  }

  void run() {
    for (size_t i = 0; i < REQUESTS_PER_QUEUE; i++) {
      active_streams_.emplace_back(*this, init_callback_);
    }

    while (!shutdown_) {
      void* tag;
      bool ok = false;
      grpc::CompletionQueue::NextStatus status = completion_queue_->AsyncNext(
          &tag, &ok, gpr_time_0(GPR_CLOCK_REALTIME));
      if (status == grpc::CompletionQueue::NextStatus::GOT_EVENT) {
        (static_cast<ActionTag<AsyncServerStream>*>(tag))->act(ok);
      }
    }
    for (auto& stream : active_streams_) {
      stream.localClose();
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
  std::list<AsyncServerStream> active_streams_;
  bool shutdown_{false};
  std::function<void(grpc::ServerContext*,
                     grpc::ServerAsyncReaderWriter<Response, Request>*,
                     grpc::ServerCompletionQueue*, void*)>
      init_callback_;
  static constexpr size_t REQUESTS_PER_QUEUE = 3;
  friend class AsyncServerStream;
};

#endif