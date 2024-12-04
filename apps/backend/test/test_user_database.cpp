#include <boost/test/unit_test.hpp>
#include <hiddenframe_headers.h>
#include <sqlite/sqlite3.h>
#include <iostream>
#include <stdexcept>

BOOST_AUTO_TEST_SUITE(UserDatabaseTestSuite)

BOOST_AUTO_TEST_CASE(TestCreateAndAuthenticateUser)
{
    try
    {

        createNewAdmin("amitoj", "meow");
        BOOST_CHECK_THROW(createNewAdmin("amitoj", "notmeow"), std::runtime_error);

        // Test authentications
        bool auth1 = authenticateUser("NotPat", "meow");
        bool auth2 = authenticateUser("amitoj", "me0w");
        bool auth3 = authenticateUser("amitoj", "meow");

        BOOST_CHECK(!auth1);
        BOOST_CHECK(!auth2);
        BOOST_CHECK(auth3);

        if (!auth1 && !auth2 && auth3)
        {
            BOOST_TEST_MESSAGE("Authorized a user correctly!");
        }
        else
        {
            BOOST_FAIL("Authentication was not OK! Test of UAC FAILED");
        }

        int64_t id1 = createInvite("amitoj",1);
        int64_t id2 = createInvite("amitoj",2);
        int64_t id3 = createInvite("amitoj",3);
        createNewUser("1", "p1", id1);
        createNewUser("2", "p2", id2);
        createNewUser("3", "p3", id3);
        BOOST_CHECK_THROW(createNewUser("4", "p4", 0000), std::runtime_error);
        BOOST_TEST_MESSAGE("Created new users successfully!");
    }
    catch (const std::runtime_error &e)
    {
        BOOST_FAIL(e.what());
    }
}

BOOST_AUTO_TEST_SUITE_END()