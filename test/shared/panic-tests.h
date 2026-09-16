/*
* Copyright 2025-6 The pio-i2s Contributors.
* Licensed under the BSD-3 License.
*/

// Host-only tests for the library's panic paths. The host mock's panic()
// longjmps back to testRunExpectingPanic; the device's real panic() halts
// the chip, so these tests never run there.

#ifndef TEST_PANIC_TESTS_H
#define TEST_PANIC_TESTS_H

#ifdef PIOI2S_TEST_HOST

#include <stdint.h>
#include "unity.h"
#include "pio-i2s.h"
#include "pio-i2s-config.h"
#include "test-support.h"

static void initWithInvalidDMAIRQ(void) {
    struct PioI2S self = {0};
    struct PioI2S_Config config = testDefaultConfig();
    int32_t output[TEST_DOUBLE_BUFFER_SIZE] = {0};
    config.dmaIRQ = 42;

    PioI2S_init(&self, &config, output, testDMAIRQHandler);
}

static void verifyDivisionBelowOne(void) {
    PicoI2S_verifyPIOClockDivision(0.5f);
}

#if PioI2S_MAX_CLOCK_ERROR_PPM < 1953
static void verifyDivisionWithExcessiveError(void) {
    // Half a step of rounding at a ratio just above 1.0 is the worst error the
    // divider can produce, about 1953 ppm. Only a tolerance tighter than that
    // can reject anything at all.
    PicoI2S_verifyPIOClockDivision(1.0f + 1.0f / 512.0f);
}
#endif

static void verifyDivisionAboveMax(void) {
    PicoI2S_verifyPIOClockDivision(256.0f);
}

static void testInitPanicsOnInvalidDMAIRQ(void) {
    char message[128];

    TEST_ASSERT_TRUE(testRunExpectingPanic(initWithInvalidDMAIRQ, message));
    TEST_ASSERT_EQUAL_STRING("Invalid DMA IRQ 42.", message);
}

static void testVerifyRejectsDivisionBelowOne(void) {
    TEST_ASSERT_TRUE(testRunExpectingPanic(verifyDivisionBelowOne, NULL));
}

static void testVerifyRejectsExcessiveError(void) {
#if PioI2S_MAX_CLOCK_ERROR_PPM < 1953
    TEST_ASSERT_TRUE(
        testRunExpectingPanic(verifyDivisionWithExcessiveError, NULL));
#else
    TEST_IGNORE_MESSAGE(
        "PioI2S_MAX_CLOCK_ERROR_PPM accepts every ratio the divider can round");
#endif
}

static void testVerifyAcceptsRepresentableFraction(void) {
    // 25 + 1/256 is exactly representable with the divider's 8 fractional
    // bits, so this must not panic.
    PicoI2S_verifyPIOClockDivision(25.0f + 1.0f / 256.0f);
}

static void testVerifyAcceptsRoundedFraction(void) {
    // 25 + 1/512 falls halfway between two representable values. The divider
    // rounds it, at a cost of 78 ppm, which is not a reason to refuse to run.
    PicoI2S_verifyPIOClockDivision(25.0f + 1.0f / 512.0f);
}

static void testVerifyAcceptsStandardAudioRates(void) {
    // The rates this library exists to produce. Every one of them needs a
    // fractional divider at 32-bit stereo; requiring an exactly representable
    // ratio rejects all of them. Ratios are sysclk / (rate * 2ch * 32bit *
    // 2instr) at 138 MHz.
    PicoI2S_verifyPIOClockDivision(138000000.0f / (11025.0f * 128.0f));
    PicoI2S_verifyPIOClockDivision(138000000.0f / (22050.0f * 128.0f));
    PicoI2S_verifyPIOClockDivision(138000000.0f / (44100.0f * 128.0f));
}

static void testVerifyRejectsDivisionAboveMax(void) {
    TEST_ASSERT_TRUE(testRunExpectingPanic(verifyDivisionAboveMax, NULL));
}

static void testVerifyAcceptsMaxDivision(void) {
    // 255 + 255/256 is the library's maximum, so this should not panic.
    PicoI2S_verifyPIOClockDivision(255.0f + 255.0f / 256.0f);
}

static inline void runPanicTests(void) {
    RUN_TEST(testInitPanicsOnInvalidDMAIRQ);
    RUN_TEST(testVerifyRejectsDivisionBelowOne);
    RUN_TEST(testVerifyRejectsExcessiveError);
    RUN_TEST(testVerifyAcceptsRepresentableFraction);
    RUN_TEST(testVerifyAcceptsRoundedFraction);
    RUN_TEST(testVerifyAcceptsStandardAudioRates);
    RUN_TEST(testVerifyRejectsDivisionAboveMax);
    RUN_TEST(testVerifyAcceptsMaxDivision);
}

#endif // PIOI2S_TEST_HOST
#endif // TEST_PANIC_TESTS_H
