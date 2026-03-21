/*
 * Copyright (c) 2024 The ZMK Contributors
 * SPDX-License-Identifier: MIT
 *
 * SOCD (Simultaneous Opposing Cardinal Directions) Behavior
 * Implements "last input priority": when holding key A and pressing key B,
 * the keyboard acts as if A was released and only B is active.
 * When B is released, A becomes active again if it is still physically held.
 */

#define DT_DRV_COMPAT zmk_behavior_socd

#include <zephyr/device.h>
#include <drivers/behavior.h>
#include <zephyr/logging/log.h>
#include <zmk/behavior.h>
#include <zmk/event_manager.h>
#include <zmk/events/keycode_state_changed.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#if DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT)

/* Maximum number of distinct opposing-key pairs across the entire keymap.
 * Each unique (key, opponent) combination uses one slot. 1 pair = 1 slot. */
#define MAX_SOCD_PAIRS 8

struct socd_pair_state {
    uint32_t key_a; /* normalized: lower keycode of the pair */
    uint32_t key_b; /* normalized: higher keycode of the pair */
    bool a_held;    /* key_a is physically held by the user */
    bool b_held;    /* key_b is physically held by the user */
    bool valid;
};

/* Accessed only from the ZMK system workqueue (single-threaded), so no lock needed. */
static struct socd_pair_state states[MAX_SOCD_PAIRS];

static struct socd_pair_state *get_state(uint32_t key, uint32_t opponent) {
    uint32_t ka = (key <= opponent) ? key : opponent;
    uint32_t kb = (key <= opponent) ? opponent : key;

    for (int i = 0; i < MAX_SOCD_PAIRS; i++) {
        if (states[i].valid && states[i].key_a == ka && states[i].key_b == kb) {
            return &states[i];
        }
        if (!states[i].valid) {
            states[i].key_a = ka;
            states[i].key_b = kb;
            states[i].a_held = false;
            states[i].b_held = false;
            states[i].valid = true;
            return &states[i];
        }
    }
    return NULL;
}

static int on_keymap_binding_pressed(struct zmk_behavior_binding *binding,
                                     struct zmk_behavior_binding_event event) {
    uint32_t key = binding->param1;
    uint32_t opponent = binding->param2;

    struct socd_pair_state *state = get_state(key, opponent);
    if (!state) {
        LOG_ERR("SOCD: exceeded max pairs (%d); key 0x%08X will behave as plain kp",
                MAX_SOCD_PAIRS, key);
        return ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(key, true, event.timestamp));
    }

    bool is_a = (key <= opponent);
    bool opponent_held = is_a ? state->b_held : state->a_held;

    if (is_a) {
        state->a_held = true;
    } else {
        state->b_held = true;
    }

    if (opponent_held) {
        /* Synthetically release the opponent so only our key is active */
        int ret = ZMK_EVENT_RAISE(
            zmk_keycode_state_changed_from_encoded(opponent, false, event.timestamp));
        if (ret < 0) {
            LOG_ERR("SOCD: failed to raise synthetic release for opponent 0x%08X: %d", opponent,
                    ret);
            return ret;
        }
    }

    return ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(key, true, event.timestamp));
}

static int on_keymap_binding_released(struct zmk_behavior_binding *binding,
                                      struct zmk_behavior_binding_event event) {
    uint32_t key = binding->param1;
    uint32_t opponent = binding->param2;

    struct socd_pair_state *state = get_state(key, opponent);
    if (!state) {
        LOG_ERR("SOCD: no state found on release for key 0x%08X; sending plain release", key);
        return ZMK_EVENT_RAISE(
            zmk_keycode_state_changed_from_encoded(key, false, event.timestamp));
    }

    bool is_a = (key <= opponent);
    if (is_a) {
        state->a_held = false;
    } else {
        state->b_held = false;
    }

    bool opponent_held = is_a ? state->b_held : state->a_held;

    int ret = ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(key, false, event.timestamp));

    if (opponent_held) {
        /* Re-activate opponent since it is still physically held */
        ZMK_EVENT_RAISE(zmk_keycode_state_changed_from_encoded(opponent, true, event.timestamp));
    }

    return ret;
}

static const struct behavior_driver_api behavior_socd_driver_api = {
    .binding_pressed = on_keymap_binding_pressed,
    .binding_released = on_keymap_binding_released,
};

#define SOCD_INST(n)                                                                               \
    BEHAVIOR_DT_INST_DEFINE(n, NULL, NULL, NULL, NULL, POST_KERNEL,                               \
                            CONFIG_KERNEL_INIT_PRIORITY_DEFAULT, &behavior_socd_driver_api);

DT_INST_FOREACH_STATUS_OKAY(SOCD_INST)

#endif /* DT_HAS_COMPAT_STATUS_OKAY */
