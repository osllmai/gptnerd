#ifndef MODELLIST_H
#define MODELLIST_H

#include <QObject>
#include <QQmlEngine>
#include <QtQml>
#include <QFileInfo>
#include <QDebug>
#include <QFile>
#include <QDir>
#include <QVariant>
#include <QVariantMap>
#include <QIODevice>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QMap>

#include <QAbstractListModel>
#include "model.h"
#include "download.h"
#include "currentmodellist.h"
#include "database.h"

class ModelList : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(CurrentModelList *currentModelList READ currentModelList WRITE setCurrentModelList NOTIFY currentModelListChanged)
    Q_PROPERTY(int downloadProgress READ downloadProgress NOTIFY downloadProgressChanged)

    enum ChatlRoles {
        IdRole = Qt::UserRole + 1,
        FileSizeRole,
        RamRamrequiredRole,
        ParametersRole,
        QuantRole,
        TypeRole,
        PromptTemplateRole,
        SystemPromptRole,
        NameRole,
        InformationRole,
        FileNameRole,
        UrlRole,
        DirectoryPathRole,
        IconModelRole,
        DownloadPercentRole,
        IsDownloadingRole,
        DownloadFinishedRole,
        BackendTypeRole
    };



public:
    enum class BackendType {
        LocalModel,
        OnlineProvider
    };
    Q_ENUM(BackendType)

    explicit ModelList(QObject *parent = nullptr);
<<<<<<< HEAD
    Q_INVOKABLE void downloadRequest(const int index, QString directoryPath);
    Q_INVOKABLE void cancelRequest(const int index);
    Q_INVOKABLE void deleteRequest(const int index);
    Q_INVOKABLE void addModel(QString directoryPath);
=======
    Q_INVOKABLE void downloadRequest(int index , const QString &directoryPath);
    Q_INVOKABLE void cancelRequest(int index);
    Q_INVOKABLE void deleteRequest(int index);
    Q_INVOKABLE void addModel(const QString &directoryPath);
>>>>>>> 584d027 (--wip-- [skip ci])

    //*------------------------------------------------------------------------------****************************------------------------------------------------------------------------------*//
    //*-------------------------------------------------------------------------------* QAbstractItemModel interface*------------------------------------------------------------------------------*//
    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;
    //*---------------------------------------------------------------------------* end QAbstractItemModel interface *----------------------------------------------------------------------------*//


    //*----------------------------------------------------------------------------------------***************---------------------------------------------------------------------------------------*//
    //*----------------------------------------------------------------------------------------* Read Property  *----------------------------------------------------------------------------------------*//
    CurrentModelList *currentModelList() const;
    double downloadProgress() const;
    //*--------------------------------------------------------------------------------------* end Read Property *-------------------------------------------------------------------------------------*//


    //*----------------------------------------------------------------------------------------***************---------------------------------------------------------------------------------------*//
    //*----------------------------------------------------------------------------------------* Write Property  *----------------------------------------------------------------------------------------*//
    void setCurrentModelList(CurrentModelList *currentModelList);
    //*-------------------------------------------------------------------------------------* end Write Property *--------------------------------------------------------------------------------------*//

public slots:
    void handleDownloadProgress(int index, qint64 bytesReceived, qint64 bytesTotal);
    void handleDownloadFinished(int index);

signals:
    void currentModelListChanged();
    void downloadProgressChanged();

private:
    class OnlineProviderData {
    public:
        QString name;
        QString description;
    };
    class DataItem {
    public:
        DataItem(QSharedPointer<OnlineProviderData> provider);
        DataItem(QSharedPointer<Model> model);

        const BackendType type;
        const QSharedPointer<OnlineProviderData> provider;
        const QSharedPointer<Model> model;
    };
    QList<DataItem*> _data;
    // QList<Model*> models;
    QList<Download*>downloads;
    CurrentModelList *m_currentModelList;
    int m_downloadProgress;

    void readModelFromJSONFile();
    void updateDownloadProgress();
    void deleteDownloadModel(const int index);
    void loadLocalModelsFromJson(QJsonArray jsonArray);
    void loadOnlineProvidersFromJson(QJsonArray jsonArray);
};

#endif // MODELLIST_H
