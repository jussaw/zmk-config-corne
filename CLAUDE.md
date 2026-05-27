# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build

Firmware is built entirely via GitHub Actions — there is no local build step. Push to `main` (or open a PR) to trigger a build. Download `.uf2` artifacts from the Actions run and flash each half via USB.

The left half (`corne_left`) is built with ZMK Studio support (`studio-rpc-usb-uart` snippet + `CONFIG_ZMK_STUDIO=y`); the right half is not.

## Hardware

- **Controllers**: nice!nano (board ID `nice_nano//zmk`)
- **Shields**: `corne_left` / `corne_right` with `nice_view_adapter` and `nice_view_gem`
- **Display module**: [nice-view-gem](https://github.com/M165437/nice-view-gem) — pulled via `config/west.yml` from the `m165437` remote at `main`

## Architecture

| File | Purpose |
|---|---|
| `config/corne.keymap` | All layers and custom behaviors (DeviceTree syntax) |
| `config/corne.conf` | Kconfig options (display, BLE, sleep, pointing) |
| `config/west.yml` | West manifest — pins ZMK and nice-view-gem dependencies |
| `build.yaml` | GitHub Actions matrix — defines which board/shield/snippet combos to build |

## Layers

```
0  WINDOWS   — Colemak-DH, home row mods (LGUI/LALT/LCTL/LSFT on A R S T)
1  LOWER     — Symbols (!, @, #…) and brackets; LOWER+UPPER → ADJUST
2  UPPER     — Numbers, F-keys, navigation; UPPER+LOWER → ADJUST
3  ADJUST    — Media keys, mouse movement, and scroll (conditional layer: LOWER+UPPER)
4  SETTINGS  — Bluetooth profile selection (BT0–BT4), game toggle
5  GAME      — QWERTY, no home row mods, modifier keys on bottom row
```

Bluetooth profiles are assigned: BT0 = Windows desktop, BT1 = Personal MBA, BT2 = Work MBP.

## Custom Behaviors

- **`hm` (home_row_mod)**: `tap-preferred` hold-tap, 200 ms tapping term, `require-prior-idle-ms = 125`. Used on home row for modifier access.
- **`HYPER`** (`#define`): expands to `LC(LS(LA(LGUI)))`, used as `&kp HYPER` on the left thumb key of every layer.
