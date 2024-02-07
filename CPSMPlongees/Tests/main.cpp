#include <gtest/gtest.h>

#include <CPSMDatabase.hpp>
#include <CPSMGlobals.hpp>

int main(int argc, char *argv[])
{
    ::testing::InitGoogleTest(&argc, argv);

    cpsm::db::InitDB<true,true>("testDB.db");

    return RUN_ALL_TESTS();
}
