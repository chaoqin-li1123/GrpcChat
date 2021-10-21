#include <grpcpp/grpcpp.h>

#include <string>
#include <unordered_map>

#include "src/bank.grpc.pb.h"

class SyncBankServiceImpl final : public Banking::Bank::Service {
 public:
  grpc::Status WithdrawMoney(grpc::ServerContext* context,
                             const Banking::WithdrawMoneyRequest* request,
                             Banking::WithdrawMoneyResponse* response) override;
  grpc::Status WithdrawMoneyStreaming(
      grpc::ServerContext* context,
      grpc::ServerReaderWriter<Banking::WithdrawMoneyResponse,
                               Banking::WithdrawMoneyRequest>* stream)
      override {
    Banking::WithdrawMoneyRequest request;
    while (stream->Read(&request)) {
      Banking::WithdrawMoneyResponse response;
      response.set_user("ming");
      response.set_dollars(100);
      stream->Write(response);
    }
    return grpc::Status::OK;
  }
  grpc::Status DepositMoney(grpc::ServerContext* context,
                            const Banking::DepositMoneyRequest* request,
                            Banking::DepositMoneyResponse* response) override;
  grpc::Status DepositMoneyStreaming(
      grpc::ServerContext* context,
      grpc::ServerReaderWriter<Banking::DepositMoneyResponse,
                               Banking::DepositMoneyRequest>* stream) override {
    return grpc::Status::OK;
  }

 private:
  std::unordered_map<std::string, int> balance;
};