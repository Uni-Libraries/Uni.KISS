// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText : 2022-2024 Uni-Libraries contributors

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

//
// Includes
//

// stdlib
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>



//
// Defines
//

/** Frame delimiter. */
#define UNI_KISS_FEND  (0xC0U)

/** Escape byte. */
#define UNI_KISS_FESC  (0xDBU)

/** Escaped frame delimiter (follows FESC). */
#define UNI_KISS_TFEND (0xDCU)

/** Escaped escape byte (follows FESC). */
#define UNI_KISS_TFESC (0xDDU)

/** Largest encoded frame for a payload of @p payload_size bytes: FEND, type, every byte escaped, FEND. */
#define UNI_KISS_ENCODED_SIZE_MAX(payload_size) (2U * (size_t)(payload_size) + 3U)



//
// Functions
//

/**
 * Decode one complete KISS frame `FEND type payload FEND`.
 *
 * The frame must be accepted by uni_kiss_valid(). The type byte (`src[1]`, port and command) is not
 * interpreted: callers that only want data frames for port 0 must check `src[1] == 0x00` themselves.
 * Leading back-to-back FENDs must be stripped by the caller (they belong to the byte-stream framer).
 *
 * @param src      encoded frame
 * @param src_size encoded frame size in bytes
 * @param dst      destination buffer for the unescaped payload
 * @param dst_size destination capacity in bytes; `src_size - 3` always suffices
 * @return payload size, or 0 when the frame is invalid, does not fit into @p dst or has an empty payload
 */
size_t uni_kiss_decode(const uint8_t *src, size_t src_size, uint8_t *dst, size_t dst_size);

/**
 * Encode @p src as a KISS data frame for port 0 (`FEND 0x00 escaped-payload FEND`).
 *
 * @param src      payload, may be NULL when @p src_size is 0
 * @param src_size payload size in bytes
 * @param dst      destination buffer
 * @param dst_size destination capacity in bytes; UNI_KISS_ENCODED_SIZE_MAX(src_size) always suffices
 * @return encoded frame size, or 0 when an argument is invalid or the frame does not fit into @p dst
 *         (nothing is ever written past `dst + dst_size`)
 */
size_t uni_kiss_encode(const uint8_t *src, size_t src_size, uint8_t *dst, size_t dst_size);

/**
 * Check that @p src is one well-formed KISS frame: at least 3 bytes, FEND first and last, no FEND in
 * between, a type byte that is neither FEND nor FESC, and every FESC followed by TFEND or TFESC.
 */
bool uni_kiss_valid(const uint8_t *src, size_t src_size);

#ifdef __cplusplus
}
#endif
