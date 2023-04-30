#pragma once

#include <QSqlQuery>
#include <QString>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlRecord>
#include <QSqlResult>

#include <QRegularExpression>
#include <QFile>

#include <QDebug>

namespace db {

inline
    auto Def(){
    return QSqlDatabase::database();
}

inline
    bool OpenLocal(const QString &fileName, const QString &connectionName = ""){
    QSqlDatabase db;
    if(connectionName.isEmpty())
        db = QSqlDatabase::addDatabase("QSQLITE");
    else
        db = QSqlDatabase::addDatabase("QSQLITE",connectionName);
    db.setHostName("localhost");
    db.setDatabaseName(fileName);
    if(!db.open())
    {
        qCritical() << "Cannot open db : " << db.lastError();
        return false;
    }

    return true;
}

inline QString ResultToString(QSqlQuery* query){
    QString out{};
    const auto kColCount{query->record().count()};
    QString header{};
    for(int32_t i{}; i < kColCount; ++i){
        header += query->record().fieldName(i) + " ";
    }
    out += header+"\n";
    while (query->next()) {
        QString line{};
        for(int32_t i{}; i < kColCount; ++i){
            line += "\""+ query->value(i).toString() + "\" ";
        }
        out += line + "\n";
    }
    return out;
}


/*!
 * \brief Execute a SQL script
 * @warning It requires each statement which should be executed independantly to start on a new line
 * @note Each line ending with a ";" will be considered as an ending individual SQL query
 *
 * Ex: SELECT * FROM Table; SELECT * FROM Table1;
 * Will be considered as one unique request. Whereas
 * Ex:
 *      SELECT * FROM Table;
 *      SELECT * FROM Table1;
 * Will be considered as two different queries
 *
 * \param file_name
 * \param db
 * \return
 */
template<bool kPrintResults = false,bool kPrintSuccessfulQuery = false>
bool ExecScript(const QString& file_name,QSqlDatabase* db){
    qInfo() << "Executing SQL script:" << file_name;
    QFile file(file_name);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qCritical() << "Could not open SQL script";
        return false;
    }

    QTextStream in(&file);
    QString sql = in.readAll();
    file.close();

    static const QRegularExpression regex_comment{R"(^\s*--)", QRegularExpression::MultilineOption | QRegularExpression::CaseInsensitiveOption};
    static const QRegularExpression regex_query_end{R"(\s*;\s*$)", QRegularExpression::MultilineOption | QRegularExpression::CaseInsensitiveOption};
    static const QRegularExpression regex_check_file_end{R"(\w{2,})", QRegularExpression::CaseInsensitiveOption |
                                                                          QRegularExpression::DotMatchesEverythingOption};

    std::vector<std::tuple<QString,int>> queries{};

    QString current_query{};
    int32_t line_index{};
    int32_t starting_line_index{-1};
    for(const auto& line: sql.split('\n',Qt::KeepEmptyParts)){
        ++line_index;
        if(line.isEmpty()){
            continue;
        }
        if(regex_comment.match(line).hasMatch())
        {
            continue;
        }

        if(starting_line_index < 0){
            starting_line_index = line_index;
        }
        current_query += "\n" + line;
        if(regex_query_end.match(line).hasMatch())
        {
            queries.emplace_back(current_query,starting_line_index);
            current_query.clear();
            starting_line_index = -1;
        }
    }
    if(regex_check_file_end.match(current_query).hasMatch())
    {
        qWarning() << "Script may not close the last query properly:";
        qWarning() << current_query;
    }

    int success_count{};
    bool success{true};
    QSqlQuery query(*db);
    for(const auto& [query_str,line]: queries)
    {
        const auto query_success{query.exec(query_str)};
        if(query_success){
            if constexpr(kPrintSuccessfulQuery){
                qDebug().nospace().noquote() << "\tQuery (Succeeded) l." << line << ": " << query_str;
            }
            if constexpr(kPrintResults){
                qInfo().nospace().noquote() << ResultToString(&query);
            }
        }else{
            qWarning().nospace().noquote() << "\tQuery (Failed) l." << line << ": " << query_str;
            qWarning().nospace().noquote() << "=> Error: " << query.lastError();
        }

        success = success && query_success;
        if(query_success) success_count += 1;
    }

    qInfo().nospace().noquote() << "Successful query count: " << success_count << "/" << queries.size();

    return success;
}

/*!
 * \brief formatListForSQL : format a list like `{1,2,3}` in `"(1,2,3)"`
 * \param in : input list
 * \return formatted string
 */
template<typename T>
inline
    QString formatListForSQL(const QVector<QVariant>& in)
{
    QString out{'('};
    for(const auto& e : in){
        out = out+QString("%0,").arg(e.value<T>());
    }
    if(out.endsWith(','))
        out.resize(out.size()-1);
    out += ")";
    return out;
}

template<typename T>
inline
    QString formatListForSQL(const QVector<T>& in)
{
    QString out{'('};
    for(const auto& e : in){
        out = out+QString("%0,").arg(e);
    }
    if(out.endsWith(','))
        out.resize(out.size()-1);
    out += ")";
    return out;
}


/*!
 * \brief questionMarkList : create a list of questino mark for SQL requests.
 *              For example : `{1,3,6,9}` will produce `"(?,?,?,?)"`
 * \param in
 * \return
 */
template<typename T>
inline
    QString questionMarkList(const QVector<T>& in)
{
    QString out{'('};
    for(const auto& e : in){
        out = out+"?,";
    }
    if(out.endsWith(','))
        out.resize(out.size()-1);
    out += ")";
    return out;
}

/*!
 * \brief prepRequestListFilter : prepare a list of elements for a SQL request
 * \param in : input list
 * \return a formatted string like (?,?,?,?) and a vector<QVariant> containing values to use in the sql req
*/
template<typename T>
inline
    std::tuple<QString,QVector<QVariant>> prepRequestListFilter(const QVector<T>& in)
{
    auto ouStr{questionMarkList(in)};
    QVector<QVariant> outL(in.size());
    for(int i{}; i < outL.size();++i)
    {
        outL[i] = QVariant{in[i]};
    }
    return {ouStr,outL};
}


}//namespace db
