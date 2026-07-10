// test.cpp : Defines the entry point for the console application.
//

#include "pch.h"
#include "app/pa/PAAgent/src/PasHelpers.h"
#include "app/pa/PAAgent/src/PasTableManager.h"
#include "app/pa/PAAgent/src/MasterPasTableManager.h"
#include "core/utilities/src/DebugUtilEx.h"
#include "core/utilities/src/RunParams.h"
#include "core/utilities/src/MakeNameValuePairsString.h"
#include "core/utilities/src/stdutil/strings.h"
#include "core/utilities/src/StdUtil.h"
#include <boost/test/unit_test.hpp>

using namespace std::literals;
using namespace TA_Base_Core;
using namespace TA_IRS_App;

BOOST_AUTO_TEST_SUITE(PAAGENT)
BOOST_AUTO_TEST_SUITE(PasHelperTest)

BOOST_AUTO_TEST_CASE(test_getBit)
{
    {
        std::uint8_t x = 0x35;
        BOOST_TEST(PasHelpers::getBit(&x, 0) == true);
        BOOST_TEST(PasHelpers::getBit(&x, 1) == false);
        BOOST_TEST(PasHelpers::getBit(&x, 2) == true);
        BOOST_TEST(PasHelpers::getBit(&x, 3) == false);
        BOOST_TEST(PasHelpers::getBit(&x, 4) == true);
        BOOST_TEST(PasHelpers::getBit(&x, 5) == true);
        BOOST_TEST(PasHelpers::getBit(&x, 6) == false);
        BOOST_TEST(PasHelpers::getBit(&x, 7) == false);
    }
}

BOOST_AUTO_TEST_CASE(test_setBit)
{
    {
        std::uint8_t x = 0;
        PasHelpers::setBit(&x, 0, true);
        //PasHelpers::setBit(&x, 0, true);
        LOG_INFO("%s", st::to_hex_string(x));
        BOOST_TEST(x == 1);

        PasHelpers::setBit(&x, 1, true);
        LOG_INFO("%s", st::to_hex_string(x));
        BOOST_TEST(x == 3);

        PasHelpers::setBit(&x, 0, false);
        LOG_INFO("%s", st::to_hex_string(x));
        BOOST_TEST(x == 2);
    }
}

BOOST_AUTO_TEST_CASE(test_setBit_false)
{
    {
        std::uint8_t x = 0xFF;
        PasHelpers::setBit(&x, 0, false);
        LOG_INFO("%s", st::to_hex_string(x));
        BOOST_TEST(x == 0xFE);

        PasHelpers::setBit(&x, 7, false);
        BOOST_TEST(x == 0x7E);
    }
}

#if 0
BOOST_AUTO_TEST_CASE(test)
{
    {
        RunParams::getInstance().set(RPARAM_DBCONNECTIONFILE, R"(X:\LocalTest\J155_TIP\ConfigFiles\CSV\OCCConnectionStrings.csv)");
        // RunParams::getInstance().(RPARAM_DBFOLDERPATH, R"(X:\LocalTest\J155_TIP\Database)");
        //RunParams::getInstance().("DbConnectionDefaultMaxTry", 1);
        //RunParams::getInstance().("DbConnectionDefaultTimeout", 1);
        RunParams::getInstance().set(RPARAM_LOCATIONKEY, "1");
        RunParams::getInstance().set(RPARAM_DBPREFIX + "ta_occ@localhost"s, RPARAM_DBONLINE);
        RunParams::getInstance().set(RPARAM_DBPREFIX + "ta_occ@localhost"s, RPARAM_DBONLINE);
        RunParams::getInstance().set(RPARAM_DBPREFIX + "J155_OCC"s, RPARAM_DBONLINE);
        RunParams::getInstance().set(RPARAM_DBPREFIX + "j155_ta_occ@127.0.0.1"s, RPARAM_DBONLINE);
    }

    //auto mgr = std::make_shared<MasterPasTableManager>(1);
    //auto t200 = new Table200{0, 0, 0};
    //TA_IRS_Bus::Scheduler scheduler;
    //auto rt200 = std::make_shared<ReadTable200>(*t200, scheduler, scheduler, *mgr);
    //auto name = st::
}
#endif

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
