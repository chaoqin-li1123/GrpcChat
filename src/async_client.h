#ifndef ASYNC_CLIENT_H
#define ASYNC_CLIENT_H
#include <grpcpp/grpcpp.h>

#include <iostream>
#include <memory>
#include <string>

#include "async_stream.h"
#include "src/bank.grpc.pb.h"

template <typename ServiceStub, typename Request, typename Response>
struct AsyncClientStream : public AsyncStream {
  AsyncClientStream(std::string endpoint, std::unique_ptr<ServiceStub>&& stub)
      : stub_(std::move(stub)),
        rw_(stub_->PrepareAsyncDepositMoneyStreaming(&context_, &cq_)) {
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
      bool ok = false;
      bool has_new_event =
          cq_.AsyncNext(&tag, &ok, gpr_time_0(GPR_CLOCK_REALTIME));
      if (ok && has_new_event) {
        (static_cast<ActionTag<AsyncClientStream>*>(tag))->act();
      }
    }
  }
  void onReadComplete() override {
    std::cout << "read complete" << std::endl;
    std::cout << response_.user() << std::endl;
  }

  void onInitComplete() override {
    std::cout << "init complete" << std::endl;
    request_.set_user("client");
    rw_->Read(&response_, &read_tag_);
    rw_->Write(request_, &write_tag_);
  }

  void onWriteComplete() override {
    std::cout << "write complete" << std::endl;
  }

  void onFinishComplete() override {}

  void sendRequest(Request& request) { rw_->Write(request, &write_tag_); }

 private:
  grpc::CompletionQueue cq_;
  grpc::ClientContext context_;
  std::shared_ptr<grpc::Channel> channel_;
  std::unique_ptr<ServiceStub> stub_;
  std::shared_ptr<grpc::ClientAsyncReaderWriter<Request, Response>> rw_;
  Response response_;
  Request request_;
  ActionTag<AsyncClientStream> init_tag_{this, Action::INIT};
  ActionTag<AsyncClientStream> read_tag_{this, Action::READ};
  ActionTag<AsyncClientStream> write_tag_{this, Action::WRITE};
  ActionTag<AsyncClientStream> finish_tag_{this, Action::FINISH};
};

#endif