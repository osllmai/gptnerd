#pragma once

#include "abstractchatprovider.h"

class QNetworkAccessManager;
class OpenAI : public AbstractChatProvider
{
    Q_OBJECT
    Q_PROPERTY(QString apiKey READ apiKey WRITE setApiKey NOTIFY apiKeyChanged FINAL)

public:
    OpenAI(const QString &apiKey, QObject *parent = nullptr);

    void prompt(const QString &message) override;
    QString apiKey() const;
    void setApiKey(const QString &newApiKey);

private Q_SLOTS:
    void onReplyFinished() ;

Q_SIGNALS:
    void apiKeyChanged();

private:
    QString m_apiKey;
    QNetworkAccessManager* networkManager;

};