#ifndef DATABASE_H
#define DATABASE_H

#include <QDateTime>
#include <QSqlError>
#include <QSqlDatabase>
#include <QSqlQuery>
#include "model.h"
#include "chat.h"
#include "message.h"

 // TODO: convert this namespace to a singleton class
class PhoenixDatabace
{
public:
    static PhoenixDatabace *instance();

    // QSqlError initDb();

    // Model* insertModel(const QString &name, const QString &path, Model::BackendType backendType = Model::BackendType::LocalModel);

    // int insertConversation(const QString &title, const QDateTime date);
    // int insertMessage(const QString &text,
    //                   bool isPrompt,
    //                   int numberOfTokens,
    //                   int executionTime,
    //                   const Message *parent,
    //                   int conversation_id,
    //                   const QDateTime date);

    // QSqlError deleteModel(int id);
    // QSqlError deleteConversation(int id);

    // QList<Model*> readModel();
    // QList<Chat*> readConversation();
    // QSqlError readMessage(Message *root, int conversation_id);

    // QSqlError updateModelPath(int id, const QString &path);
    // QSqlError updateConversationName(int id, const QString &name);
    // QSqlError updateConversationDate(int id, const QDateTime date);
    // QSqlError updateModelApiKey(int id, const QString &apiKey);

private:
    QSqlDatabase _db;
};


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
                apiKey	TEXT,
                backendType	NUMERIC NOT NULL DEFAULT 0,
                PRIMARY KEY(id AUTOINCREMENT))
        )");

const auto INSERT_MODEL_SQL = QLatin1String(R"(
        INSERT INTO model(name, path, backendType) VALUES (?, ?, ?)
        )");

const auto READ_MODEL_SQL = QLatin1String(R"(
        SELECT id, name, path, apiKey, backendType FROM model
        )");

const auto UPDATE_PATH_MODEL_SQL = QLatin1String(R"(
        UPDATE model SET path=? Where id=?
        )");


const auto UPDATE_APIKEY_CONVERSATION_SQL = QLatin1String(R"(
        UPDATE model SET apiKey=? Where id=?
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
                PRIMARY KEY(id AUTOINCREMENT))
        )");

const auto INSERT_CONVERSATION_SQL = QLatin1String(R"(
        INSERT INTO conversation(title, date) VALUES (?, ?)
        )");

const auto READ_CONVERSATION_SQL = QLatin1String(R"(
        SELECT id, title, date FROM conversation
        )");

const auto UPDATE_DATE_CONVERSATION_SQL = QLatin1String(R"(
        UPDATE conversation SET date=? Where id=?
        )");

const auto UPDATE_TITLE_CONVERSATION_SQL = QLatin1String(R"(
        UPDATE conversation SET title=? Where id=?
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

Model* insertModel(const QString &name, const QString &path, Model::BackendType backendType = Model::BackendType::LocalModel);

int insertConversation(const QString &title, const QDateTime date);
int insertMessage(const QString &text,
                  const bool isPrompt,
                  const int numberOfTokens,
                  const int executionTime,
                  const Message *parent,
                  const int &conversation_id,
                  const QDateTime date);

QSqlError deleteModel(const int &id);
QSqlError deleteConversation(const int &id);

QList<Model*> readModel();
QList<Chat*> readConversation();
QSqlError readMessage(Message *root, const int &conversation_id);

QSqlError updateModelPath(int id, const QString &path);
QSqlError updateConversationName(int id, const QString &name);
QSqlError updateConversationDate(int id, const QDateTime date);
QSqlError updateModelApiKey(int id, const QString &apiKey);

//-------------------------------**End Function Query**---------------------------//
}
#endif // DATABASE_H
