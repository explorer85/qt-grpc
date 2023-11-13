import QtQuick 2.11
import QtQuick.Window 2.11
import QtQuick.Controls 2.4
import QtQuick.Layouts 1.1

Window {
    visible: true
    width: 1000
    height: 600
    title: qsTr("Chat Server")

    ColumnLayout {
        anchors.fill: parent

        RowLayout {
            Button {
                text: "send Message from Exchange"
                onClicked: {
                    backend.sendMessageFromExchange(tfMessageText.text, repeatCount.value)
                }
            }
            TextField {
                id: tfMessageText
                text: "message from exchange"
            }
            Label {
                text: "повторить: "

            }
            SpinBox {
                id: repeatCount
                value: 3

            }
        }

        Label {
            text: "Активные клиенты"
        }
        ListView {
            width: 400
            height: 50
            ScrollBar.vertical: ScrollBar{
                policy: ScrollBar.AlwaysOn
            }

            model: backend.clientsList
            delegate: Rectangle {
                width: parent.width
                height: 20
                Text {
                    id: name
                    text: modelData
                }
            }
        }

        RowLayout {
            ColumnLayout {
                Layout.fillWidth: true
                Layout.preferredWidth: 3

                Label {
                    Layout.preferredHeight: 10
                    Layout.fillWidth: true
                    text: "Входящие Сообщения"

                }
                CheckBox {
                    id: cbDefferedReply
                    checked: true
                    onCheckStateChanged: {
                        backend.setReplyToMessageDefferedEnabled(checked)
                        console.log("55555  " + checked)
                    }
                }

                ListView {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    id: lvInMessages
                    ScrollBar.vertical: ScrollBar{
                        policy: ScrollBar.AlwaysOn
                    }
                    Connections {
                        target: backend.messagesModel
                        onRowsInserted: {
                            lvInMessages.positionViewAtEnd();
                        }
                    }



                    model: backend.messagesModel
                    delegate: ItemDelegate {
                        width: parent.width
                        height: 20
                        RowLayout {
                            anchors.fill: parent
                            Label {
                                Layout.fillWidth: true
                                Layout.preferredWidth: 6
                                Layout.preferredHeight: parent.height-2
                                text: messageText
                                background: Rectangle {
                                    color: messageStatus == 0 ? "yellow" : "lightGreen"
                                }
                            }
                                Button {
                                    id: btnSend
                                    Layout.fillWidth: true
                                    Layout.preferredWidth: 1
                                    Layout.preferredHeight: parent.height - 2
                                    text: "sendReply"
                                    enabled: messageStatus == 0
                                    onClicked: {
                                        backend.sendReplyToMessageDeffered(messageId)

                                    }
                                }


                        }
                    }
                }
            }
            //исходящие
            ColumnLayout {
                Layout.fillWidth: true
                Layout.preferredWidth: 2

                Label {
                    Layout.preferredHeight: 10
                    Layout.fillWidth: true
                    text: "Исходящие Сообщения"
                }
                ListView {
                    id: lvOutMessages
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    ScrollBar.vertical: ScrollBar{
                        policy: ScrollBar.AlwaysOn
                    }
                    Connections {
                        target: backend.outMessagesModel
                        onRowsInserted: {
                            lvOutMessages.positionViewAtEnd();
                        }
                    }


                    model: backend.outMessagesModel
                    delegate: ItemDelegate {
                        width: parent.width
                        height: 20
                        Label {
                            text: messageText
                            background: Rectangle {
                                color: messageStatus == 0 ? "yellow" : "lightGreen"
                            }

                        }
                    }
                }
            }
        }
    }
}
