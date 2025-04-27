#include <stdint.h>

uint32_t mpc_divide_unsigned(uint32_t numerator, uint32_t denominator) {
    uint32_t result = 0;
    uint32_t remainder = numerator;

    // Define step as an inline function
    #define STEP(i, remainder, result)                        \
        do {                                                   \
            uint32_t shifted_denom = denominator << (i);       \
            int32_t diff = (int32_t)(remainder - shifted_denom); \
            uint32_t ge = ((diff >> 31) & 1) ^ 1;              \
            remainder = remainder - (shifted_denom * ge);      \
            result = result | (ge << (i));                     \
        } while(0)

    // Unroll all 32 steps manually
    STEP(31, remainder, result);
    STEP(30, remainder, result);
    STEP(29, remainder, result);
    STEP(28, remainder, result);
    STEP(27, remainder, result);
    STEP(26, remainder, result);
    STEP(25, remainder, result);
    STEP(24, remainder, result);
    STEP(23, remainder, result);
    STEP(22, remainder, result);
    STEP(21, remainder, result);
    STEP(20, remainder, result);
    STEP(19, remainder, result);
    STEP(18, remainder, result);
    STEP(17, remainder, result);
    STEP(16, remainder, result);
    STEP(15, remainder, result);
    STEP(14, remainder, result);
    STEP(13, remainder, result);
    STEP(12, remainder, result);
    STEP(11, remainder, result);
    STEP(10, remainder, result);
    STEP(9, remainder, result);
    STEP(8, remainder, result);
    STEP(7, remainder, result);
    STEP(6, remainder, result);
    STEP(5, remainder, result);
    STEP(4, remainder, result);
    STEP(3, remainder, result);
    STEP(2, remainder, result);
    STEP(1, remainder, result);
    STEP(0, remainder, result);

    #undef STEP

    return result;
}
