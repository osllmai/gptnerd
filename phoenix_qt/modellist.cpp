#include "modellist.h"

#include <QDebug>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QtSql>

ModelList::ModelList(QObject *parent)
    : QAbstractListModel(parent)
    , m_currentModelList(new CurrentModelList(this))
    , m_downloadProgress(0)
{
    //read from database
    _models = phoenix_databace::readModel();

    readModelFromJSONFile();
}

//*------------------------------------------------------------------------------**************************-----------------------------------------------------------------------------*//
//*------------------------------------------------------------------------------* QAbstractItemModel interface *------------------------------------------------------------------------------*//
int ModelList::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return _models.size();
}

QVariant ModelList::data(const QModelIndex &index, int role = Qt::DisplayRole) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= _models.count())
        return {};

    //The index is valid
    auto model = _models[index.row()];

    switch (static_cast<ChatlRoles>(role)) {
    case IdRole:
        return model->id();
    case NameRole:
        return model->name();
    case InformationRole:
        return model->information();
    case FileSizeRole:
        return model->fileSize();
    case RamRamrequiredRole:
        return model->ramRamrequired();
    case ParametersRole:
        return model->parameters();
    case QuantRole:
        return model->quant();
    case TypeRole:
        return model->type();
    case PromptTemplateRole:
        return model->promptTemplate();
    case SystemPromptRole:
        return model->systemPrompt();
    case FileNameRole:
        return model->fileName();
    case UrlRole:
        return model->url();
    case DirectoryPathRole:
        return model->directoryPath();
    case IconModelRole:
        return model->icon();
    case DownloadPercentRole:
        return model->downloadPercent();
    case IsDownloadingRole:
        return model->isDownloading();
    case DownloadFinishedRole:
        return model->downloadFinished();

    case BackendTypeRole:
        return QVariant::fromValue(model->backendType());
    }

    return {};
}

QHash<int, QByteArray> ModelList::roleNames() const
{
    // clang-format off
    return {
        {IdRole, "id"},
        {FileSizeRole, "fileSize"},
        {RamRamrequiredRole, "ramRamrequired"},
        {ParametersRole, "parameters"},
        {QuantRole, "quant"},
        {TypeRole, "type"},
        {PromptTemplateRole, "promptTemplate"},
        {SystemPromptRole, "systemPrompt"},
        {NameRole, "name"},
        {InformationRole, "information"},
        {FileNameRole, "fileName"},
        {UrlRole, "url"},
        {DirectoryPathRole, "directoryPath"},
        {IconModelRole, "icon"},
        {DownloadPercentRole, "downloadPercent"},
        {IsDownloadingRole, "isDownloading"},
        {DownloadFinishedRole, "downloadFinished"},
        {BackendTypeRole, "backendType"}
    };
    // clang-format on
}

Qt::ItemFlags ModelList::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;
    return Qt::ItemIsEditable;
}

bool ModelList::setData(const QModelIndex &index, const QVariant &value, int role)
{
    auto model = _models[index.row()];

    bool somethingChanged{false};

    switch (role) {
    case IdRole:
        if (model->id() != value.toInt()) {
            model->setId(value.toInt());
            somethingChanged = true;
        }
        break;
    case FileSizeRole:
        if (model->fileSize() != value.toDouble()) {
            model->setFileSize(value.toDouble());
            somethingChanged = true;
        }
        break;
    case RamRamrequiredRole:
        if (model->ramRamrequired() != value.toInt()) {
            model->setRamRamrequired(value.toInt());
            somethingChanged = true;
        }
        break;
    case ParametersRole:
        if (model->parameters() != value.toString()) {
            model->setParameters(value.toString());
            somethingChanged = true;
        }
        break;
    case QuantRole:
        if (model->quant() != value.toString()) {
            model->setQuant(value.toString());
            somethingChanged = true;
        }
        break;
    case TypeRole:
        if (model->type() != value.toString()) {
            model->setType(value.toString());
            somethingChanged = true;
        }
        break;
    case PromptTemplateRole:
        if (model->promptTemplate() != value.toString()) {
            model->setPromptTemplate(value.toString());
            somethingChanged = true;
        }
        break;
    case SystemPromptRole:
        if (model->systemPrompt() != value.toString()) {
            model->setSystemPrompt(value.toString());
            somethingChanged = true;
        }
        break;
    case NameRole:
        if (model->name() != value.toString()) {
            model->setName(value.toString());
            somethingChanged = true;
        }
        break;
    case InformationRole:
        if (model->information() != value.toString()) {
            model->setInformation(value.toString());
            somethingChanged = true;
        }
        break;
    case FileNameRole:
        if (model->fileName() != value.toString()) {
            model->setFileName(value.toString());
            somethingChanged = true;
        }
        break;
    case UrlRole:
        if (model->url() != value.toString()) {
            model->setUrl(value.toString());
            somethingChanged = true;
        }
        break;
    case DirectoryPathRole:
        if (model->directoryPath() != value.toString()) {
            model->setDirectoryPath(value.toString());
            somethingChanged = true;
        }
        break;
    case IconModelRole:
        if (model->icon() != value.toString()) {
            model->setIcon(value.toString());
            somethingChanged = true;
        }
        break;
    case DownloadPercentRole:
        if (model->downloadPercent() != value.toDouble()) {
            model->setDownloadPercent(value.toDouble());
            somethingChanged = true;
        }
        break;
    case IsDownloadingRole:
        if (model->isDownloading() != value.toBool()) {
            model->setIsDownloading(value.toBool());
            somethingChanged = true;
        }
        break;
    case DownloadFinishedRole:
        if (model->downloadFinished() != value.toBool()) {
            model->setDownloadFinished(value.toBool());
            somethingChanged = true;
        }
    }
    if (somethingChanged) {
        emit dataChanged(index, index, QVector<int>() << role);
        return true;
    }
    return false;
}
//*---------------------------------------------------------------------------* end QAbstractItemModel interface *----------------------------------------------------------------------------*//

//*----------------------------------------------------------------------------------------***************----------------------------------------------------------------------------------------*//
//*----------------------------------------------------------------------------------------* Read Property  *----------------------------------------------------------------------------------------*//
CurrentModelList *ModelList::currentModelList() const
{
    return m_currentModelList;
}
double ModelList::downloadProgress() const
{
    return m_downloadProgress;
}
//*--------------------------------------------------------------------------------------* end Read Property *-------------------------------------------------------------------------------------*//

//*----------------------------------------------------------------------------------------***************----------------------------------------------------------------------------------------*//
//*----------------------------------------------------------------------------------------* Write Property  *----------------------------------------------------------------------------------------*//
void ModelList::setCurrentModelList(CurrentModelList *currentModelList)
{
    if (m_currentModelList == currentModelList)
        return;
    m_currentModelList = currentModelList;
    emit currentModelListChanged();
}

Model *ModelList::at(int index) const
{
    return _models.at(index);
}

//*-------------------------------------------------------------------------------------* end Write Property *--------------------------------------------------------------------------------------*//

void ModelList::downloadRequest(int index , const QString &path){
    auto directoryPath = QString{path}.remove("file:///");

    Model *model = _models[index];

    model->setDirectoryPath(directoryPath+ "/" + model->fileName());
    model->setIsDownloading(true);

    Download *download = new Download(index, model->url(), model->directoryPath());
    if(downloads.size()<3){
        connect(download, &Download::downloadProgress, this, &ModelList::handleDownloadProgress, Qt::QueuedConnection);
        connect(download, &Download::downloadFinished, this, &ModelList::handleDownloadFinished, Qt::QueuedConnection);
        download->downloadModel();
    }
    downloads.append(download);

    emit dataChanged(createIndex(index, 0), createIndex(index, 0), {DirectoryPathRole, IsDownloadingRole});
}

void ModelList::setApiKey(int id, const QString &apiKey)
{
    auto modelIt = std::find_if(_models.begin(), _models.end(), [&id](Model *m) {
        return m->id() == id;
    });
    if (modelIt == _models.end())
        return;
    phoenix_databace::updateModelApiKey(id, apiKey);
    m_currentModelList->addModel(*modelIt);
}

void ModelList::addModel(const QString &path)
{
    auto directoryPath = QString{path}.remove("file:///");

    QFileInfo fileInfo(directoryPath);
    QString fileName = fileInfo.baseName();
    double fileSize = (fileInfo.size()/10000000)*0.01;

    //add from database
    Model *model =phoenix_databace::insertModel(fileName, directoryPath);
    model->setDownloadFinished(true);
    model->setFileSize(fileSize);
    model->setIcon("images/userIcon.svg");
    model->setInformation("This model has been successfully added to the application by you.");
    if(model != nullptr){
        const int index = _models.size();
        beginInsertRows(QModelIndex(), index, index);
        _models.append(model);
        endInsertRows();
        m_currentModelList->addModel(model);
    }
    emit currentModelListChanged();

}

void ModelList::handleDownloadProgress(const int index,
                                       const qint64 bytesReceived,
                                       const qint64 bytesTotal)
{
    Model *model = _models[index];
    qDebug() << static_cast<double>(bytesReceived) / static_cast<double>(bytesTotal);
    model->setDownloadPercent(static_cast<double>(bytesReceived) / static_cast<double>(bytesTotal));

    updateDownloadProgress();

    emit dataChanged(createIndex(index, 0), createIndex(index, 0), {DownloadPercentRole});
}

void ModelList::handleDownloadFinished(int index)
{
    Model *model = _models[index];
    model->setIsDownloading(false);
    model->setDownloadFinished(true);
    model->setDownloadPercent(0);
    phoenix_databace::updateModelPath(model->id(), model->directoryPath());

    m_currentModelList->addModel(model);

    updateDownloadProgress();
    deleteDownloadModel(index);

    emit dataChanged(createIndex(index, 0),
                     createIndex(index, 0),
                     {IsDownloadingRole, DownloadFinishedRole, DownloadPercentRole});
    emit currentModelListChanged();
}

void ModelList::cancelRequest(int index)
{
    Model *model = _models[index];
    for(int indexSearch =0 ;indexSearch<downloads.size() && indexSearch<3 ;indexSearch++)
        if(downloads[indexSearch]->index() == index)
            downloads[indexSearch]->cancelDownload();
    model->setIsDownloading(false);
    model->setDownloadFinished(false);
    model->setDownloadPercent(0);

    updateDownloadProgress();
    deleteDownloadModel(index);
    emit dataChanged(createIndex(index, 0), createIndex(index, 0), {DownloadFinishedRole, IsDownloadingRole, DownloadPercentRole});
}

void ModelList::deleteRequest(int index)
{
    Model *model = _models[index];

    model->setIsDownloading(false);
    model->setDownloadFinished(false);
    m_currentModelList->deleteModel(model);

    if (model->url() == "") {
        beginRemoveRows(QModelIndex(), index, index);
        phoenix_databace::deleteModel(model->id());
        delete _models.takeAt(index);
        endRemoveRows();

        // chat->unloadAndDeleteLater();
    } else if (model->directoryPath() != "") {
        QFile file(model->directoryPath());
        if (file.exists()) {
            file.remove();
        }
        phoenix_databace::updateModelPath(model->id(), "");
    }

    emit dataChanged(createIndex(index, 0),
                     createIndex(index, 0),
                     {DownloadFinishedRole, IsDownloadingRole});
    emit currentModelListChanged();
}

void ModelList::loadLocalModelsFromJson(QJsonArray jsonArray)
{
    for (const QJsonValue &value : jsonArray) {
        QJsonObject jsonObj = value.toObject();

        QString modelName = jsonObj["name"].toString();
        QString modelFilename = jsonObj["filename"].toString();
        double modelFilesize = jsonObj["filesize"].toString().toDouble();
        QString url = jsonObj["url"].toString();
        int ramRamrequired = jsonObj["ramrequired"].toString().toInt();
        QString parameters = jsonObj["parameters"].toString();
        QString quant = jsonObj["quant"].toString();
        QString type = jsonObj["type"].toString();
        QString description = jsonObj["description"].toString();
        QString promptTemplate = jsonObj["promptTemplate"].toString();
        QString systemPrompt = jsonObj["systemPrompt"].toString();
        QString icon = jsonObj["icon"].toString();

        auto existingModelIt = std::find_if(_models.begin(),
                                            _models.end(),
                                            [&modelName](Model *model) {
                                                return model->name() == modelName;
                                            });

        if (existingModelIt == _models.end()) {
            auto model = phoenix_databace::insertModel(modelName, "");
            if (model != nullptr) {
                model->setFileName(modelFilename);
                model->setFileSize(modelFilesize);
                model->setUrl(url);
                model->setRamRamrequired(ramRamrequired);
                model->setParameters(parameters);
                model->setQuant(quant);
                model->setType(type);
                model->setInformation(description);
                model->setPromptTemplate(promptTemplate);
                model->setSystemPrompt(systemPrompt);
                model->setIcon(icon);

                const int index = _models.size();
                beginInsertRows(QModelIndex(), index, index);
                _models << model;

                endInsertRows();
            }
        } else {
            auto existingModel = *existingModelIt;

            existingModel->setFileName(modelFilename);
            existingModel->setFileSize(modelFilesize);
            existingModel->setUrl(url);
            existingModel->setRamRamrequired(ramRamrequired);
            existingModel->setParameters(parameters);
            existingModel->setQuant(quant);
            existingModel->setType(type);
            existingModel->setInformation(description);
            existingModel->setPromptTemplate(promptTemplate);
            existingModel->setSystemPrompt(systemPrompt);
            existingModel->setIcon(icon);
        }
    }
}

void ModelList::loadOnlineProvidersFromJson(QJsonArray jsonArray)
{
    for (const QJsonValue &value : jsonArray) {
        QJsonObject jsonObj = value.toObject();

        if (jsonObj == QJsonObject{})
            continue;


        auto modelName = jsonObj.value("name").toString();

        auto existingModelIt = std::find_if(_models.begin(),
                                            _models.end(),
                                            [&modelName](Model *model) {
                                                return model->name() == modelName;
                                            });

        Model *model;
        if (existingModelIt == _models.end()) {
            model = phoenix_databace::insertModel(modelName,
                                                       "",
                                                       Model::BackendType::OnlineProvider);

            const int index = _models.size();
            beginInsertRows(QModelIndex(), index, index);
            _models << model;
            endInsertRows();

        } else {
            model = *existingModelIt;
            model->setInformation(jsonObj.value("description").toString());
            model->setBackendType(Model::BackendType::OnlineProvider);
        }
    }
}

void ModelList::readModelFromJSONFile()
{
    QFile file(qApp->applicationDirPath() + "/models.json");
    if (!file.open(QIODevice::ReadOnly)) {
        qFatal() << file.errorString();
        return;
    }
    QByteArray jsonData = file.readAll();
    file.close();

    QJsonParseError err;
    QJsonDocument document = QJsonDocument::fromJson(jsonData, &err);
    if (err.error != QJsonParseError::NoError) {
        qWarning() << "ERROR: Couldn't parse: " << jsonData << err.errorString();
        return;
    }

    if (!document.isObject()) {
        qWarning() << "ERROR: Couldn't parse: Document's root is not json object";
        return;
    }

    auto obj = document.object();
    loadLocalModelsFromJson(obj.value("offlineModels").toArray());
    loadOnlineProvidersFromJson(obj.value("onlineModels").toArray());

    auto index = static_cast<int>(_models.size()) - 1;

    std::for_each(_models.rbegin(), _models.rend(), [&index, this](Model *model) {
        switch (model->backendType()) {
        case Model::BackendType::LocalModel:
            if (model->downloadFinished()) {
                if (model->url().isEmpty()) {
                    QFileInfo fileInfo(model->directoryPath());
                    model->setFileSize((fileInfo.size() / 10000000) * 0.01);
                    model->setIcon("images/userIcon.svg");
                    model->setInformation(
                        "This model has been successfully added to the application by you.");
                }
                m_currentModelList->addModel(model);
            } else if (model->url().isEmpty()) {
                deleteRequest(index);
            }
            break;
        case Model::BackendType::OnlineProvider:
            if (!model->apiKey().isEmpty())
                m_currentModelList->addModel(model);
            break;
        }

        index--;
    });
}

void ModelList::updateDownloadProgress()
{
    double totalBytesDownload = 0;
    double receivedBytesDownload = 0;
    for (auto &&model : _models) {
        totalBytesDownload += 1;
        receivedBytesDownload += model->downloadPercent();
    }

    if (totalBytesDownload != 0)
        m_downloadProgress = (receivedBytesDownload / totalBytesDownload) * 100;
    else
        m_downloadProgress = 0;

    qInfo() << "m_downloadProgress:  " << m_downloadProgress;
    emit downloadProgressChanged();

}

void ModelList::deleteDownloadModel(const int index){
    if(downloads.size()>3){
        connect(downloads[3], &Download::downloadProgress, this, &ModelList::handleDownloadProgress, Qt::QueuedConnection);
        connect(downloads[3], &Download::downloadFinished, this, &ModelList::handleDownloadFinished, Qt::QueuedConnection);
        downloads[3]->downloadModel();
    }
    for(int searchIndex = 0; searchIndex<downloads.size(); searchIndex++){
        if(downloads[searchIndex]->index() == index){
            Download *download = downloads[searchIndex];
            downloads.removeAt(searchIndex);
            delete download;
        }
    }
}
