# uni_kiss

KISS TNC framing (`FEND type payload FEND`, FESC/TFEND/TFESC escaping) in plain C.

- `uni_kiss_encode()` builds a port-0 data frame; `UNI_KISS_ENCODED_SIZE_MAX(n)` bytes always suffice.
  It returns 0 and never writes past `dst + dst_size` when the frame does not fit.
- `uni_kiss_valid()` checks one complete frame: delimiters, no FEND inside, no FEND/FESC type byte and
  only `FESC TFEND` / `FESC TFESC` escapes.
- `uni_kiss_decode()` unescapes a frame accepted by `uni_kiss_valid()`. The type byte (port and
  command) is not interpreted; check `src[1]` before treating a frame as data. An empty payload and an
  error both return 0.

Frames arriving over a byte stream must be split at FEND by the caller (repeated FENDs are idle fill).

## Build

```bash
cmake -S . -B build -DCPM_USE_LOCAL_PACKAGES=ON
cmake --build build
ctest --test-dir build
```

Tests use an installed Catch2 3 when available and download it through CPM otherwise. When the
project is added with `add_subdirectory()`, tests and install rules are off by default
(`UNI_KISS_BUILD_TESTS`, `UNI_KISS_INSTALL`).
