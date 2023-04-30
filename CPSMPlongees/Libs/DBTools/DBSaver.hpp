#pragma once

/*!
 * This file contains every utility function used to create database saves
*/

#include <QDate>
#include <QJsonObject>
#include <QFileInfo>
#include <QDir>
#include <any>

namespace db{

struct DBSaverInfo
{
    QString db_path;
    QString saveFolder{QDir{"Save/"}.absolutePath()};

    int frequentSaveCount{7};//how many save we should keep
    int spacedSaveCount{3};//how many save we should keep

    int frequentSaveInterval{0};//how many days between two saves
    int spacedSaveInterval{30};//how many days between two saves

    QString date_format{};

    QString FrequentSaveFolder()const{
        return QDir{saveFolder+"/"+m_frequentSaveFolder+"/"}.absolutePath();
    }
    QString SpacedSaveFolder()const{
        return QDir{saveFolder+"/"+m_spacedSaveFolder+"/"}.absolutePath();
    }

private:
    const QString m_frequentSaveFolder{"Frequent"};
    const QString m_spacedSaveFolder{"Spaced"};
};

//-------------------------------------------------------------------------------------------------

bool refreshSave(const DBSaverInfo &info);


//-------------------------------------------------------------------------------------------------

//QJsonObject dbSaverInfoToJson(gens::SetPtr ptr);
//std::any dbSaverInfoFromJson(const QJsonObject&);


}//namespace db
