#pragma once

#include <QStringList>
#include <QVector>

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>

#include <concepts>
#include <functional>

namespace db
{

#ifdef CMAKE_DEBUG_MODE
constexpr bool kDebugQueries{true};
#else
constexpr bool kDebugQueries{true};
#endif

inline
QDebug operator<<(QDebug debug, const QSqlQuery& query)
{
    QDebugStateSaver saver(debug);

    auto err{query.lastError()};
    auto qText{query.lastQuery()};
    debug << ":error:" << err.text()<<">>>";
    for(const auto& e : query.boundValues())
    {
        qText = qText.replace(qText.indexOf('?'),1,e.toString());
    }
    debug <<"    "<<qText;

    return debug;
}

inline void PrintError(const QSqlQuery& query, const QSqlError& err, const QString& caller_context = " - ", QDebug stream = qCritical()){
    QString errStr{QString{"%0 : SQL error : %1"}.arg(caller_context,err.text())};
    stream << errStr;
    stream << query;
}

//----------------------------------------------------------------

static QString c_extension_DB{".db"};//extension used to read/write files

bool openLocal(const QString &fileName, const QString &connectionName = "");

int countField(QSqlDatabase& db,const QString& table,const QString& field,QVariant value);

QVector<QVector<QVariant>> querySelect(QSqlDatabase db, QString request, const QStringList& argList, const QVector<QVariant>& valList);

//return the number of row matching the query result
/*!
 * \brief queryCount : return the count of occurences for a standard "SELECT X FROM Y...". Note that this function expect
 * this pattern to be followed.
 * \param db
 * \param request
 * \param argList
 * \param valList
 * \return
 */
int queryCount(QSqlDatabase& db,QString request,const QStringList& argList,const QVector<QVariant>& valList);

bool queryExist(QSqlDatabase& db,QString request,const QStringList& argList,const QVector<QVariant>& valList);


bool queryDelete(QSqlDatabase& db,QString request,const QStringList& argList,const QVector<QVariant>& valList);


int getLastInsertId(const QSqlDatabase& db, QString table);

//----------------------------------------------------------------

template<typename T,typename UnaryFunction>
concept ReadFromDBExtractor = requires(UnaryFunction f){
    std::invocable<UnaryFunction&,const QSqlQuery&>;
    {f(QSqlQuery())} -> std::convertible_to<T>;
};

/*
Example of lambda for "extractValue":
[&](QSqlDatabase& db) -> int
{
    int out{};
    out.id = query.value(0).value<int>();
    return out;
}

Example call :
auto results{db::readLFromDB<int>(db,[&](const QSqlQuery& q)->int{return q.value(0).value<int>();},"SELECT id FROM %0",{global::table_dives},{})};

*/

template<typename T,typename UnaryFunction>
requires ReadFromDBExtractor<T, UnaryFunction>
inline
QVector<T> readLFromDB(const QSqlDatabase& db,UnaryFunction extractValue,QString request,const QStringList& argList,const QVector<QVariant>& valList)
{
    QVector<T> out{};

    QSqlQuery query{db};

    for(const auto& e : argList)//match argument list
    {
        request = request.arg(e);
    }

    query.prepare(request);//prepare sql query

    for(const auto& e : valList) //bind all values (this way add some injection protection)
    {
        query.addBindValue(e);
    }

    query.exec();

    auto err{query.lastError()};
    if(err.type() != QSqlError::ErrorType::NoError)
    {
        PrintError(query,err,__PRETTY_FUNCTION__);
        return out;
    }

    out.reserve(1000);

    while(query.next())//while there is results
    {
        out.append(extractValue(query));
    }
    out.shrink_to_fit();

    return out;
}

template<typename T>
requires ReadFromDBExtractor<T,std::function<T(const QSqlQuery&)>>
inline
QVector<T> readLFromDB(const QSqlDatabase& db,std::function<T(const QSqlQuery&)> extractValue,QString request,const QStringList& argList,const QVector<QVariant>& valList)
{
    return readLFromDB(db,[&](const QSqlQuery& q)->T{
        return extractValue(q);
    },std::move(request),argList,valList);
}

template<typename T,typename UnaryFunction>
requires ReadFromDBExtractor<T, UnaryFunction>
inline
T readFromDB(const QSqlDatabase& db,UnaryFunction extractValue,QString request,const QStringList& argList,const QVector<QVariant>& valList)
{
    QSqlQuery query{db};

    for(const auto& e : argList)//match argument list
    {
        request = request.arg(e);
    }

    query.prepare(request);//prepare sql query

    for(const auto& e : valList) //bind all values (this way add some injection protection)
    {
        query.addBindValue(e);
    }

    query.exec();

    auto err{query.lastError()};
    if(err.type() != QSqlError::ErrorType::NoError)
    {
        PrintError(query,err,__PRETTY_FUNCTION__);
        return {};
    }

    if(!query.next())//if there is no result
    {
        if constexpr(kDebugQueries)
        {
            QString errStr{QString{"%0 : SQL no result found : %1"}.arg(__PRETTY_FUNCTION__,err.text())};
            qDebug() << query;
            qCritical() << errStr;
        }
        return {};
//        throw std::runtime_error{errStr.toStdString()};
    }

    return extractValue(query);
}

}//namespace db
