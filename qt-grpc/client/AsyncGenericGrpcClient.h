#pragma once

#include <iostream>
#include <memory>
#include <set>
#include <string>
#include <queue>

#include "../logging/LoggingCategories.h"
#include <chrono>
#include <QCoreApplication>
#include <QDebug>
#include <QObject>
#include <QThread>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include <grpcpp/grpcpp.h>
#pragma GCC diagnostic pop

#include "OneToMClientCall.h"

using grpc::Channel;
using grpc::ClientContext;

using grpc::CompletionQueue;

template<typename S>
class AsyncGenericGrpcClient
{
    template<typename T1, typename T2>
    friend class OneToMClientCall;

public:
    AsyncGenericGrpcClient(std::string serviceAdress, std::string clientId)
        : clientId_(clientId)
        , serviceAdress_(serviceAdress)

    {}
    ~AsyncGenericGrpcClient()
    {
        cancelCalls();
        qCDebug(qtgrpc_client) << "AsyncGenericGrpcClient dtor";
    }
    void cancelCalls()
    {
        qCDebug(qtgrpc_client) << "AsyncGenericGrpcClient cancelCalls" << clientCalls_.size();
        if (!clientCalls_.empty()) {
            {
                std::lock_guard<std::mutex> lock(mClientCalls_);
                for (auto &clientCall : clientCalls_) {
                    clientCall->context().TryCancel();
                }
            }

            //ждем пока не удалятся все ClientCalls
            using namespace std::chrono_literals;
            qCDebug(qtgrpc_client) << "AsyncGenericGrpcClient wait for clientcalls empty";
            while (!flagClientCallsEmpty_) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                QCoreApplication::processEvents();
                qDebug() << "processEvents" << clientCalls_.size();
            }
            flagClientCallsEmpty_ = false;
        }
    }

protected:
    void run()
    {
        std::shared_ptr<Channel> channel = grpc::CreateChannel(serviceAdress_,
                                                               grpc::InsecureChannelCredentials());

        stub_ = S::NewStub(channel);

        std::thread thread_ = std::thread([this]() {
            qCDebug(qtgrpc_client) << "start completion queue thread";
            void *got_tag;
            bool ok = false;

            while (cq.Next(&got_tag, &ok)) {
                qCDebug(qtgrpc_client) << "next" << got_tag << ok;

                AbstractClientCall *clientCall_ = static_cast<AbstractClientCall *>(got_tag);
                clientCall_->Proceed(ok);
            }
        });

        thread_.detach();
    }
    void addClientCall(std::shared_ptr<AbstractClientCall> clientCall)
    {
        std::lock_guard<std::mutex> lock(mClientCalls_);
        clientCalls_.insert(clientCall);
    }
    void removeClientCall(std::weak_ptr<AbstractClientCall> clientCall)
    {
        qCDebug(qtgrpc_client) << "removeClientCall";
        std::lock_guard<std::mutex> lock(mClientCalls_);
        clientCalls_.erase(clientCall.lock());
        if (clientCalls_.empty()) {
            std::unique_lock<std::mutex> lk(mWaitForClientCallsEmpty_);
            flagClientCallsEmpty_ = true;
            qCDebug(qtgrpc_client) << "flagClientCallsEmpty_ = true";
        }
    }

    std::string clientId_;
    CompletionQueue cq;
    std::unique_ptr<typename S::Stub> stub_;

private:
    std::string serviceAdress_;
    std::set<std::shared_ptr<AbstractClientCall>> clientCalls_;
    std::mutex mClientCalls_;
    bool flagClientCallsEmpty_ = false;
    std::mutex mWaitForClientCallsEmpty_;
};
