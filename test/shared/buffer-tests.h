/*
* Copyright 2025-6 The pio-i2s Contributors.
* Licensed under the BSD-3 License.
*/

#ifndef TEST_BUFFER_TESTS_H
#define TEST_BUFFER_TESTS_H

#include <stdint.h>
#include <string.h>
#include "unity.h"
#include <pio-i2s-config.h>
#include "pio-i2s.h"
#include "test-support.h"

static void testSetDMAReading(struct PioI2S* self, const int32_t* address) {
    dma_hw->ch[self->dataChannel].read_addr = (uintptr_t) address;
}

static void testNextOutputBufferIsTheHalfNotBeingRead(void) {
    struct PioI2S self = {0};
    struct PioI2S_Config config = testDefaultConfig();
    int32_t output[TEST_DOUBLE_BUFFER_SIZE] = {0};
    int32_t* first = &output[0];
    int32_t* second = &output[TEST_STEREO_BLOCK_SIZE];

    self.config = &config;
    self.dataChannel = 2;
    self.outputDoubleBuffer = output;
    self.stereoBlockSize = TEST_STEREO_BLOCK_SIZE;
    self.bufferPointers[0] = first;
    self.bufferPointers[1] = second;

    // Reading anywhere in the first half: fill the second.
    testSetDMAReading(&self, first);
    TEST_ASSERT_EQUAL_PTR(second, PioI2S_nextOutputBuffer(&self));
    testSetDMAReading(&self, first + TEST_STEREO_BLOCK_SIZE - 1);
    TEST_ASSERT_EQUAL_PTR(second, PioI2S_nextOutputBuffer(&self));

    // At the end of the first half, which is the start of the second: the
    // channel has moved on, so fill the first.
    testSetDMAReading(&self, second);
    TEST_ASSERT_EQUAL_PTR(first, PioI2S_nextOutputBuffer(&self));
    testSetDMAReading(&self, second + TEST_STEREO_BLOCK_SIZE - 1);
    TEST_ASSERT_EQUAL_PTR(first, PioI2S_nextOutputBuffer(&self));

    // Just past the second half, about to wrap: fill the second.
    testSetDMAReading(&self, second + TEST_STEREO_BLOCK_SIZE);
    TEST_ASSERT_EQUAL_PTR(second, PioI2S_nextOutputBuffer(&self));
}

static void testNextOutputBufferSurvivesAMissedInterrupt(void) {
    struct PioI2S self = {0};
    struct PioI2S_Config config = testDefaultConfig();
    int32_t output[TEST_DOUBLE_BUFFER_SIZE] = {0};
    int32_t* first = &output[0];
    int32_t* second = &output[TEST_STEREO_BLOCK_SIZE];

    self.config = &config;
    self.dataChannel = 2;
    self.outputDoubleBuffer = output;
    self.stereoBlockSize = TEST_STEREO_BLOCK_SIZE;
    self.bufferPointers[0] = first;
    self.bufferPointers[1] = second;

    // Two handler runs while the channel is still in the second half - which
    // is what a coalesced interrupt looks like from inside the handler. A
    // selection that alternated would hand out the second half the second
    // time, and every block after that would be written while it played.
    testSetDMAReading(&self, second + 3);
    TEST_ASSERT_EQUAL_PTR(first, PioI2S_nextOutputBuffer(&self));
    testSetDMAReading(&self, second + TEST_STEREO_BLOCK_SIZE / 2);
    TEST_ASSERT_EQUAL_PTR(first, PioI2S_nextOutputBuffer(&self));
}

#if PioI2S_ZERO_ON_UNDERRUN
static void testNextOutputBufferZeroesOnUnderrun(void) {
    struct PioI2S self = {0};
    struct PioI2S_Config config = testDefaultConfig();
    int32_t output[TEST_DOUBLE_BUFFER_SIZE];
    int32_t* bufferToFill;
    size_t i;

    memset(output, 0xAB, sizeof(output));

    self.config = &config;
    self.dataChannel = 2;
    self.outputDoubleBuffer = output;
    self.stereoBlockSize = TEST_STEREO_BLOCK_SIZE;
    self.bufferPointers[0] = &output[0];
    self.bufferPointers[1] = &output[TEST_STEREO_BLOCK_SIZE];

    // The channel is playing the second half, so the first is handed out.
    dma_hw->ch[self.dataChannel].read_addr = (uintptr_t) &output[TEST_STEREO_BLOCK_SIZE];
    bufferToFill = PioI2S_nextOutputBuffer(&self);
    TEST_ASSERT_EQUAL_PTR(&output[0], bufferToFill);

    for (i = 0; i < TEST_STEREO_BLOCK_SIZE; i++) {
        TEST_ASSERT_EQUAL_INT32(0, bufferToFill[i]);
    }

    for (i = 0; i < TEST_STEREO_BLOCK_SIZE; i++) {
        TEST_ASSERT_EQUAL_INT32(0xABABABAB, output[TEST_STEREO_BLOCK_SIZE + i]);
    }
}
#endif

static void testEndDMAInterruptHandlerIncrementsBlockCount(void) {
    struct PioI2S self = {0};
    self.dmaIRQIdx = 0;
    self.dataChannel = 2;
    self.numBlocksTransferred = 5;

    PioI2S_endDMAInterruptHandler(&self);
    TEST_ASSERT_EQUAL_UINT64(6, self.numBlocksTransferred);

    PioI2S_endDMAInterruptHandler(&self);
    TEST_ASSERT_EQUAL_UINT64(7, self.numBlocksTransferred);
}

static inline void runBufferTests(void) {
    RUN_TEST(testNextOutputBufferIsTheHalfNotBeingRead);
    RUN_TEST(testNextOutputBufferSurvivesAMissedInterrupt);
#if PioI2S_ZERO_ON_UNDERRUN
    RUN_TEST(testNextOutputBufferZeroesOnUnderrun);
#endif
    RUN_TEST(testEndDMAInterruptHandlerIncrementsBlockCount);
}

#endif // TEST_BUFFER_TESTS_H
