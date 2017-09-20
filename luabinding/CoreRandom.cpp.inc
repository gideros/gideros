/*
 * CoreRandom.cpp
 *
 *  Created on: 20 sept. 2017
 *      Author: Nico
 */

#include <stdint.h>

// Define MT19937 constants (32-bit RNG)
namespace Rnd {

namespace MT19937 {

enum
{
    // Assumes W = 32 (omitting this)
    N = 624,
    M = 397,
    R = 31,
    A = 0x9908B0DF,

    F = 1812433253,

    U = 11,
    // Assumes D = 0xFFFFFFFF (omitting this)

    S = 7,
    B = 0x9D2C5680,

    T = 15,
    C = 0xEFC60000,

    L = 18,

    MASK_LOWER = (1ull << R) - 1,
    MASK_UPPER = (1ull << R)
};

static uint32_t  mt[N];
static uint16_t  index;

// Re-init with a given seed
void Initialize(const uint32_t  seed)
{
    uint32_t  i;

    mt[0] = seed;

    for ( i = 1; i < N; i++ )
    {
        mt[i] = (F * (mt[i - 1] ^ (mt[i - 1] >> 30)) + i);
    }

    index = N;
}

uint32_t GetSeed() { return mt[0]; }
static void Twist()
{
    uint32_t  i, x, xA;

    for ( i = 0; i < N; i++ )
    {
        x = (mt[i] & MASK_UPPER) + (mt[(i + 1) % N] & MASK_LOWER);

        xA = x >> 1;

        if ( x & 0x1 )
            xA ^= A;

        mt[i] = mt[(i + M) % N] ^ xA;
    }

    index = 0;
}

// Obtain a 32-bit random number
uint32_t ExtractU32()
{
    uint32_t  y;
    int       i = index;

    if ( index >= N )
    {
        Twist();
        i = index;
    }

    y = mt[i];
    index = i + 1;

    y ^= (mt[i] >> U);
    y ^= (y << S) & B;
    y ^= (y << T) & C;
    y ^= (y >> L);

    return y;
}

double ExtractDouble() {
    double v=ExtractU32();
    v/=(0x10000);
    v/=(0x10000);
    return v;
}
}

}
