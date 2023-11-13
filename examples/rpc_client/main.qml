import QtQuick 2.11
import QtQuick.Window 2.11
import QtQuick.Controls 2.4
import QtQuick.Layouts 1.1

Window {
    visible: true
    width: 840
    height: 480
    title: qsTr("Chat Client")
    ColumnLayout {
        anchors.fill: parent
        RowLayout {
            Button
            {
                text: "send Message"
                onClicked: {
                    chatBackend.send(tfMessageText.text, sbRepeatCount.value)
                }
            }
            TextField {
                id: tfMessageText
                text: "test message"
            }
            Label {
                text: "повторить: "

            }
            SpinBox {
                id: sbRepeatCount
                value: 5

            }
            Button {
                id: startButton
                text: "enableNetworking"
                onClicked: {
                    chatBackend.enableNetworking()
                }
            }
            Button {
                id: stopButton
                text: "disableNetworking"
                onClicked: {
                    chatBackend.disableNetworking()
                }
            }
        }

        RowLayout {
            ColumnLayout {
                Layout.fillWidth: true
                Layout.preferredWidth: 1
                Label {
                    Layout.preferredHeight: 10
                    Layout.fillWidth: true
                    text: "Отправленные сообщения"
                }

                ListView {
                    id: lvOutMessages
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    ScrollBar.vertical: ScrollBar{
                        policy: ScrollBar.AlwaysOn
                    }
                    Connections {
                        target: chatBackend.messagesModel
                        onRowsInserted: {
                            lvOutMessages.positionViewAtEnd();
                        }
                    }

                    model: chatBackend.messagesModel
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

            //входящие
            ColumnLayout {
                Layout.fillWidth: true
                Layout.preferredWidth: 1
                Label {
                    Layout.preferredHeight: 10
                    Layout.fillWidth: true
                    text: "Входящие Сообщения"
                }
                ListView {
                    id: lvInMessages
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    ScrollBar.vertical: ScrollBar{
                        policy: ScrollBar.AlwaysOn
                    }
                    Connections {
                        target: chatBackend.inMessagesModel
                        onRowsInserted: {
                            lvInMessages.positionViewAtEnd();
                        }
                    }

                    model: chatBackend.inMessagesModel
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
