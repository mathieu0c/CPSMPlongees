#pragma once

#include <DBUtils.hpp>

namespace cpsm::db
{

template<bool kDeleteOnCreation = false, bool kFillDBWithTestsValues = false>
bool InitDB(const auto& kFileName){
    qInfo() << "--------- DB init function ---------";

    if constexpr(kDeleteOnCreation){
#ifdef CMAKE_DEBUG_MODE
        qInfo() << "Deleting DB file: " << kFileName;
        qInfo() << QFile::remove(kFileName);
#else
#warning Requesting to delete DB on creation even when not in DEBUG mode
#endif
    }

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

    if constexpr(kFillDBWithTestsValues){
#ifndef CMAKE_DEBUG_MODE
#warning Requesting to fill db with tests values not in DEBUG mode
#endif
        if(!::db::ExecScript(":/DBScripts/FillTestDB.sql",&db)){
            qCritical() << "Failed to execute fill test DB script";
            return false;
        }else{
            qInfo() << "Successfully executed fill test DB script";
        }
    }

    qInfo() << "--------- DB init function END ---------";
    return true;
}

}//namespace cpsm::db
