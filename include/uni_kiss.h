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
#include <stddef.h>
#include <stdint.h>



//
// Functions
//

size_t uni_kiss_decode(const uint8_t *src, size_t src_size, uint8_t *dst, size_t dst_size);

size_t uni_kiss_encode(const uint8_t *src, size_t src_size, uint8_t *dst, size_t dst_size);

bool uni_kiss_valid(const uint8_t *src, size_t src_size);

#ifdef __cplusplus
}
#endif
