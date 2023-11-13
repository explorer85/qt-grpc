#include "MessagesModel.h"
#include <QDebug>

MessagesModel::MessagesModel(QObject *parent)
    : QAbstractListModel(parent)
{}

int MessagesModel::addMessage(Message msg)
{
    msg.id = nextId_;
    nextId_++;
    qDebug() << "addMessage" << msg.id << msg.text;
    beginInsertRows(QModelIndex(), messages_.size(), messages_.size());
    messages_[msg.id] = msg;
    endInsertRows();
    emit messageAdded();
    return msg.id;
}

void MessagesModel::updateMessage(const Message &msg)
{
    qDebug() << "updateMessage" << msg.status << msg.id;
    auto it = messages_.find(msg.id);
    if (it != messages_.end()) {
        messages_[msg.id] = msg;
        int pos = std::distance(messages_.begin(), it);
        qDebug() << "update at" << pos;
        emit dataChanged(index(pos, 0), index(pos, 0));
    }
}

//void MessagesModel::removeItem(quint8 number)
//{
//    beginRemoveRows(QModelIndex(), number, number);
//    messages_.erase(number);
//    endRemoveRows();
//}

QVariant MessagesModel::data(const QModelIndex &index, int role) const
{
    auto it = messages_.begin();
    std::advance(it, index.row());

    switch (role) {
    case IdRole:
        return it->second.id;
    case TextRole:
        return it->second.text;
    case StatusRole:
        return it->second.status;
    default:
        return "";
    }
}

int MessagesModel::rowCount(const QModelIndex & /*parent*/) const
{
    return messages_.size();
}

QHash<int, QByteArray> MessagesModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[IdRole] = "messageId";
    roles[TextRole] = "messageText";
    roles[StatusRole] = "messageStatus";
    return roles;
}
