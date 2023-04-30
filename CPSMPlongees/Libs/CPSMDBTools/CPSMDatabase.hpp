#pragma once

#include <DBUtils.hpp>

namespace cpsm::db
{

template<bool kDeleteOnCreation = false>
bool InitDB(const auto& kFileName){
    qInfo() << "--------- DB init function ---------";

#ifdef CMAKE_DEBUG_MODE
    if constexpr(kDeleteOnCreation){
        qInfo() << "Deleting DB file: " << kFileName;
        qInfo() << QFile::remove(kFileName);
    }
#endif

    if(!::db::OpenLocal(kFileName)){
        qCritical() << "Failed to open db:" << kFileName;
        return false;
    }

    QSqlDatabase db{QSqlDatabase::database()};

    if(!::db::ExecScript(":/DBScripts/CreateDB.sql",&db)){
        qCritical() << "Failed to execute DB creation script";
        return false;
    }else{
        qInfo() << "Successfully executed DB creation script";
    }

    if(!::db::ExecScript(":/DBScripts/SetupTriggers.sql",&db)){
        qCritical() << "Failed to setup DB triggers creation script";
        return false;
    }else{
        qInfo() << "Successfully executed DB triggers creation script";
    }

#ifdef CMAKE_DEBUG_MODE
    if(!::db::ExecScript(":/DBScripts/FillTestDB.sql",&db)){
        qCritical() << "Failed to execute fill test DB script";
        return false;
    }else{
        qInfo() << "Successfully executed fill test DB script";
    }
#endif

    qInfo() << "--------- DB init function END ---------";
    return true;
}

}//namespace cpsm::db
