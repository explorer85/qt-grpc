#pragma once

#include "AbstractClientCall.h"
#include <QThread>

const std::string kClientidStr = "clientid";

template<typename T, typename ClientType>
class OneToMClientCall : public std::enable_shared_from_this<OneToMClientCall<T, ClientType>>,
        public AbstractClientCall
{
public:
    OneToMClientCall(std::function<void(T)> responseCallBack, ClientType* client) :
        responseCallBack_(responseCallBack), client_(client)
    {



        connect(this, &OneToMClientCall<T, ClientType>::newData, this, [this](){


            std::shared_ptr<OneToMClientCall<T, ClientType>> thisShPtr =
                     this->shared_from_this();
            qCDebug(qtgrpc_client) << "begin response callback" << thisShPtr->stringWithThisName();
            //после этой строки OneToMClientCall может быть удален!!!
            thisShPtr->responseCalled_ = true;
            thisShPtr->responseCallBack_(thisShPtr->data());
            qCDebug(qtgrpc_client) << "end response callback" ;



        }, Qt::QueuedConnection);
    }
    ~OneToMClientCall() {
        qCDebug(qtgrpc_client) << "OneToMClientCall dtor" << stringWithThisName();
    }


    std::unique_ptr<grpc::ClientAsyncReader<T>> reader_;

private:
    QString stringWithThisName() const {
         return QString("%1 0x%2").arg(typeid(T).name()).arg(reinterpret_cast<quintptr>(this), 10, 16, QChar('0'));
     }

    void Proceed(bool ok) override
    {
        if (!ok) {

            while (!responseCalled_) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                qCDebug(qtgrpc_client) << "wait for responseCalled_" << stringWithThisName();

            }

            std::weak_ptr<OneToMClientCall<T, ClientType>> w_ptrThis = this->shared_from_this();
            client_->removeClientCall(w_ptrThis);

            return;
        }

        while (reader_.get() == nullptr) {
        }

        reader_->Read(&data_, (void*)this);

        //не генерируем сигнал с данными при первом приходе
        if (!firstTimeReadPassed_) {
            firstTimeReadPassed_ = true;
            return;
        } else {
            std::lock_guard<std::mutex> lock(mMessages_);
            incomingMessages_.push(data_);
            qCDebug(qtgrpc_client) << "Proceed";
            responseCalled_ = false;
            emit newData();
            qCDebug(qtgrpc_client) << "EndOf EmitNewData";
        }
    }
    T data()
    {
        std::lock_guard<std::mutex> lock(mMessages_);
        if (!incomingMessages_.empty()) {
            T newData = incomingMessages_.front();
            incomingMessages_.pop();
            return newData;
        } else {
            return T();
        }
    }

    std::function<void(T)> responseCallBack_;

    Status status_;
    T data_;
    ClientType *client_;
    bool firstTimeReadPassed_{false};

    std::mutex mMessages_;
    std::queue<T> incomingMessages_;

    bool responseCalled_ = true;
};
