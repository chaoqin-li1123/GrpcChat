#include "sync_bank_server.h"

grpc::Status SyncBankServiceImpl::WithdrawMoney(
    grpc::ServerContext* context, const Banking::WithdrawMoneyRequest* request,
    Banking::WithdrawMoneyResponse* response) {
  response->set_user("ming");
  response->set_dollars(request->dollars());
  return grpc::Status::OK;
}
grpc::Status SyncBankServiceImpl::DepositMoney(
    grpc::ServerContext* context, const Banking::DepositMoneyRequest* request,
    Banking::DepositMoneyResponse* response) {
  response->set_user("ming");
  response->set_dollars(request->dollars());
  return grpc::Status::OK;
}