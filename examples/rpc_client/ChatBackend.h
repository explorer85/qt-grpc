#pragma once

#include <memory>
#include <QCoreApplication>
#include <QObject>
#include <QUuid>

#include "../protos/binarychat.qgrpc.client.h"
#include <functional>

#include "../common/MessagesModel.h"

using binarychat::BinaryMessage;
using commontypes::Empty;
using commontypes::MessageState;
using grpc::CompletionQueue;

class ChatBackend : public QObject
{
    Q_OBJECT
    Q_PROPERTY(MessagesModel *messagesModel READ messagesModel NOTIFY messagesModelChanged)
    Q_PROPERTY(MessagesModel *inMessagesModel READ inMessagesModel NOTIFY inMessagesModelChanged)
public:
    ChatBackend()
        : QObject(nullptr)
    {
        asyncGenericGreeter = std::make_unique<AsyncChatClient>("0.0.0.0:50061", "1");
        enableNetworking();
    }

    Q_INVOKABLE void enableNetworking()
    {
        auto responseCallBack = [this](::binarychat::BinaryMessage msg) mutable {
            qDebug() << "AsyncSubscribeToMessage::newData " << QString::fromStdString(msg.data());
            Message m{0, QString::fromStdString(msg.data()), 0};
            inMsgModel_.addMessage(m);
        };
        Empty request;
        asyncGenericGreeter->SubscribeToMessage(request, responseCallBack);
    }

    Q_INVOKABLE void disableNetworking() { asyncGenericGreeter->cancelCalls(); }

    Q_INVOKABLE void send(QString message, int count)
    {
        BinaryMessage msg;
        static int i = 0;
        for (int j = 0; j < count; j++) {
            Message m{0, message + QString::number(i), commontypes::SendingState::kSending};
            i++;
            m.id = msgModel_.addMessage(m);

            msg.set_data(m.text.toStdString());

            auto responseCallBack = [this, m](::commontypes::MessageState state) mutable {
                qDebug() << "New Response for " << m.id << "is " << state.state();
                if (state.state() != commontypes::SendingState::kSending) {
                    m.status = state.state();
                    msgModel_.updateMessage(m);
                }
            };
            asyncGenericGreeter->SendMessage(msg, responseCallBack);
        }

        qDebug() << "end of send";
    }

    MessagesModel *messagesModel() { return &msgModel_; }
    MessagesModel *inMessagesModel() { return &inMsgModel_; }
signals:
    void messagesModelChanged();
    void inMessagesModelChanged();

private:
    std::unique_ptr<AsyncChatClient> asyncGenericGreeter;

    MessagesModel msgModel_;
    MessagesModel inMsgModel_;
};
