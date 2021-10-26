#include <grpcpp/grpcpp.h>

#include <string>
#include <unordered_map>

#include "src/bank.grpc.pb.h"

class SyncBankServiceImpl final : public Banking::Bank::Service {
 public:
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