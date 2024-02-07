#include <gtest/gtest.h>

#include <CPSMDatabase.hpp>
#include <CPSMGlobals.hpp>

QSqlDatabase dbt{QSqlDatabase::database()};

bool FillDBWithTests(){
    if(!::db::ExecScript("../Libs/CPSMDBTools/DBScripts/FillTestDB.sql",&dbt)){
        qCritical() << "Failed to execute fill test DB script";
        return false;
    }else{
        qInfo() << "Successfully executed fill test DB script";
    }
    return true;
}

bool ClearDB(){
    if(!::db::ExecScript(":/DBScripts/ClearDB.sql",&dbt)){
        qCritical() << "Failed to execute clear test DB script";
        return false;
    }else{
        qInfo() << "Successfully executed clear test DB script";
    }
    return true;
}

TEST(CPSMTests, DBTest)
{
    ASSERT_TRUE(ClearDB());
    ASSERT_TRUE(FillDBWithTests());



    EXPECT_EQ(1, 1);
}
