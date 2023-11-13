#pragma once

#include "CommonCallData.h"
#include <memory>
#include <mutex>
#include <sstream>
#include <thread>
#include <chrono>

template<typename S, typename TRequest, typename TReply, typename RequestFuncType, typename ServerType, typename CallDataType>
class OnetoMCallData
    : public std::enable_shared_from_this<
          OnetoMCallData<S, TRequest, TReply, RequestFuncType, ServerType, CallDataType>>,
      public AbstractCommonCallData
{
    using TRequestNoPtr = typename std::remove_pointer<TRequest>::type;
    using SignalFuncType = std::_Mem_fn<void (ServerType::*)(std::shared_ptr<CallDataType>)>;

public:
    explicit OnetoMCallData(typename S::AsyncService *service,
                            ServerCompletionQueue *cq,
                            RequestFuncType func,
                            SignalFuncType signal,
                            SignalFuncType deleteSignal,
                            ServerType *server)
        : service_(service)
        , cq_(cq)
        , func_(func)
        , signal_(signal)
        , deleteSignal_(deleteSignal)
        , server_(server)
        , status_(CREATE)

    {
        Proceed(true);
    }
    ~OnetoMCallData() { qCDebug(qtgrpc_server) << "~OnetoMCallData" << stringWithThisName(); }
    virtual void Proceed(bool ok) override
    {
        if (ok == false) {
            server_->removeCalldata(this->shared_from_this());
            return;
        }


        if (status_ == CREATE) {
            ctx_.AsyncNotifyWhenDone(0);
            auto requestFn = std::bind(func_, service_, &ctx_, &request_, &responder_, cq_, cq_, this);
            requestFn();

            qCDebug(qtgrpc_server) << "OnetoMCallData::Proceed: New responder " <<  stringWithThisName();
            status_ = PROCESS;

        } else if (status_ == PROCESS) {
            if (reply_thread_created_) {
                cvWaitForNext.notify_one();
            }

            //создаем поток для ответа на подписку единственный раз в этой CallData
            if (!reply_thread_created_) {
                //создаем новый CallData типа CallDataType (CallDataType это сгенерированный  тип SendMessageCallData)
                auto newCallData = std::make_shared<CallDataType>(service_, cq_, server_);
                server_->addCalldata(newCallData);

                signal_(server_, std::dynamic_pointer_cast<CallDataType>(this->shared_from_this()));

                reply_thread_created_ = true;
                std::thread thread_ = std::thread([this]() {
                    //пока не пришел сигнал блокируемся
                    while (true) {
                          std::this_thread::sleep_for(std::chrono::milliseconds(10));
//                        auto tid = std::this_thread::get_id();
//                        std::ostringstream ss;
//                        ss << tid;
//                        std::string mystring = ss.str();

                        if (ctx_.IsCancelled()) {
                            qCDebug(qtgrpc_server) << "OnetoMCallData"
                                            << "client unexpectly disconnected" << stringWithThisName();
                            cvWaitForNext.notify_one();
                            status_ = FINISH;
                            deleteSignal_(server_,
                                          std::dynamic_pointer_cast<CallDataType>(
                                              this->shared_from_this()));

                            server_->removeCalldata(this->shared_from_this());
                            break;
                        }


                        if (got_reply_) {
                            qCDebug(qtgrpc_server) << "OnetoMCallData Write reply";

                            mReply_.lock();
                            responder_.Write(reply_, (void *) this);
                            got_reply_ = false;
                            mReply_.unlock();
                        } else if (finish_) {
                            qCDebug(qtgrpc_server) << "OnetoMCallData Finish";
                            status_ = FINISH;
                            responder_.Finish(grpc::Status::OK, this);
                            break;
                        }
                    }
                });
                thread_.detach();
            }

        } else if (status_ == FINISH) {
            cvWaitForNext.notify_one();
            qCDebug(qtgrpc_server) << "OnetoMCallData::Proceed: Removing Callldata ";
            deleteSignal_(server_, std::dynamic_pointer_cast<CallDataType>(this->shared_from_this()));
            server_->removeCalldata(this->shared_from_this());
        }
    }
    //отправить ответ на RPC
    //блокирует основной поток до отправки сообщения
    void sendReply(TReply reply)
    {
        qCDebug(qtgrpc_server) << "sendReply";
        mReply_.lock();
        reply_ = reply;
        got_reply_ = true;
        mReply_.unlock();
        if (status_ != FINISH) {
            //Ждем пока grpc не прислал next после responder_.Write
            std::unique_lock<std::mutex> lk(mWaitForNext);
            qCDebug(qtgrpc_server) << "sendReply cvWaitForNext";
            cvWaitForNext.wait(lk);
        }
    }
    //законччить данный RPC
    void finish()
    {
        finish_ = true;
        if (status_ != FINISH) {
            //Ждем пока grpc не прислал next после responder_.Finish
            std::unique_lock<std::mutex> lk(mWaitForNext);
            qCDebug(qtgrpc_server) << "finish cvWaitForNext";
            cvWaitForNext.wait(lk);
        }
    }
    //содержимое запроса RPC
    const TRequestNoPtr& request() {
        return request_; }

    //Id клиента данного RPC
    QString clientId()
    {
        auto clientMetadataMMap = ctx_.client_metadata();
        auto clientIdRef = clientMetadataMMap.find("clientid");
        std::string clientIdString(clientIdRef->second.begin(), clientIdRef->second.end());
        return QString::fromStdString(clientIdString);
    }

private:
    QString stringWithThisName() const {
        return QString("%1 0x%2").arg(typeid(CallDataType).name()).arg(reinterpret_cast<quintptr>(this), 10, 16, QChar('0'));
    }

    typename S::AsyncService *service_;
    ServerCompletionQueue *cq_;
    TRequestNoPtr request_;
    std::mutex mReply_;
    TReply reply_;
    ServerAsyncWriter<TReply> responder_{&ctx_};
    RequestFuncType func_;
    SignalFuncType signal_;
    SignalFuncType deleteSignal_;
    ServerType *server_;
    enum CallStatus { CREATE, PROCESS, FINISH };
    CallStatus status_; // The current serving state.

    bool got_reply_ = false;

    std::mutex mWaitForNext;
    std::condition_variable cvWaitForNext;

    bool reply_thread_created_ = false;
    bool finish_ = false;
};
