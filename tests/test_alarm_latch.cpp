define CATCH_CONFIG_MAIN
include <catch2/catch_all.hpp>
include "alarm_latch.h"

TEST_CASE("AlarmLatch - Transient Noise Elimination Checks", "[safety_latch]") {
    AlarmLatch o2Latch(3000);
    uint32_t timestamp = 1000;

    SECTION("A brief, transient violation must not trigger a critical alarm") {
        REQUIRE(o2Latch.update(true, timestamp) == false);
        REQUIRE(o2Latch.isCountingDown() == true);
        timestamp += 1500;
        REQUIRE(o2Latch.update(true, timestamp) == false);
        timestamp += 100;
        REQUIRE(o2Latch.update(false, timestamp) == false);
        REQUIRE(o2Latch.isCountingDown() == false);
    }

    SECTION("A continuous violation past the time gate must latch a critical evacuation alert") {
        o2Latch.update(true, timestamp);
        timestamp += 1000;
        REQUIRE(o2Latch.update(true, timestamp) == false);
        timestamp += 1000;
        REQUIRE(o2Latch.update(true, timestamp) == false);
        timestamp += 1100;
        REQUIRE(o2Latch.update(true, timestamp) == true);
        REQUIRE(o2Latch.isCountingDown() == false);
    }

    SECTION("A single safe sample inside the countdown window must instantly reset the safety timer") {
        o2Latch.update(true, timestamp);
        timestamp += 2900;
        REQUIRE(o2Latch.update(true, timestamp) == false);
        timestamp += 100;
        REQUIRE(o2Latch.update(false, timestamp) == false);
        timestamp += 500;
        REQUIRE(o2Latch.update(true, timestamp) == false);
    }
}

TEST_CASE("AlarmLatch - Instant Spill Configuration Checks", "[safety_latch]") {
    AlarmLatch cryoLatch(0);
    uint32_t timestamp = 5000;

    SECTION("A cryogenic floor leak must bypass grace periods and trip immediately") {
        bool alarmActive = cryoLatch.update(true, timestamp);
        REQUIRE(alarmActive == true);
    }
}
