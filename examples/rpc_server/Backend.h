#pragma once

#include "../common/MessagesModel.h"
#include "../protos/binarychat.qgrpc.server.h"
#include <QObject>

using namespace binarychat_server;

class Backend : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QVariantList clientsList READ clientsList NOTIFY clientsListChanged)
    Q_PROPERTY(MessagesModel *messagesModel READ messagesModel NOTIFY messagesModelChanged)
    Q_PROPERTY(MessagesModel *outMessagesModel READ outMessagesModel NOTIFY outMessagesModelChanged)
public:
    Backend();

    Q_INVOKABLE void sendMessageFromExchange(QString message, int count);
    Q_INVOKABLE void sendReplyToMessageDeffered(int messageId);
    Q_INVOKABLE void setReplyToMessageDefferedEnabled(bool enable);

    QVariantList clientsList() { return clientsList_; }
    void setClientsList();
    MessagesModel *messagesModel() { return &msgModel_; }
    MessagesModel *outMessagesModel() { return &outMsgModel_; }
signals:
    void clientsListChanged();
    void messagesModelChanged();
    void outMessagesModelChanged();

private:
    QVariantList clientsList_;

    std::shared_ptr<AsyncChatServer> asyncServer;
    //CallData для отложенной отправки ответа на сообщение клиенту
    std::map<int, std::shared_ptr<AsyncChatServer::SendMessageCallData>> defferedReplyToMessageCdMap_;
    bool replyToMessageDefferedEnabled_ = true;
    //CallData для постоянной отправки сообщений клиенту
    std::map<QString, std::shared_ptr<AsyncChatServer::SubscribeToMessageCallData>> subscribeToMessageCdMap_;

    MessagesModel msgModel_;
    MessagesModel outMsgModel_;

    void sendReplyToMessage(std::shared_ptr<AsyncChatServer::SendMessageCallData>, int msgId);
};
