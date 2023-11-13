#pragma once

#include <QCoreApplication>
#include <memory>
#include <set>
#include <thread>
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include <grpc++/grpc++.h>
#pragma GCC diagnostic pop

#include "CommonCallData.h"
#include "UnaryRequestCallData.h"

using grpc::Server;
using grpc::ServerAsyncReader;
using grpc::ServerAsyncResponseWriter;
using grpc::ServerAsyncWriter;
using grpc::ServerBuilder;
using grpc::ServerCompletionQueue;
using grpc::ServerContext;

template<typename S>
class AsyncGenericGrpcServer
{
public:
    AsyncGenericGrpcServer(std::string server_address)
        : server_address_(server_address)
    {}

    ~AsyncGenericGrpcServer() {



        qCDebug(qtgrpc_server) << "AsyncGenericGrpcServer dtor";
        stopServer();
    }


    void stopServer() {

        qCDebug(qtgrpc_server) << "server Shutdown";
        using namespace std::chrono_literals;
        auto deadline = std::chrono::system_clock::now() + 1000ms;
        server_->Shutdown(deadline);
        qCDebug(qtgrpc_server) << "server_ Shutdown complete";
       cq_->Shutdown();
       qCDebug(qtgrpc_server) << "cq_ Shutdown complete";


       while (!cqFinished_) {
       using namespace std::chrono_literals;
       std::this_thread::sleep_for(2ms);
       QCoreApplication::processEvents();
       }


       qCDebug(qtgrpc_server) << "server Shutdown complete";


    }


    void run()
    {
        ServerBuilder builder;
        // Listen on the given address without any authentication mechanism.
        builder.AddListeningPort(server_address_, grpc::InsecureServerCredentials());
        // Register "service_" as the instance through which we'll communicate with
        // clients. In this case it corresponds to an *asynchronous* service.

        builder.RegisterService(&service_);
        // Get hold of the completion queue used for the asynchronous communication
        // with the gRPC runtime.
        cq_ = builder.AddCompletionQueue();
        // Finally assemble the server.
        server_ = builder.BuildAndStart();
        std::cout << "Server listening on " << server_address_ << std::endl;

        std::thread thread_ = std::thread([this]() {
            qCDebug(qtgrpc_server) << "start completion queue thread";
            void *tag = nullptr; // uniquely identifies a request.
            bool ok;
            while (true) {
               bool res = cq_->Next(&tag, &ok);
                qCDebug(qtgrpc_server) << "Next" << tag << ok << res;
                if (res == false) {
                    qCDebug(qtgrpc_server) << "leaving cq thread";
                    break;
                }



                //не обрабатываем тег который приходит после срабатывания AsyncNotifyWhenDone(0)
                //после прихода этого тега в потоке сработает ctx_.IsCancelled()
                if (tag == 0)
                    continue;

                //  GPR_ASSERT(cq_->Next(&tag, &ok));
                AbstractCommonCallData *callData_ = static_cast<AbstractCommonCallData *>(tag);
                callData_->Proceed(ok);
            }
            cqFinished_ = true;
            qCDebug(qtgrpc_server) << "finish completion queue thread";
        });

        thread_.detach();
    }

    //   ::grpc::ServerContext* context, ::helloworld::BinaryMessage* request, ::grpc::ServerAsyncWriter< ::helloworld::MessageState>* writer,
    //  ::grpc::CompletionQueue* new_call_cq, ::grpc::ServerCompletionQueue* notification_cq, void *tag

    void addCalldata(std::shared_ptr<AbstractCommonCallData> callData)
    {
        callDatas_.insert(callData);
    }
    void removeCalldata(std::shared_ptr<AbstractCommonCallData> callData)
    {
        callDatas_.erase(callData);
    }

protected:
    std::unique_ptr<ServerCompletionQueue> cq_;
    bool cqFinished_ = false;
    typename S::AsyncService service_;
    std::unique_ptr<Server> server_;
    std::string server_address_;

private:
    std::set<std::shared_ptr<AbstractCommonCallData>> callDatas_;
};
