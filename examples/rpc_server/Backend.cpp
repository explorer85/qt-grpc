#include "Backend.h"
#include <QCoreApplication>

Backend::Backend()
{
    asyncServer = std::make_shared<AsyncChatServer>("0.0.0.0:50061");
    //SendMessage
    connect(asyncServer.get(),
            &AsyncChatServer::newCallDataSendMessage,
            this,
            [this](std::shared_ptr<AsyncChatServer::SendMessageCallData> callData) {
                qDebug() << "SendMessageCalldata::newCall "
                         << QString::fromStdString(callData->request().data()) << callData.get();

                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                Message m{0,
                          QString::fromStdString(callData->request().data())
                              + " sender: " + callData->clientId(),
                          commontypes::SendingState::kSending};
                int messageId = msgModel_.addMessage(m);

                if (replyToMessageDefferedEnabled_) {
                    defferedReplyToMessageCdMap_[messageId] = callData;
                } else {
                    sendReplyToMessage(callData, messageId);
                }

                QCoreApplication::processEvents();
            });

    //    connect(&asyncServer, &AsyncChatServer::finishCallDataSendMessage,
    //            this, [this](std::shared_ptr<AsyncChatServer::SendMessageCallData> callData){

    //        qDebug() << "SendMessageCallDataDeleted" << callData.get();
    //        subscribeToMessageCdMap.erase(callData->clientId());
    //        setClientsList();

    //    });

    //SubscribeToMessage
    connect(asyncServer.get(),
            &AsyncChatServer::newCallDataSubscribeToMessage,
            this,
            [this](std::shared_ptr<AsyncChatServer::SubscribeToMessageCallData> callData) {
                qDebug() << "SubscribeToMessageCallData::newCall " << callData.get();

                subscribeToMessageCdMap_[callData->clientId()] = callData;

                setClientsList();
            });

    connect(asyncServer.get(),
            &AsyncChatServer::finishCallDataSubscribeToMessage,
            this,
            [this](std::shared_ptr<AsyncChatServer::SubscribeToMessageCallData> callData) {
                // std::this_thread::sleep_for(std::chrono::milliseconds(3000));

                qDebug() << "SubscribeToMessageCallDataDeleted" << callData.get();
                subscribeToMessageCdMap_.erase(callData->clientId());
                setClientsList();
            });
}

void Backend::sendMessageFromExchange(QString message, int count)
{
    if (!subscribeToMessageCdMap_.empty()) {
        for (int i = 0; i < count; i++) {
            BinaryMessage msg;
            static int j = 0;
            QString sendmessage = message + QString::number(j++);
            msg.set_data(sendmessage.toStdString());

            for (auto cd : subscribeToMessageCdMap_) {
                qDebug() << "sendMessageFromExchange " << QString::fromStdString(msg.data());
                cd.second->sendReply(msg);

                Message m{0, QString::fromStdString(msg.data()), commontypes::SendingState::kSending};
                outMsgModel_.addMessage(m);
            }
        }
    }
}

void Backend::sendReplyToMessageDeffered(int messageId)
{
    qDebug() << "replyTo" << messageId;
    auto callData = defferedReplyToMessageCdMap_[messageId];
    defferedReplyToMessageCdMap_.erase(messageId);

    sendReplyToMessage(callData, messageId);
}

void Backend::setReplyToMessageDefferedEnabled(bool enable)
{
    replyToMessageDefferedEnabled_ = enable;
}

void Backend::setClientsList()
{
    QVariantList newClientsList;
    for (auto cd : subscribeToMessageCdMap_)
        newClientsList.append(cd.first);

    clientsList_ = newClientsList;
    clientsListChanged();
}

void Backend::sendReplyToMessage(std::shared_ptr<AsyncChatServer::SendMessageCallData> callData,
                                 int msgId)
{
    commontypes::MessageState mState;
    mState.set_state(commontypes::SendingState::kSended);
    callData->sendReply(mState);
    callData->finish();

    //обновляем сообщение в модели
    Message m{msgId,
              QString::fromStdString(callData->request().data()) + " sender: " + callData->clientId(),
              commontypes::SendingState::kSended};
    msgModel_.updateMessage(m);
}
