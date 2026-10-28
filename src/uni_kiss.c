// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText : 2022-2024 Uni-Libraries contributors

//
// Includes
//

// Uni.KISS
#include "uni_kiss.h"



//
// Functions
//

size_t uni_kiss_decode(const uint8_t *src, size_t src_size, uint8_t *dst, size_t dst_size) {
    size_t result_size = 0U;

    if (dst == NULL || !uni_kiss_valid(src, src_size)) {
        return 0U;
    }

    // payload lies between the type byte and the trailing FEND; uni_kiss_valid() guarantees that
    // every FESC in that range is followed by TFEND or TFESC before the trailing FEND
    for (size_t i = 2U; i < src_size - 1U; i++) {
        uint8_t byte = src[i];
        if (byte == UNI_KISS_FESC) {
            i++;
            byte = (src[i] == UNI_KISS_TFEND) ? UNI_KISS_FEND : UNI_KISS_FESC;
        }
        if (result_size == dst_size) {
            return 0U;
        }
        dst[result_size++] = byte;
    }

    return result_size;
}


size_t uni_kiss_encode(const uint8_t *src, size_t src_size, uint8_t *dst, size_t dst_size) {
    size_t result_size = 0U;

    if (dst == NULL || (src == NULL && src_size != 0U) || dst_size < 3U) {
        return 0U;
    }

    dst[result_size++] = UNI_KISS_FEND;
    dst[result_size++] = 0x00U; // port 0, data frame

    for (size_t i = 0U; i < src_size; i++) {
        const uint8_t byte = src[i];
        const bool escaped = (byte == UNI_KISS_FEND || byte == UNI_KISS_FESC);
        const size_t needed = escaped ? 2U : 1U;

        // keep one byte for the trailing FEND
        if (dst_size - result_size < needed + 1U) {
            return 0U;
        }

        if (escaped) {
            dst[result_size++] = UNI_KISS_FESC;
            dst[result_size++] = (byte == UNI_KISS_FEND) ? UNI_KISS_TFEND : UNI_KISS_TFESC;
        } else {
            dst[result_size++] = byte;
        }
    }

    dst[result_size++] = UNI_KISS_FEND;
    return result_size;
}


bool uni_kiss_valid(const uint8_t *src, size_t src_size) {
    if (src == NULL || src_size < 3U) {
        return false;
    }
    if (src[0] != UNI_KISS_FEND || src[src_size - 1U] != UNI_KISS_FEND) {
        return false;
    }

    // the type byte is not escaped by this implementation
    if (src[1] == UNI_KISS_FEND || src[1] == UNI_KISS_FESC) {
        return false;
    }

    for (size_t i = 2U; i < src_size - 1U; i++) {
        if (src[i] == UNI_KISS_FEND) {
            return false;
        }
        if (src[i] == UNI_KISS_FESC) {
            i++;
            if (i >= src_size - 1U || (src[i] != UNI_KISS_TFEND && src[i] != UNI_KISS_TFESC)) {
                return false;
            }
        }
    }

    return true;
}
