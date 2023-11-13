#pragma once

#include "CommonCallData.h"

template<typename S,
         typename Tcontext,
         typename TRequest,
         typename TReply,
         typename TNewCq,
         typename TNotifCq,
         typename TTag,
         typename TH,
         typename TAsyncService>
class UnaryRequestCallData : public AbstractCommonCallData
{
    typedef typename std::remove_pointer<TRequest>::type TRequestNoPtr;
    //  typedef std::function<TReply(TRequest)> THandler;

public:
    explicit UnaryRequestCallData(
        typename S::AsyncService *service,
        ServerCompletionQueue *cq,
        std::_Mem_fn<void (TAsyncService::*)(
            Tcontext, TRequest, ::grpc::ServerAsyncWriter<TReply> *, TNewCq, TNotifCq, TTag)> func,
        TH handler)
        : service_(service)
        , cq_(cq)
        , func_(func)
        , handler_(handler)
        , status_(CREATE)
        , prefix("Hello ")
    {
        Proceed(true);
    }
    //    void setHandler(std::function<void()> handler) {

    //    }

    virtual void Proceed(bool /*ok*/) override
    {
        if (status_ == CREATE) {
            // TRequestNoPtr tt;
            // BinaryMessage r = tt;
            // int a = tt;

            //  BinaryMessage request_;
            auto requestFn = std::bind(func_, service_, &ctx_, &request_, &responder_, cq_, cq_, this);
            requestFn();

            qCDebug(qtgrpc_server) << "UnaryRequestCallData::Proceed: New responder for ";
            //service_->RequestSendMessage(&ctx_, &request_, &responder_, cq_, cq_, this);
            status_ = PROCESS;
        } else if (status_ == PROCESS) {
            if (reply_send_) {
                qCDebug(qtgrpc_server) << "UnaryRequestCallData::Proceed: finish";
                responder_.Finish(grpc::Status::OK, this);
                status_ = FINISH;
            } else {
                auto newCallData
                    = new UnaryRequestCallData<S, Tcontext, TRequest, TReply, TNewCq, TNotifCq, TTag, TH, TAsyncService>(
                        service_, cq_, func_, handler_);
                auto requestFn = std::bind(
                    func_, service_, &ctx_, &request_, &responder_, cq_, cq_, newCallData);
                requestFn();

                qCDebug(qtgrpc_server) << "UnaryRequestCallData::Proceed: send response";

                TReply reply_ = handler_(&request_);

                responder_.Write(reply_, (void *) this);
                reply_send_ = true;
            }

        } else if (status_ == FINISH) {
            qCDebug(qtgrpc_server) << "UnaryRequestCallData::Proceed: Good Bye";
            //delete this;
        }
    }

private:
    bool reply_send_ = false;
    // The means of communication with the gRPC runtime for an asynchronous
    // server.
    typename S::AsyncService *service_;
    // The producer-consumer queue where for asynchronous server notifications.
    ServerCompletionQueue *cq_;
    // Context for the rpc, allowing to tweak aspects of it such as the use
    // of compression, authentication, as well as to send metadata back to the
    // client.
    ServerContext ctx_;
    // What we get from the client.
    TRequestNoPtr request_;
    // What we send back to the client.

    // Let's implement a tiny state machine with the following states.
    enum CallStatus { CREATE, PROCESS, FINISH };
    CallStatus status_; // The current serving state.
    std::string prefix;

    ServerAsyncWriter<TReply> responder_{&ctx_};

    std::_Mem_fn<void (
        TAsyncService::*)(Tcontext, TRequest, ::grpc::ServerAsyncWriter<TReply> *, TNewCq, TNotifCq, TTag)>
        func_;

    TH handler_;
};
