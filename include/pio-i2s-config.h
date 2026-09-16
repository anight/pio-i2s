/*
* Copyright 2025-6 The pio-i2s Contributors.
* Licensed under the BSD-3 License.
*/

/*
* To override the configuration definitions within this file,
* put a directory containing your own pio-i2s-config.h file
* on the include path ahead of the pio-i2s's include directory.
*/
#ifndef PIOI2S_CONFIG_H
#define PIOI2S_CONFIG_H

/**
 * @brief Whether PioI2S_nextOutputBuffer zeroes the buffer before returning it.
 */
#ifndef PioI2S_ZERO_ON_UNDERRUN
#define PioI2S_ZERO_ON_UNDERRUN 0
#endif

/**
 * @brief The largest sample rate error, in parts per million, that
 * PicoI2S_verifyPIOClockDivision will accept from rounding the clock divider.
 *
 * The PIO divider has 8 fractional bits, so a requested ratio is rounded to the
 * nearest 1/256. The worst case that rounding can cost is half a step, which is
 * largest in relative terms at the smallest ratio - about 1953 ppm just above
 * 1.0, falling as the ratio grows. The default is therefore permissive: it
 * accepts every ratio the hardware can express. Lower it if you are clocking
 * against something that cares.
 */
#ifndef PioI2S_MAX_CLOCK_ERROR_PPM
#define PioI2S_MAX_CLOCK_ERROR_PPM 5000
#endif

#endif /* PIOI2S_CONFIG_H */
