//
// Includes
//

// Uni.KISS
#include "uni_kiss.h"



//
// Defines
//

size_t uni_kiss_decode(const uint8_t *src, size_t src_size, uint8_t *dst, size_t dst_size) {
    size_t result_size = 0;

    //check size
    if (src_size <= 3) {
        return 0;
    }
    if (dst_size + 3 < src_size) {
        return 0;
    }

    //check trailer
    if (src[0] != 0xC0 && src[src_size - 1] != 0xC0) {
        return 0;
    }

    size_t i = 2;
    while (i < src_size - 1) {
        uint8_t b_1 = src[i];
        uint8_t b_2 = src[i + 1];

        if (b_1 == 0xDB && b_2 == 0xDC) {
            dst[result_size++] = 0xC0;
            i += 2;
        } else if (b_1 == 0xDB && b_2 == 0xDD) {
            dst[result_size++] = 0xDB;
            i += 2;
        } else {
            dst[result_size++] = b_1;
            i += 1;
        }
    }
    return result_size;
}


size_t uni_kiss_encode(const uint8_t *src, size_t src_size, uint8_t *dst, size_t dst_size) {
    size_t result_size = 0;

    if (dst_size < 3) {
        return 0;
    }
    dst[0] = 0xC0;
    dst[1] = 0x00;

    result_size = 2;
    for (size_t i = 0; i < src_size; i++) {
        if (src[i] == 0xC0) {
            dst[result_size] = 0xDB;
            dst[result_size + 1] = 0xDC;
            result_size++;
        } else if (src[i] == 0xDB) {
            dst[result_size] = 0xDB;
            dst[result_size + 1] = 0xDD;
            result_size++;
        } else {
            dst[result_size] = src[i];
        }
        result_size++;
        if (dst_size == result_size) {
            return 0;
        }
    }
    dst[result_size] = 0xC0;
    result_size++;

    return result_size;
}


bool uni_kiss_valid(const uint8_t *src, size_t src_size) {
    bool result = false;

    if (src_size < 3) {
        return false;
    }
    if (0xC0 != src[0] || 0xC0 != src[src_size - 1]) {
        return false;
    }

    for (size_t i = 2; i < src_size - 1; i++) {
        if (0xC0 == src[i]) {
            return false;
        }
    }

    return true;
}
