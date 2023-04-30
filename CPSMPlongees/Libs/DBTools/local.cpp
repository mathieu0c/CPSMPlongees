//#include "DBApi/Database.hpp"

//#include "../global.hpp"

////#include "DataStruct/Diver.h"
////#include "DataStruct/Dive.h"
//#include "DBApi/DataStructs.hpp"
//#include "DBApi/DBDiver.hpp"
//#include "DBApi/DBDiverLevel.hpp"
//#include "DBApi/DBApi.hpp"

//#include "DBApi/Generic.hpp"

//#include <tuple>

//#include <QSqlDatabase>
//#include <QSqlQuery>
//#include <QSqlError>

//#include <QStringList>

//#include <QDebug>


//namespace db
//{

//bool createDB(QSqlDatabase db)
//{
//    using namespace global;

//    QString queryStr = "CREATE TABLE IF NOT EXISTS %1(id INTEGER PRIMARY KEY AUTOINCREMENT, "
//                       "level TEXT UNIQUE NOT NULL,"
//                       "sortValue INTEGER NOT NULL)";
//    auto err = QSqlQuery{queryStr.arg(table_diverLevel),db}.lastError();
//    if(err.type() != QSqlError::ErrorType::NoError)
//    {
//        QString errStr{QString{"%0 : SQL error : %1"}.arg(__func__,err.text())};
//        qCritical() << errStr;
//        return false;
//    }

//    auto addDiverLevel{
//        [&](const auto& name,const auto& sortValue)
//        {
//            auto id{storeInDB(data::DiverLevel{.level=name,.sortValue=sortValue},db,global::table_diverLevel)};
//            return id > 0;
//        }
//    };

//    auto sortLevelValue{0};
//    if(!addDiverLevel("Débutant",(sortLevelValue++)*10))
//        return false;
//    if(!addDiverLevel("PE12",(sortLevelValue++)*10))
//        return false;
//    if(!addDiverLevel("N1",(sortLevelValue++)*10))
//        return false;
//    if(!addDiverLevel("PE40",(sortLevelValue++)*10))
//        return false;
//    if(!addDiverLevel("PA20",(sortLevelValue++)*10))
//        return false;
//    if(!addDiverLevel("N2",(sortLevelValue++)*10))
//        return false;
//    if(!addDiverLevel("PA40",(sortLevelValue++)*10))
//        return false;
//    if(!addDiverLevel("PE60",(sortLevelValue++)*10))
//        return false;
//    if(!addDiverLevel("PA60",(sortLevelValue++)*10))
//        return false;
//    if(!addDiverLevel("N3",(sortLevelValue++)*10))
//        return false;
//    if(!addDiverLevel("N4",(sortLevelValue++)*10))
//        return false;
//    if(!addDiverLevel("E1",(sortLevelValue++)*10))//N2-3 initiateur
//        return false;
//    if(!addDiverLevel("E2",(sortLevelValue++)*10))//N4 initiateur
//        return false;
//    if(!addDiverLevel("E3",(sortLevelValue++)*10))//MF1
//        return false;
//    if(!addDiverLevel("E4",(sortLevelValue++)*10))//MF2
//        return false;



//    //table divers addresses

//    queryStr = "CREATE TABLE IF NOT EXISTS %1(id INTEGER PRIMARY KEY AUTOINCREMENT, "
//               "address TEXT, "
//               "postalCode TEXT, "
//               "city TEXT)";
//    err = QSqlQuery{queryStr.arg(table_diversAddresses),db}.lastError();
//    if(err.type() != QSqlError::ErrorType::NoError)
//    {
//        QString errStr{QString{"%0 : SQL error : %1"}.arg(__func__,err.text())};
//        qCritical() << errStr;
//        return false;
//    }


//    //table members
//    queryStr = "CREATE TABLE IF NOT EXISTS %1(id INTEGER PRIMARY KEY AUTOINCREMENT,"
//    "firstName TEXT, "
//    "lastName TEXT,"
//    "birthDate TEXT, "
//    "email TEXT, "
//    "phoneNumber TEXT, "
//    "memberAddressId INTEGER, "
//    "licenseNumber TEXT, "
//    "certifDate TEXT, "
//    "diverLevelId INTEGER, "
//    "member INTEGER CHECK(member=0 OR member=1), "
//    "registrationDate TEXT, "
//    "paidDives INTEGER, "
//    "gear_regulator INTEGER, "
//    "gear_suit INTEGER, "
//    "gear_computer INTEGER, "
//    "gear_jacket INTEGER, "
//    "FOREIGN KEY(memberAddressId) REFERENCES %2(id), "
//    "FOREIGN KEY(diverLevelId) REFERENCES %3(id))";
//    err = QSqlQuery{queryStr.arg(table_divers,table_diversAddresses,table_diverLevel),db}.lastError();
//    if(err.type() != QSqlError::ErrorType::NoError)
//    {
//        QString errStr{QString{"%0 : SQL error : %1"}.arg(__func__,err.text())};
//        qCritical() << errStr;
//        return false;
//    }



//    // Diving sites

//    queryStr = "CREATE TABLE IF NOT EXISTS %1(id INTEGER PRIMARY KEY AUTOINCREMENT, name TEXT NOT NULL)";
//    err = QSqlQuery{queryStr.arg(table_divingSites),db}.lastError();
//    if(err.type() != QSqlError::ErrorType::NoError)
//    {
//        QString errStr{QString{"%0 : SQL error : %1"}.arg(__func__,err.text())};
//        qCritical() << errStr;
//        return false;
//    }

//    if(storeInDB(data::DivingSite{-1,"Les deux frères"},db,global::table_divingSites) < 0)
//        return false;

//    if(storeInDB(data::DivingSite{-1,"La sèche du pêcheur"},db,global::table_divingSites) < 0)
//        return false;

//    if(storeInDB(data::DivingSite{-1,"L'arroyo"},db,global::table_divingSites) < 0)
//        return false;

//    if(storeInDB(data::DivingSite{-1,"Le tromblon"},db,global::table_divingSites) < 0)
//        return false;

//    if(storeInDB(data::DivingSite{-1,"La vallée aux gorgones"},db,global::table_divingSites) < 0)
//        return false;
//    //----
//    if(storeInDB(data::DivingSite{-1,"La sèche de St-Elme"},db,global::table_divingSites) < 0)
//        return false;

//    if(storeInDB(data::DivingSite{-1,"La Pierre à Pierre"},db,global::table_divingSites) < 0)
//        return false;

//    if(storeInDB(data::DivingSite{-1,"La Corée"},db,global::table_divingSites) < 0)
//        return false;

//    if(storeInDB(data::DivingSite{-1,"La roche à l'ancre"},db,global::table_divingSites) < 0)
//        return false;

//    if(storeInDB(data::DivingSite{-1,"Le Dornier"},db,global::table_divingSites) < 0)
//        return false;

//    if(storeInDB(data::DivingSite{-1,"La cabine du Dornier"},db,global::table_divingSites) < 0)
//        return false;

//    if(storeInDB(data::DivingSite{-1,"Le SMB2"},db,global::table_divingSites) < 0)
//        return false;

//    if(storeInDB(data::DivingSite{-1,"Le marauder"},db,global::table_divingSites) < 0)
//        return false;


//    // Dives

//    queryStr = "CREATE TABLE IF NOT EXISTS %1(id INTEGER PRIMARY KEY AUTOINCREMENT,"
//               "date TEXT,"
//               "time TEXT,"
//               "diveSiteId INTEGER)";
//    err = QSqlQuery{queryStr.arg(table_dives),db}.lastError();
//    if(err.type() != QSqlError::ErrorType::NoError)
//    {
//        QString errStr{QString{"Dives : SQL error : %1"}.arg(err.text())};
//        qCritical() << errStr;
//        return false;
//    }


//    // DivesMembers

//    queryStr = "CREATE TABLE IF NOT EXISTS %1(diveId INTEGER, diverId TEXT, diveType TEXT,"
//               "PRIMARY KEY(diveId, diverId),FOREIGN KEY(diveId) REFERENCES %2(id),FOREIGN KEY(diverId) REFERENCES %3(id))";
//    err = QSqlQuery{queryStr.arg(table_divesMembers,table_dives,table_divers),db}.lastError();
//    if(err.type() != QSqlError::ErrorType::NoError)
//    {
//        QString errStr{QString{"DivesMembers : SQL error : %1"}.arg(err.text())};
//        qCritical() << errStr;
//        return false;
//    }


//    return true;
//}

//bool initDB(QSqlDatabase db)
//{
//    using namespace global;

//    QString queryStr = "DROP TABLE IF EXISTS %1";
//    auto err = QSqlQuery{queryStr.arg(table_diverLevel),db}.lastError();
//    if(err.type() != QSqlError::ErrorType::NoError)
//    {
//        QString errStr{QString{"SQL error : %1"}.arg(err.text())};
//        qCritical() << errStr;
//        return false;
//    }



//    //table divers addresses
//    queryStr = "DROP TABLE IF EXISTS %1";
//    err = QSqlQuery{queryStr.arg(table_diversAddresses),db}.lastError();
//    if(err.type() != QSqlError::ErrorType::NoError)
//    {
//        QString errStr{QString{"SQL error : %1"}.arg(err.text())};
//        qCritical() << errStr;
//        return false;
//    }

//    queryStr = "DROP TABLE IF EXISTS %1";
//    err = QSqlQuery{queryStr.arg(table_divers),db}.lastError();
//    if(err.type() != QSqlError::ErrorType::NoError)
//    {
//        QString errStr{QString{"SQL error : %1"}.arg(err.text())};
//        qCritical() << errStr;
//        return false;
//    }

//    queryStr = "DROP TABLE IF EXISTS %1";
//    err = QSqlQuery{queryStr.arg(table_divingSites),db}.lastError();
//    if(err.type() != QSqlError::ErrorType::NoError)
//    {
//        QString errStr{QString{"SQL error : %1"}.arg(err.text())};
//        qCritical() << errStr;
//        return false;
//    }

//    queryStr = "DROP TABLE IF EXISTS %1";
//    err = QSqlQuery{queryStr.arg(table_dives),db}.lastError();
//    if(err.type() != QSqlError::ErrorType::NoError)
//    {
//        QString errStr{QString{"SQL error : %1"}.arg(err.text())};
//        qCritical() << errStr;
//        return false;
//    }

//    queryStr = "DROP TABLE IF EXISTS %1";
//    err = QSqlQuery{queryStr.arg(table_divesMembers),db}.lastError();
//    if(err.type() != QSqlError::ErrorType::NoError)
//    {
//        QString errStr{QString{"SQL error : %1"}.arg(err.text())};
//        qCritical() << errStr;
//        return false;
//    }

//    createDB(db);


////    auto tempAdd{data::Address{-1,"15 rue de Sauvigney","70000","Pusy et Épenoux"}};
////    tempAdd.id = addToDB(tempAdd,db,table_diversAddresses);
////    qDebug() << tempAdd;

////    tempAdd = {-1,"1360 Route de Sainte-Annce, Résidence ARPEJ, Log.316","29280","Plouzané"};
////    tempAdd.id = addToDB(tempAdd,db,table_diversAddresses);
////    qDebug() << tempAdd;


//    //table members


//    /*if(!data::addToDB({"Maaurice","SOL","momo@fr.fr",2,true,84},db,table_members))
//    {
//        return false;
//    }*/

//    data::Diver tempDiver{-1,"Mathieu","CHARS",
//                          QDate::fromString("1998-10-28",global::format_date),
//                          "mathieu@free.fr","0136984125",
//                          {-1,"15 rue des miolis","14000","Juin"},
//                          "AE-258-36",
//                          QDate::fromString("2021-08-01",global::format_date),
//                          8,//diverLevelId
//                          false,//Member ?
//                          QDate(2020,01,01),//registration date
//                          2,//paidDives
//                          true,//
//                          false,//
//                          true,//
//                          false};//
//    data::Diver tempDiver2{-1,"Bidule","TRUX",QDate::fromString("1975-01-14",global::format_date),"trux@free.fr","0136984125",{-1,"12 rue des miolis","14000","Juin"},"AE-258-54",QDate::fromString("2021-08-01",global::format_date),3,false,QDate(2020,01,01),10,1,0,1,0};
//    data::Diver tempDiver3{-1,"Nouveau","PLONGEUR",QDate::fromString("1975-01-14",global::format_date),"nouveau@free.fr","0136984125",{-1,"1360 Route","14000","Juillet"},"AE-258-54",QDate::fromString("2021-08-01",global::format_date),1,false,QDate(2020,01,01),20,1,0,1,0};
//    data::Diver tempDiver4{-1,"Machin","TEST",QDate::fromString("1975-01-14",global::format_date),"test@free.fr","0136984125",{-1,"23 route de Sainte Anne","24200","Août"},"AE-258-54",QDate::fromString("2021-08-01",global::format_date),5,false,QDate(2020,01,01),3,1,0,1,0};

//    if(!storeInDB(tempDiver,db,table_divers))
//        return false;

//    if(!storeInDB(tempDiver2,db,table_divers))
//        return false;

//    if(!storeInDB(tempDiver3,db,table_divers))
//        return false;

//    if(!storeInDB(tempDiver4,db,table_divers))
//        return false;


//    //table Dives

//    data::Dive tempDive{-1,QDate::currentDate(),QTime::currentTime(),2,{{1,1,data::DiveType::exploration},{1,2,data::DiveType::technical}}};
//    data::Dive tempDive2{-1,QDate::currentDate().addDays(-1),QTime::currentTime(),1,{{2,1,data::DiveType::technical}}};
////    data::Dive tempDive3{-1,QDate::currentDate().addDays(1),1,{}};


//    auto firstId{storeInDB(tempDive,db,global::table_dives,global::table_divers,global::table_divesMembers)};
//    auto secondId{storeInDB(tempDive2,db,global::table_dives,global::table_divers,global::table_divesMembers)};
////    auto thirdId{data::addToDB(tempDive3,db,table_dives)};
//    if(firstId < 0)
//    {
//        return false;
//    }
//    if(secondId < 0)
//    {
//        return false;
//    }
////    if(thirdId < 0)
////    {
////        return false;
////    }

////    QSqlQuery loginQuery{"SELECT id FROM Users"/*+" WHERE id="+tempLogin*/,db};
////    loginQuery.exec();
////    while(loginQuery.next())
////    {
////        qDebug() << loginQuery.value(0);
////    }

//    return true;
//}

//QStringList getDiverLevels(QSqlDatabase db)
//{
//    auto rawVal{querySelect(db,QString{"SELECT level FROM %0"},{global::table_diverLevel},{})};
//    QStringList out{};

//    for(const auto& e : rawVal)
//    {
//        out += e.at(0).toString();
//    }
//    return out;
//}

//QStringList getDiveSites(QSqlDatabase db)
//{
//    auto rawVal{querySelect(db,QString{"SELECT name FROM %0"},{global::table_divingSites},{})};
//    QStringList out{};

//    for(const auto& e : rawVal)
//    {
//        out += e.at(0).toString();
//    }
//    return out;
//}

//}//namespace db
