# Mercury Test Board Firmware

Teensy firmware for the Mercury test board used by the M1 manufacturing test platform.

This project was converted from the older `M1Combined/TestBoardMercury` CMake-style tree to the same PlatformIO approach used by the red-diamonds fixture reference project:

```text
git@github.com:lbuchman/redDiamondsFixture.git
teensy/
```

The red-diamonds project is the reference for skeleton, shared module usage, and formatting files. This repository owns the Mercury-specific firmware.

## Active Firmware Facts

- Build system: PlatformIO
- Environment: `teensy41`
- Framework: Arduino
- Version define: `FWVERSION=0.1` in `platformio.ini`
- Runtime readback: shell command `about` returns JSON containing `fw`

## Build

```bash
pio run
```

The build produces:

```text
.pio/build/teensy41/firmware.hex
```

## Upload

For a directly connected Teensy:

```bash
pio run -t upload
```

The existing `program.sh` is still present for fixture Raspberry Pi programming flow.

## Platform Version Rule

For platform release control:

- expected version: release manifest source commit and `FWVERSION`
- actual installed version: hardware readback through the board command path, using `about`

