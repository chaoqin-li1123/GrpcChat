#include "async_bank_server.h"

AsyncBankServiceImpl::AsyncBankServiceImpl(const std::string& endpoint) {
  server_builder_.AddListeningPort(endpoint, grpc::InsecureServerCredentials());
  server_builder_.RegisterService(this);
  completion_queue_ = server_builder_.AddCompletionQueue();
  server_ = server_builder_.BuildAndStart();
  run();
}

AsyncBankServiceImpl::~AsyncBankServiceImpl() {
  server_->Shutdown();
  completion_queue_->Shutdown();
}

void AsyncBankServiceImpl::run() {
  for (size_t i = 0; i < REQUESTS_PER_QUEUE; i++) {
    active_requests_.emplace_back(*this);
  }

  while (true) {
    void* tag;
    bool ok;
    completion_queue_->Next(&tag, &ok);
    assert(ok);
    static_cast<ActiveRequest*>(tag)->process();
  }
}

void AsyncBankServiceImpl::ActiveRequest::process() {
  if (state_ == State::INIT) {
    response_.set_user("ming");
    response_.set_dollars(request_.dollars());
    state_ = State::PROCESS;
    responder_.Write(response_, this);
  } else if (state_ == State::PROCESS) {
    responder_.Finish(grpc::Status::OK, this);
    state_ = State::FINISH;
  } else {
    assert(state_ == State::FINISH);
    renew();
  }
}

void AsyncBankServiceImpl::ActiveRequest::renew() {
  for (auto it = server_.active_requests_.begin();
       it != server_.active_requests_.end(); it++) {
    if (&(*it) == this) {
      server_.active_requests_.erase(it);
      server_.active_requests_.emplace_back(server_);
      break;
    }
  }
}