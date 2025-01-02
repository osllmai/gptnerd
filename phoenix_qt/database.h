#ifndef DATABASE_H
#define DATABASE_H

#include <QDateTime>
#include <QSqlError>
#include <QSqlQuery>
#include "model.h"
#include "chat.h"
#include "message.h"

namespace phoenix_databace{
const auto FOREIGN_KEYS_SQL = QLatin1String(R"(
        PRAGMA foreign_keys = ON;
        )");

//------------------------------------**************-------------------------------//
//------------------------------------**Model Query**-------------------------------//
const auto MODEL_SQL = QLatin1String(R"(
        CREATE TABLE model(
                id INTEGER NOT NULL UNIQUE,
                name TEXT NOT NULL,
                path TEXT NOT NULL,
                PRIMARY KEY(id AUTOINCREMENT))
        )");

const auto INSERT_MODEL_SQL = QLatin1String(R"(
        INSERT INTO model(name, path) VALUES (?, ?)
        )");

const auto READ_MODEL_SQL = QLatin1String(R"(
        SELECT id, name, path FROM model
        )");

const auto UPDATE_PATH_MODEL_SQL = QLatin1String(R"(
        UPDATE model SET path=? Where id=?
        )");

const auto DELETE_MODEL_SQL = QLatin1String(R"(
        DELETE FROM model where id = ?
        )");
//---------------------------------**End Model Query**----------------------------//


//--------------------------------*******************---------------------------//
//--------------------------------**Conversation Query**--------------------------//
const auto CONVERSATION_SQL = QLatin1String(R"(
        CREATE TABLE conversation(
                id INTEGER NOT NULL UNIQUE,
                title TEXT NOT NULL,
                date DATE NOT NULL,
                stream BOOL NOT NULL,
                promptTemplate TEXT NOT NULL,
                systemPrompt TEXT NOT NULL,
                temperature REAL NOT NULL,
                topK INTEGER NOT NULL,
                topP REAL NOT NULL,
                minP REAL NOT NULL,
                repeatPenalty REAL NOT NULL,
                promptBatchSize INTEGER NOT NULL,
                maxTokens INTEGER NOT NULL,
                repeatPenaltyTokens INTEGER NOT NULL,
                contextLength INTEGER NOT NULL,
                numberOfGPULayers INTEGER NOT NULL,
                PRIMARY KEY(id AUTOINCREMENT))
        )");

const auto INSERT_CONVERSATION_SQL = QLatin1String(R"(
        INSERT INTO conversation(title, date, stream, promptTemplate,systemPrompt,
                                                    temperature, topK, topP, minP, repeatPenalty,
                                                    promptBatchSize, maxTokens, repeatPenaltyTokens, contextLength, numberOfGPULayers)
                                                    VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
        )");

const auto READ_CONVERSATION_SQL = QLatin1String(R"(
        SELECT id, title, date, stream, promptTemplate,systemPrompt,
                        temperature, topK, topP, minP, repeatPenalty,
                        promptBatchSize, maxTokens, repeatPenaltyTokens,
                        contextLength, numberOfGPULayers
        FROM conversation
        )");

const auto UPDATE_DATE_CONVERSATION_SQL = QLatin1String(R"(
        UPDATE conversation SET date=? Where id=?
        )");

const auto UPDATE_TITLE_CONVERSATION_SQL = QLatin1String(R"(
        UPDATE conversation SET title=? Where id=?
        )");

const auto UPDATE_MODEL_SETTINGS_CONVERSATION_SQL = QLatin1String(R"(
        UPDATE conversation
        SET stream=?, promptTemplate=?, systemPrompt=?,
                temperature=?, topK=?, topP=?, minP=?, repeatPenalty=?,
                promptBatchSize=?, maxTokens=?, repeatPenaltyTokens=?,
                contextLength=?, numberOfGPULayers=?
        Where id=?
        )");

const auto DELETE_CONVERSATION_SQL = QLatin1String(R"(
        DELETE FROM conversation where id = ?
        )");
//-----------------------------**End Conversation Query**-----------------------//


//----------------------------------****************------------------------------//
//----------------------------------**Message Query**------------------------------//
const auto MESSAGE_SQL = QLatin1String(R"(
        CREATE TABLE message(
                id INTEGER NOT NULL UNIQUE,
                text TEXT NOT NULL,
                is_prompt BOOL NOT NULL,
                number_of_token INTEGER,
                execution_time INTEGER,
                parent_id INTEGER,
                conversation_id INTEGER NOT NULL,
                date DATE NOT NULL,
                PRIMARY KEY(id AUTOINCREMENT),
                foreign key(parent_id) REFERENCES message(id) ON DELETE CASCADE,
                foreign key(conversation_id) REFERENCES conversation(id) ON DELETE CASCADE)
        )");

const auto INSERT_MESSAGE_SQL = QLatin1String(R"(
        INSERT INTO message (text, is_prompt, number_of_token, execution_time, parent_id, conversation_id, date) VALUES (?, ?, ?, ?, ?, ?, ?)
        )");

const auto FIND_ROOT_MESSAGE_SQL = QLatin1String(R"(
        SELECT id FROM message WHERE parent_id = null AND conversation_id =?
        )");

const auto FIND_CHILD_MESSAGE_SQL = QLatin1String(R"(
        SELECT id, text, is_prompt, number_of_token, execution_time, date FROM message WHERE parent_id = ? AND conversation_id =?
        )");

const auto DELETE_MESSAGE_SQL = QLatin1String(R"(
        DELETE FROM message where conversation_id = ?
        )");

//-------------------------------**End Message Query**----------------------------//


//----------------------------------****************------------------------------//
//----------------------------------**Function Query**------------------------------//
QSqlError initDb();

Model* insertModel(const QString &name, const QString &path);
int insertConversation(const QString &title, const QDateTime date, const bool &stream,
                    const QString &promptTemplate, const QString &systemPrompt, const double &temperature,
                    const int &topK, const double &topP, const double &minP, const double &repeatPenalty,
                    const int &promptBatchSize, const int &maxTokens, const int &repeatPenaltyTokens,
                    const int &contextLength, const int &numberOfGPULayers);
int insertMessage(const QString &text, const bool isPrompt, const int numberOfTokens,
                  const int executionTime, const Message *parent, const int &conversation_id,const QDateTime date);

QSqlError deleteModel(const int &id);
QSqlError deleteConversation(const int &id);

QList<Model*> readModel();
QList<Chat*> readConversation();
QSqlError readMessage(Message *root, const int &conversation_id);

QSqlError updateModelPath(const int &id, const QString &path);
QSqlError updateConversationName(const int &id, const QString &name);
QSqlError updateConversationDate(const int &id, const QDateTime date);

//-------------------------------**End Function Query**---------------------------//
}
#endif // DATABASE_H
