// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText : 2022-2024 Uni-Libraries contributors

//
// Includes
//

// stdlib
#include <cstring>

// catch
#include <catch2/catch_test_macros.hpp>

// Uni.AX25
#include "uni_kiss.h"



//
// tests
//

TEST_CASE("kiss_decode", "[kiss]") {
    SECTION("nullptr") {
        REQUIRE_FALSE(uni_kiss_decode(nullptr,0,nullptr,0));
    }
}


TEST_CASE("kiss_encode", "[kiss]") {
    SECTION("nullptr") {
        REQUIRE_FALSE(uni_kiss_encode(nullptr,0,nullptr,0));
    }
}


TEST_CASE("kiss_valid", "[kiss]") {
    SECTION("nullptr") {
        REQUIRE_FALSE(uni_kiss_valid(nullptr,0));
        REQUIRE_FALSE(uni_kiss_valid(nullptr,1));
    }
}
