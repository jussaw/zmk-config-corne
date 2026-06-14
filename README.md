# zmk-config-corne

Personal [ZMK](https://zmk.dev) firmware config for a wireless [Corne](https://github.com/foostan/crkbd) (42-key, 3×6+3) split keyboard running on nice!nano controllers with nice-view-gem displays.

The base layer is **Colemak-DH** with home-row mods, plus symbol, number/navigation, media/mouse, settings, and gaming layers.

## Hardware

| Part | Detail |
|---|---|
| Controllers | [nice!nano](https://nicekeyboards.com/nice-nano/) (board ID `nice_nano//zmk`) |
| Shields | `corne_left` / `corne_right` |
| Displays | [nice-view-gem](https://github.com/M165437/nice-view-gem) on `nice_view_adapter` |
| Keyboard name | `j-corne-nano` |

## Layers

```
0  BASE      — Colemak-DH, home row mods (LGUI/LALT/LCTL/LSFT on A R S T)
1  LOWER     — Symbols (!, @, #…) and brackets
2  UPPER     — Numbers, F-keys, navigation
3  ADJUST    — Media keys, mouse movement, and scroll (conditional: LOWER + UPPER)
4  SETTINGS  — Bluetooth profile selection (BT0–BT4), game toggle
5  GAME      — QWERTY, no home row mods, modifiers on bottom row
```

`ADJUST` is a conditional layer that activates automatically when `LOWER` and `UPPER` are held together. `GAME` is toggled on/off from the `SETTINGS` layer (and from its own bottom-right thumb key).

The full ASCII layout for every layer lives in the comments above each layer block in [`config/corne.keymap`](config/corne.keymap).

### Bluetooth profiles

| Profile | Device |
|---|---|
| BT0 | Windows desktop |
| BT1 | Personal MacBook Air |
| BT2 | Work MacBook Pro |
| BT3 / BT4 | unassigned |

Select a profile from the `SETTINGS` layer; `BT CLR` clears the current profile's pairing.

## Custom behaviors

- **`hm` (home_row_mod)** — `tap-preferred` hold-tap, 200 ms tapping term, `require-prior-idle-ms = 125`. Provides modifier access (GUI/Alt/Ctrl/Shift) on the home row of the `BASE` and `UPPER` layers.
- **`HYPER`** — `#define` expanding to `LC(LS(LA(LGUI)))` (Ctrl+Shift+Alt+GUI). Bound as `&kp HYPER` on the left thumb of every non-gaming layer.
- **`socd` (SOCD cleaning)** — from the [zmk-behavior-socd](https://github.com/nguyendown/zmk-behavior-socd) module, applied to `A`/`D` on the `GAME` layer so opposing simultaneous presses resolve to the last input.

## Pointing / mouse

`CONFIG_ZMK_POINTING=y` enables mouse emulation, used on the `ADJUST` layer. Movement and scroll speeds are tuned above ZMK defaults:

- `ZMK_POINTING_DEFAULT_MOVE_VAL = 1500` (default 600)
- `ZMK_POINTING_DEFAULT_SCRL_VAL = 20` (default 10)

## Repository layout

| Path | Purpose |
|---|---|
| `config/corne.keymap` | All layers and custom behaviors (DeviceTree syntax) |
| `config/corne.conf` | Kconfig options — display, BLE, sleep, pointing |
| `config/west.yml` | West manifest — pins ZMK and module dependencies |
| `build.yaml` | GitHub Actions build matrix (board/shield/snippet combos) |
| `.github/workflows/build.yml` | Workflow that runs the ZMK user-config build |
| `boards/shields/` | Local shield overrides (currently empty) |

## Building & flashing

Firmware is built entirely in **GitHub Actions** — there is no local build step.

1. Push to `main` (or open a pull request) to trigger the build.
2. Open the run under the repo's **Actions** tab and download the `.uf2` artifacts (`corne_left`, `corne_right`).
3. Put each half into bootloader mode (double-tap reset) and copy its `.uf2` onto the `NICENANO` USB drive.

The left half is built with **ZMK Studio** support (`studio-rpc-usb-uart` snippet + `CONFIG_ZMK_STUDIO=y`), so it can be edited live via [ZMK Studio](https://zmk.dev/docs/features/studio); the right half is built without it.

## Dependencies

Pinned in [`config/west.yml`](config/west.yml), all tracking `main`:

- [`zmk`](https://github.com/zmkfirmware/zmk) — core firmware
- [`nice-view-gem`](https://github.com/M165437/nice-view-gem) — custom display module
- [`zmk-behavior-socd`](https://github.com/nguyendown/zmk-behavior-socd) — SOCD cleaning behavior

## Power

Deep sleep enabled (`CONFIG_ZMK_SLEEP=y`) with a 15-minute idle timeout (`CONFIG_ZMK_IDLE_SLEEP_TIMEOUT=900000`). BLE TX power is boosted (`CONFIG_BT_CTLR_TX_PWR_PLUS_8=y`).
