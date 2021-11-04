#ifndef ASYNC_CLIENT_H
#define ASYNC_CLIENT_H
#include <grpcpp/grpcpp.h>

#include <iostream>
#include <list>
#include <memory>
#include <string>

#include "async_stream.h"
#include "src/chat.grpc.pb.h"

template <typename ServiceStub, typename Request, typename Response>
struct AsyncClientStream : public AsyncStream {
  AsyncClientStream(std::string endpoint, std::unique_ptr<ServiceStub>&& stub)
      : stub_(std::move(stub)),
        rw_(stub_->PrepareAsyncChatStreaming(&context_, &cq_)) {
    run();
  }
  ~AsyncClientStream() {
    cq_.Shutdown();
    void* tag;
    bool ok = false;
    while (cq_.Next(&tag, &ok)) {
    }
  }
  void run() {
    rw_->StartCall(&init_tag_);
    while (true) {
      void* tag;
      bool ok;
      grpc::CompletionQueue::NextStatus status =
          cq_.AsyncNext(&tag, &ok, gpr_time_0(GPR_CLOCK_REALTIME));
      if (status == grpc::CompletionQueue::NextStatus::GOT_EVENT && ok) {
        (static_cast<ActionTag<AsyncClientStream>*>(tag))->act(ok);
      }
    }
  }

  void onReadComplete(bool ok) override {
    std::cout << "read complete " << response_.user() << std::endl;
    received_responses_.push_back(response_);
    rw_->Read(&response_, &read_tag_);
  }

  void onInitComplete(bool ok) override {
    std::cout << "init complete" << std::endl;
    request_.set_user("client");
    send(request_);
  }

  void onWriteComplete(bool ok) override {
    std::cout << "write complete" << std::endl;
    write_pending_ = false;
    writePendingRequest();
  }

  void onFinishComplete(bool ok) override {}

  void send(Request const& request) {
    pending_requests_.push_back(request);
    writePendingRequest();
  }

  std::unique_ptr<Response> receive() {
    if (received_responses_.empty()) {
      return nullptr;
    }
    Request response = received_responses_.front();
    received_responses_.pop_front();
    rw_->Read(&response_, &read_tag_);
    return std::make_unique<Response>(move(response));
  }

 private:
  void writePendingRequest() {
    if (write_pending_ || pending_requests_.empty()) return;
    request_ = pending_requests_.front();
    pending_requests_.pop_front();
    rw_->Write(request_, &write_tag_);
    write_pending_ = true;
  }

  grpc::CompletionQueue cq_;
  grpc::ClientContext context_;
  std::shared_ptr<grpc::Channel> channel_;
  std::unique_ptr<ServiceStub> stub_;
  std::shared_ptr<grpc::ClientAsyncReaderWriter<Request, Response>> rw_;
  bool write_pending_{false};
  std::list<Response> received_responses_;
  std::list<Request> pending_requests_;
  Response response_;
  Request request_;
  ActionTag<AsyncClientStream> init_tag_{this, Action::INIT};
  ActionTag<AsyncClientStream> read_tag_{this, Action::READ};
  ActionTag<AsyncClientStream> write_tag_{this, Action::WRITE};
  ActionTag<AsyncClientStream> finish_tag_{this, Action::FINISH};
};

#endif