#ifndef CHATLISTMODEL_H
#define CHATLISTMODEL_H

#include <QObject>
#include <QQmlEngine>
#include <QtQml>

#include <QAbstractListModel>
#include "chat.h"

class ChatListModel : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(Chat *currentChat READ currentChat WRITE setCurrentChat NOTIFY currentChatChanged)
    enum ChatlRoles {
        IdRole = Qt::UserRole + 1,
        TitleRole,
    };
public:
    explicit ChatListModel(QObject *parent );

    //*------------------------------------------------------------------------------****************************------------------------------------------------------------------------------*//
    //*-------------------------------------------------------------------------------* QAbstractItemModel interface*------------------------------------------------------------------------------*//
    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;
    //*---------------------------------------------------------------------------* end QAbstractItemModel interface *----------------------------------------------------------------------------*//


    //*----------------------------------------------------------------------------------------***************----------------------------------------------------------------------------------------*//
    //*----------------------------------------------------------------------------------------* Read Property  *----------------------------------------------------------------------------------------*//
    Chat *currentChat() const;
    //*--------------------------------------------------------------------------------------* end Read Property *-------------------------------------------------------------------------------------*//


    //*----------------------------------------------------------------------------------------***************----------------------------------------------------------------------------------------*//
    //*----------------------------------------------------------------------------------------* Write Property  *----------------------------------------------------------------------------------------*//
    void setCurrentChat(Chat *chat);
    //*-------------------------------------------------------------------------------------* end Write Property *--------------------------------------------------------------------------------------*//

    Q_INVOKABLE void addChat();
    Q_INVOKABLE Chat* getChat(int index);
    Q_INVOKABLE void deleteChat(int index);

signals:
    void currentChatChanged();

public slots:

private:
    Chat* m_currentChat = nullptr;
    QList<Chat*> chats;
};

#endif // CHATLISTMODEL_H
