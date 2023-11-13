#include <QtTest>

#include "../protos/binarychat.qgrpc.client.h"
#include "../protos/binarychat.qgrpc.server.h"

using namespace binarychat_server;
using namespace binarychat;

class ClientServerTest : public QObject
{
    Q_OBJECT

public:
private slots:
    void testConstructDestructClient();
    void testConstructDestructServer();
    void testConstructDestruct();
    void testCancelCallsFromClient();
    void testCancelCalls();
    void testPingPong();
};

void ClientServerTest::testConstructDestructClient()
{
    AsyncChatClient asyncChatClient("0.0.0.0:50061", "1");
}

void ClientServerTest::testConstructDestructServer()
{
    AsyncChatServer asyncServer("0.0.0.0:50061");
}

void ClientServerTest::testConstructDestruct()
{
    AsyncChatServer asyncServer("0.0.0.0:50061");
    AsyncChatClient asyncChatClient("0.0.0.0:50061", "1");
}

//вызов AsyncChatClient::cancelCalls из тела лямбды для ответа на запрос
//раньше приводил к зависанию ожидания удаления всех выполнившихся запросов
void ClientServerTest::testCancelCallsFromClient()
{
    AsyncChatServer asyncChatServer("0.0.0.0:50061");
    connect(&asyncChatServer,
            &AsyncChatServer::newCallDataSendMessage,
            this,
            [&](std::shared_ptr<AsyncChatServer::SendMessageCallData> callData) {
                qDebug() << "SendMessageCalldata::newCall "
                         << QString::fromStdString(callData->request().data()) << callData.get();
                Reply reply;
                reply.set_result(true);
                callData->sendReply(reply);
                callData->finish();
            });

    AsyncChatClient asyncChatClient("0.0.0.0:50061", "1");
    BinaryMessage request;
    request.set_data("hello");
    asyncChatClient.SendMessage(request, [&asyncChatClient](binarychat::Reply reply) {
        qDebug() << "reply is" << reply.result();
        asyncChatClient.cancelCalls();
    });
    QTest::qWait(1000);
}

void ClientServerTest::testCancelCalls()
{
    for (int i = 0; i < 7; i++) {
        qDebug() << "testCancelCalls ITERATION";
        AsyncChatClient asyncChatClient("0.0.0.0:50061", "1");

        AsyncChatServer asyncChatServer("0.0.0.0:50061");
        connect(&asyncChatServer,
                &AsyncChatServer::newCallDataSendMessage,
                this,
                [&](std::shared_ptr<AsyncChatServer::SendMessageCallData> callData) {
                    qDebug() << "SendMessageCalldata::newCall "
                             << QString::fromStdString(callData->request().data()) << callData.get();
                    Reply reply;
                    reply.set_result(true);
                    callData->sendReply(reply);
                    // QTest::qWait(3000);
                    asyncChatClient.cancelCalls();
                    callData->finish();
                });

        BinaryMessage request;
        request.set_data("hello");
        asyncChatClient.SendMessage(request, [](binarychat::Reply reply) {
            qDebug() << "reply is" << reply.result();
        });

        qDebug() << "end of test";
        QTest::qWait(1000);
    }
}

void ClientServerTest::testPingPong()
{
    AsyncChatClient asyncChatClient("0.0.0.0:50061", "1");

    AsyncChatServer asyncChatServer("0.0.0.0:50061");
    connect(&asyncChatServer,
            &AsyncChatServer::newCallDataSendMessage,
            this,
            [&](std::shared_ptr<AsyncChatServer::SendMessageCallData> callData) {
                qDebug() << "SendMessageCalldata::newCall "
                         << QString::fromStdString(callData->request().data()) << callData.get();
                Reply reply;
                reply.set_result(true);
                callData->sendReply(reply);
                callData->sendReply(reply);
                // QTest::qWait(3000);
                callData->finish();
            });

    std::function<void(binarychat::Reply)> replyFunc = [](binarychat::Reply reply) {
        qDebug() << "reply is!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << reply.result();
    };

    for (int i = 0; i < 5; i++) {
        BinaryMessage request;
        request.set_data((QString("hello %1").arg(i)).toStdString());
        asyncChatClient.SendMessage(request, replyFunc);
    }

    QTest::qWait(1000);
}

QTEST_MAIN(ClientServerTest)
#include "clientservertest.moc"

//   QCOMPARE(res, true);
