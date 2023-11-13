#pragma once

#include <QAbstractListModel>

struct Message
{
    int id;
    QString text;
    int status;
};

class MessagesModel : public QAbstractListModel
{
    Q_OBJECT

public:
    MessagesModel(QObject *parent = 0);

    enum ExampleRoles { IdRole = Qt::UserRole + 1, TextRole, StatusRole };
    Q_ENUM(ExampleRoles);

    Q_INVOKABLE int addMessage(Message msg);
    Q_INVOKABLE void updateMessage(const Message &msg);
    //Q_INVOKABLE void removeItem(quint8 number);

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
signals:
    void messageAdded();

protected:
    QHash<int, QByteArray> roleNames() const override;

private:
    std::map<int, Message> messages_;
    int nextId_ = 0;
};
