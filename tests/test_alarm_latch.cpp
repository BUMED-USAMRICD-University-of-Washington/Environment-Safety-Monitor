#define CATCH_CONFIG_MAIN  // Tells Catch2 to provide a main() execution wrapper
#include <catch2/catch_all.hpp>
#include "alarm_latch.h"

TEST_CASE("AlarmLatch - Transient Noise Elimination Checks", "[safety_latch]") {
    // Instantiate a 3000ms (3 second) delay latch simulating the O2 loop
    AlarmLatch o2Latch(3000);
    uint32_t timestamp = 1000; // Mock hardware clock tick starting at 1000ms

    SECTION("A brief, transient violation must not trigger a critical alarm") {
        // First violation packet arrives
        REQUIRE(o2Latch.update(true, timestamp) == false);
        REQUIRE(o2Latch.isCountingDown() == true);

        // Time advances by 1.5 seconds (under the 3-second limit)
        timestamp += 1500;
        REQUIRE(o2Latch.update(true, timestamp) == false);

        // Hazard clears completely (transient noise clears)
        timestamp += 100;
        REQUIRE(o2Latch.update(false, timestamp) == false);
        REQUIRE(o2Latch.isCountingDown() == false);
    }

    SECTION("A continuous violation past the time gate must latch a critical evacuation alert") {
        // Hazard begins
        o2Latch.update(true, timestamp);

        // Progressively step forward by 1 second intervals
        timestamp += 1000; // Total 1.0s elapsed
        REQUIRE(o2Latch.update(true, timestamp) == false);

        timestamp += 1000; // Total 2.0s elapsed
        REQUIRE(o2Latch.update(true, timestamp) == false);

        timestamp += 1100; // Total 3.1s elapsed (Exceeds the 3000ms threshold window)
        REQUIRE(o2Latch.update(true, timestamp) == true); // MUST return true (Evacuate Building)
        REQUIRE(o2Latch.isCountingDown() == false);
    }

    SECTION("A single safe sample inside the countdown window must instantly reset the safety timer") {
        o2Latch.update(true, timestamp); // Threat starts at t=1000ms

        timestamp += 2900; // 2.9 seconds pass (Close to breaching the gate)
        REQUIRE(o2Latch.update(true, timestamp) == false);

        timestamp += 100;  // Normal air flows briefly past the probe at 3.0 seconds
        REQUIRE(o2Latch.update(false, timestamp) == false); // Reset triggered!

        timestamp += 500;  // Threat re-emerges 0.5 seconds later
        REQUIRE(o2Latch.update(true, timestamp) == false); // Timer must start completely over from 0
    }
}

TEST_CASE("AlarmLatch - Instant Spill Configuration Checks", "[safety_latch]") {
    // Instantiate a 0ms delay latch simulating the cryogenic floor sensor array
    AlarmLatch cryoLatch(0);
    uint32_t timestamp = 5000;

    SECTION("A cryogenic floor leak must bypass grace periods and trip immediately") {
        // Liquid nitrogen drops onto the floor probe
        bool alarmActive = cryoLatch.update(true, timestamp);
        
        // Assert that the alarm triggers instantly with zero delays
        REQUIRE(alarmActive == true);
    }
}
