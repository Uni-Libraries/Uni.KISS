// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText : 2022-2024 Uni-Libraries contributors

//
// Includes
//

// stdlib
#include <cstdint>
#include <cstring>
#include <memory>
#include <random>
#include <vector>

// catch
#include <catch2/catch_test_macros.hpp>

// Uni.KISS
#include "uni_kiss.h"



//
// helpers
//

namespace {

using Bytes = std::vector<uint8_t>;

// Exactly-sized heap buffer so that AddressSanitizer reports any write past the end.
struct ExactBuffer {
    explicit ExactBuffer(size_t size) : data(new uint8_t[size == 0U ? 1U : size]), size(size) {}
    std::unique_ptr<uint8_t[]> data;
    size_t size;
};

Bytes Encode(const Bytes &payload) {
    Bytes frame(UNI_KISS_ENCODED_SIZE_MAX(payload.size()));
    const size_t size = uni_kiss_encode(payload.data(), payload.size(), frame.data(), frame.size());
    REQUIRE(size >= 3U);
    frame.resize(size);
    return frame;
}

Bytes Decode(const Bytes &frame, size_t dst_size) {
    ExactBuffer dst(dst_size);
    const size_t size = uni_kiss_decode(frame.data(), frame.size(), dst.data.get(), dst.size);
    REQUIRE(size <= dst_size);
    return Bytes(dst.data.get(), dst.data.get() + size);
}

// Independent reference decoder: returns false for anything that is not one well-formed frame.
bool ReferenceDecode(const Bytes &frame, Bytes &payload) {
    payload.clear();
    if (frame.size() < 3U || frame.front() != 0xC0U || frame.back() != 0xC0U || frame[1] == 0xC0U || frame[1] == 0xDBU) {
        return false;
    }
    for (size_t i = 2U; i + 1U < frame.size(); i++) {
        if (frame[i] == 0xC0U) {
            return false;
        }
        if (frame[i] != 0xDBU) {
            payload.push_back(frame[i]);
            continue;
        }
        if (i + 2U >= frame.size()) {
            return false;
        }
        i++;
        if (frame[i] == 0xDCU) {
            payload.push_back(0xC0U);
        } else if (frame[i] == 0xDDU) {
            payload.push_back(0xDBU);
        } else {
            return false;
        }
    }
    return true;
}

} // namespace



//
// tests
//

TEST_CASE("kiss_decode", "[kiss]") {
    SECTION("nullptr") {
        REQUIRE_FALSE(uni_kiss_decode(nullptr, 0, nullptr, 0));
        REQUIRE_FALSE(uni_kiss_decode(nullptr, 16, nullptr, 16));
        const Bytes frame{0xC0, 0x00, 0x41, 0xC0};
        REQUIRE_FALSE(uni_kiss_decode(frame.data(), frame.size(), nullptr, 16));
    }

    SECTION("plain and escaped payload") {
        const Bytes frame{0xC0, 0x00, 0x41, 0xDB, 0xDC, 0x42, 0xDB, 0xDD, 0xC0};
        REQUIRE(Decode(frame, 16) == Bytes{0x41, 0xC0, 0x42, 0xDB});
    }

    SECTION("destination sized to the decoded payload is enough") {
        const Bytes frame{0xC0, 0x00, 0xDB, 0xDC, 0xDB, 0xDD, 0xC0};
        REQUIRE(Decode(frame, 2) == Bytes{0xC0, 0xDB});
        REQUIRE(Decode(frame, 1).empty());
    }

    SECTION("missing leading or trailing FEND is rejected") {
        // regression: the trailer check used '&&' and accepted a frame missing either delimiter
        REQUIRE(Decode(Bytes{0xC0, 0x00, 0x41, 0x42}, 16).empty());
        REQUIRE(Decode(Bytes{0x00, 0x00, 0x41, 0xC0}, 16).empty());
    }

    SECTION("invalid escape sequences are rejected") {
        REQUIRE(Decode(Bytes{0xC0, 0x00, 0xDB, 0x41, 0xC0}, 16).empty());
        REQUIRE(Decode(Bytes{0xC0, 0x00, 0x41, 0xDB, 0xC0}, 16).empty());
        REQUIRE(Decode(Bytes{0xC0, 0x00, 0xDB, 0xC0}, 16).empty());
    }

    SECTION("FEND inside the frame or as type byte is rejected") {
        REQUIRE(Decode(Bytes{0xC0, 0x00, 0x41, 0xC0, 0x42, 0xC0}, 16).empty());
        REQUIRE(Decode(Bytes{0xC0, 0xC0, 0x00, 0x41, 0xC0}, 16).empty());
    }

    SECTION("huge destination size does not overflow the size check") {
        const Bytes frame{0xC0, 0x00, 0x41, 0x42, 0xC0};
        uint8_t dst[2]{};
        REQUIRE(uni_kiss_decode(frame.data(), frame.size(), dst, SIZE_MAX - 1U) == 2U);
        REQUIRE(dst[0] == 0x41);
        REQUIRE(dst[1] == 0x42);
    }
}


TEST_CASE("kiss_encode", "[kiss]") {
    SECTION("nullptr") {
        REQUIRE_FALSE(uni_kiss_encode(nullptr, 0, nullptr, 0));
        uint8_t dst[8]{};
        REQUIRE_FALSE(uni_kiss_encode(nullptr, 1, dst, sizeof(dst)));
        REQUIRE(uni_kiss_encode(nullptr, 0, dst, sizeof(dst)) == 3U);
    }

    SECTION("escaping") {
        REQUIRE(Encode(Bytes{0x41, 0xC0, 0x42, 0xDB}) == Bytes{0xC0, 0x00, 0x41, 0xDB, 0xDC, 0x42, 0xDB, 0xDD, 0xC0});
    }

    SECTION("exact fit succeeds, one byte less fails without writing past the end") {
        const Bytes payloads[] = {{0x41, 0x42}, {0xC0, 0xDB}, {0x41, 0xC0}, {0xC0, 0x41, 0xDB, 0x42}};
        for (const auto &payload : payloads) {
            const Bytes expected = Encode(payload);
            for (size_t dst_size = 0; dst_size < expected.size(); dst_size++) {
                ExactBuffer dst(dst_size);
                REQUIRE(uni_kiss_encode(payload.data(), payload.size(), dst.data.get(), dst.size) == 0U);
            }
            ExactBuffer dst(expected.size());
            REQUIRE(uni_kiss_encode(payload.data(), payload.size(), dst.data.get(), dst.size) == expected.size());
            REQUIRE(memcmp(dst.data.get(), expected.data(), expected.size()) == 0);
        }
    }

    SECTION("escape straddling the end of the buffer") {
        // regression: a two-byte escape written at dst_size - 1 overflowed the buffer, and the
        // end-of-buffer check then never matched again
        const Bytes payload{0x41, 0xC0, 0x42, 0x43, 0x44, 0x45};
        ExactBuffer dst(4U);
        REQUIRE(uni_kiss_encode(payload.data(), payload.size(), dst.data.get(), dst.size) == 0U);
    }
}


TEST_CASE("kiss_valid", "[kiss]") {
    SECTION("nullptr") {
        REQUIRE_FALSE(uni_kiss_valid(nullptr, 0));
        REQUIRE_FALSE(uni_kiss_valid(nullptr, 1));
        REQUIRE_FALSE(uni_kiss_valid(nullptr, 16));
    }

    SECTION("frames") {
        const Bytes empty{0xC0, 0x00, 0xC0};
        const Bytes data{0xC0, 0x00, 0x41, 0xDB, 0xDC, 0xC0};
        const Bytes bad_escape{0xC0, 0x00, 0x41, 0xDB, 0x42, 0xC0};
        const Bytes dangling_escape{0xC0, 0x00, 0x41, 0xDB, 0xC0};
        REQUIRE(uni_kiss_valid(empty.data(), empty.size()));
        REQUIRE(uni_kiss_valid(data.data(), data.size()));
        REQUIRE_FALSE(uni_kiss_valid(bad_escape.data(), bad_escape.size()));
        REQUIRE_FALSE(uni_kiss_valid(dangling_escape.data(), dangling_escape.size()));
    }
}


TEST_CASE("kiss_fuzz", "[kiss][fuzz]") {
    std::mt19937 rng(0x4B495353U);
    auto random_byte = [&rng]() {
        // bias towards the special bytes so that escapes are exercised often
        static constexpr uint8_t special[] = {0xC0, 0xDB, 0xDC, 0xDD, 0x00};
        const uint32_t pick = rng() % 8U;
        return pick < 5U ? special[pick] : static_cast<uint8_t>(rng());
    };

    SECTION("roundtrip with every destination size") {
        for (int iteration = 0; iteration < 2000; iteration++) {
            Bytes payload(rng() % 48U);
            for (auto &byte : payload) {
                byte = random_byte();
            }
            const Bytes frame = Encode(payload);
            REQUIRE(uni_kiss_valid(frame.data(), frame.size()));

            for (size_t dst_size = 0; dst_size <= frame.size(); dst_size++) {
                ExactBuffer encoded(dst_size);
                const size_t size = uni_kiss_encode(payload.data(), payload.size(), encoded.data.get(), encoded.size);
                REQUIRE((size == (dst_size >= frame.size() ? frame.size() : 0U)));
            }
            for (size_t dst_size = 0; dst_size <= payload.size(); dst_size++) {
                const Bytes decoded = Decode(frame, dst_size);
                REQUIRE(decoded == (dst_size >= payload.size() ? payload : Bytes{}));
            }
        }
    }

    SECTION("random and mutated frames match the reference decoder") {
        for (int iteration = 0; iteration < 20000; iteration++) {
            Bytes frame;
            if (iteration % 2 == 0) {
                frame.resize(rng() % 24U);
                for (auto &byte : frame) {
                    byte = random_byte();
                }
                if (frame.size() >= 2U && rng() % 2U == 0U) {
                    frame.front() = 0xC0;
                    frame.back() = 0xC0;
                }
            } else {
                Bytes payload(rng() % 24U);
                for (auto &byte : payload) {
                    byte = random_byte();
                }
                frame = Encode(payload);
                const uint32_t mutations = 1U + rng() % 3U;
                for (uint32_t m = 0; m < mutations; m++) {
                    switch (rng() % 3U) {
                    case 0:
                        frame[rng() % frame.size()] = random_byte();
                        break;
                    case 1:
                        frame.erase(frame.begin() + static_cast<std::ptrdiff_t>(rng() % frame.size()));
                        break;
                    default:
                        frame.insert(frame.begin() + static_cast<std::ptrdiff_t>(rng() % (frame.size() + 1U)), random_byte());
                        break;
                    }
                    if (frame.empty()) {
                        break;
                    }
                }
            }

            Bytes expected;
            const bool reference_valid = ReferenceDecode(frame, expected);
            REQUIRE(uni_kiss_valid(frame.data(), frame.size()) == reference_valid);

            const size_t dst_size = rng() % 32U;
            const Bytes decoded = Decode(frame, dst_size);
            if (reference_valid && expected.size() <= dst_size) {
                REQUIRE(decoded == expected);
            } else {
                REQUIRE(decoded.empty());
            }

            // encoding is canonical: a valid port-0 data frame re-encodes to the same bytes
            if (reference_valid && frame[1] == 0x00U) {
                REQUIRE(Encode(expected) == frame);
            }
        }
    }
}
