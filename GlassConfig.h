/*
 * Copyright 2026 默檐
 * SPDX-License-Identifier: Apache-2.0
 *
 * 本项目（LiquidGlass-Cpp）是基于 Kyant 项目的 C++ 独立移植与二次开发。
 * 项目开源地址：[https://github.com/SilentEaves/LiquidGlass-Cpp.git]
 *
 * ==================== 声明 ====================
 * 本项目由 默檐 于 2026 年独立完成 C++ 移植与二次修改。
 * 具体修改包括：将原项目（Kotlin）逻辑重写为 C++。
 * 本项目同样遵守并应用 Apache License, Version 2.0 协议。
 * ====================================================
 *
 * ------------------------------------------------------------------------
 * 第三方开源项目声明：
 * 本文件的核心代码逻辑移植自 AndroidLiquidGlass 项目，来源 Kyant 的液态玻璃项目。
 *
 * 原项目版权：Copyright 2025 Kyant
 * 原项目地址：https://github.com/Kyant0/AndroidLiquidGlass
 * 原项目许可证：Apache License, Version 2.0
 * 原项目许可证链接：http://www.apache.org/licenses/LICENSE-2.0
 * ------------------------------------------------------------------------
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#pragma once

namespace lgx {
namespace config {

inline constexpr float DENSITY_MIN = 0.5f;
inline constexpr float DENSITY_MAX = 8.0f;
inline constexpr float CAPSULE_RADIUS = 3.4028235e+38f;
inline constexpr float DEGREES_TO_RADIANS = 0.017453292f;
inline constexpr float COLOR_CHANNEL = 255.0f;
inline constexpr float HALF = 0.5f;
inline constexpr float PI = 3.1415927f;
inline constexpr float TWO_PI = 6.2831855f;
inline constexpr float HALF_PI = 1.5707964f;

namespace Render {

inline constexpr float BLUR_SIGMA_FROM_RADIUS = 0.57735f;
inline constexpr float BLUR_MIN_SIGMA = 0.5f;
inline constexpr float BLUR_FBO_SIGMA_CAP = 3.0f;
inline constexpr float BLUR_STEP_FROM_SIGMA = 0.25f;
inline constexpr float BLUR_MIN_STEP = 0.0f;
inline constexpr int BLUR_TAPS = 12;
inline constexpr int BLUR_DECIMATE_LIMIT = 16;
inline constexpr int BLUR_TARGET_MIN_SPAN = 8;
inline constexpr float BLUR_STEP_BOX_AT = 1.5f;
inline constexpr float BLUR_BOX_HALF_RATIO = 0.5f;
inline constexpr int BLUR_TARGET_ALIGN = 32;
inline constexpr int PANEL_JOB_LIMIT = 64;
inline constexpr float BLUR_CAPTURE_SHRINK_RATIO = 0.25f;
inline constexpr int CAPTURE_ALLOC_QUANTUM = 64;
inline constexpr int ALLOC_SLACK_DIVISOR = 2;
inline constexpr int BLUR_CAPTURE_SHRINK_FRAMES = 120;
inline constexpr float BLUR_RADIUS_MAX_DP = 10.0f;
inline constexpr float LENS_REACH_MAX = 128.0f;

inline constexpr float CAPTURE_PAD_BASE = 3.0f;
inline constexpr float CAPTURE_PAD_BLUR_GAIN = 2.0f;
inline constexpr float CAPTURE_PAD_BLUR_GAIN_SAFE = 3.0f;
inline constexpr float CAPTURE_PAD_MAX = 640.0f;

inline constexpr float EDGE_WIDTH_MIN = 0.5f;
inline constexpr float EDGE_SOFTNESS_MIN = 0.25f;
inline constexpr float EDGE_ANGLE_DEFAULT = 45.0f;
inline constexpr float EDGE_FALLOFF_DEFAULT = 1.0f;
inline constexpr float EDGE_PLAIN_WIDTH = 0.5f;
inline constexpr float EDGE_PLAIN_SOFTNESS = 0.25f;
inline constexpr float EDGE_PLAIN_ALPHA = 1.0f;
inline constexpr float EDGE_AMBIENT_WIDTH = 0.33333334f;
inline constexpr float EDGE_AMBIENT_SOFTNESS = 0.16666667f;
inline constexpr float EDGE_AMBIENT_ALPHA = 0.38f;
inline constexpr float DARK_GUARD_FLOOR = 0.045f;

inline constexpr float EDGE_CONTOUR_ALPHA = 0.50f;
inline constexpr float EDGE_CONTOUR_ANGLE = -90.0f;
inline constexpr float EDGE_CONTOUR_FALLOFF = 1.35f;
inline constexpr float EDGE_CONTOUR_WIDTH = 0.6f;
inline constexpr float EDGE_CONTOUR_SOFTNESS = 0.30f;
inline constexpr float EDGE_CONTOUR_PRESS_GAIN = 0.34f;

inline constexpr float SHADOW_EXTENT_GAIN = 2.0f;
inline constexpr float GLOW_REACH_SCALE = 1.5f;
inline constexpr float SPECTRAL_MAX = 2.0f;

inline constexpr int PRISM_PAIRS = 3;
inline constexpr float PRISM_DIST[3] = {1.0f / 3.0f, 2.0f / 3.0f, 1.0f};
inline constexpr float PRISM_EVEN_RED[3] = {3.0f / 21.0f, 3.0f / 21.0f, 9.0f / 42.0f};
inline constexpr float PRISM_ODD_RED[3] = {3.0f / 21.0f, 3.0f / 21.0f, 3.0f / 42.0f};
inline constexpr float PRISM_EVEN_GREEN[3] = {6.0f / 21.0f, 3.0f / 42.0f, 0.0f};
inline constexpr float PRISM_ODD_GREEN[3] = {0.0f, 3.0f / 42.0f, 0.0f};
inline constexpr float PRISM_EVEN_BLUE[3] = {7.0f / 42.0f, 7.0f / 42.0f, 7.0f / 42.0f};
inline constexpr float PRISM_ODD_BLUE[3] = {-7.0f / 42.0f, -7.0f / 42.0f, -7.0f / 42.0f};
inline constexpr float PRISM_CENTER_GREEN = 6.0f / 21.0f;

inline constexpr float PANEL_OPACITY_MAX = 1.0f;
inline constexpr float TRACK_NEUTRAL_ALPHA = 0.20f;
inline constexpr float SPECTRAL_MIN = 0.001f;
inline constexpr float ALPHA_MIN = 0.001f;
inline constexpr float CONTENT_ALPHA_MIN = 0.004f;
inline constexpr float SURFACE_ALPHA_MIN = 0.005f;
inline constexpr float GEOMETRY_MIN = 0.001f;
inline constexpr float SPAN_MIN = 0.0001f;
inline constexpr float GLOW_LEVEL_MIN = 0.001f;
inline constexpr float BACKDROP_SCALE_MIN = 0.001f;
inline constexpr int CAPTURE_CLEAR_BYTES = 16384;

inline constexpr unsigned SHADOW_TINT_SOFT = 0x0D000000u;
inline constexpr float SHADOW_OFFSET_DIVISOR = 6.0f;
inline constexpr unsigned SHADOW_TINT_DIALOG = 0x000000u;
inline constexpr unsigned DIALOG_CONFIRM_TEXT = 0xFFFFFFu;
inline constexpr unsigned EDGE_TINT = 0x80FFFFFFu;
inline constexpr unsigned EDGE_TINT_PLAIN = 0x61FFFFFFu;
inline constexpr unsigned EDGE_TINT_AMBIENT = 0x61FFFFFFu;
inline constexpr unsigned EDGE_TINT_CONTOUR = 0xCCFFFFFFu;
inline constexpr unsigned EDGE_TINT_DIALOG = 0xFFFFFFu;
inline constexpr unsigned MENU_BAND = 0x3DFFFFFFu;
inline constexpr unsigned ALERT = 0xFF3B30u;


}

namespace Color {

inline constexpr float SWITCH_SECONDS = 0.34f;

inline constexpr float LUMA_BAND_LO = 0.30f;
inline constexpr float LUMA_BAND_HI = 0.72f;
inline constexpr float LUMA_FLIP = 0.50f;

inline constexpr float DARK_PRIMARY_R = 0.07f;
inline constexpr float DARK_PRIMARY_G = 0.08f;
inline constexpr float DARK_PRIMARY_B = 0.10f;
inline constexpr float DARK_SECONDARY_R = 0.11f;
inline constexpr float DARK_SECONDARY_G = 0.12f;
inline constexpr float DARK_SECONDARY_B = 0.16f;
inline constexpr float LIGHT_PRIMARY_R = 0.98f;
inline constexpr float LIGHT_PRIMARY_G = 0.98f;
inline constexpr float LIGHT_PRIMARY_B = 0.99f;

inline constexpr float SHEET_GAIN = 1.0f;
inline constexpr float SHEET_MIX_DARK = 0.06f;
inline constexpr float SHEET_MIX_LIGHT = 0.18f;
inline constexpr float SHEET_PULL_DARK = 0.20f;
inline constexpr float SHEET_PULL_LIGHT = 0.10f;
inline constexpr float SHEET_ALPHA_DARK = 0.34f;
inline constexpr float SHEET_ALPHA_LIGHT = 0.62f;

inline constexpr float KNOB_MIX_DARK = 0.06f;
inline constexpr float KNOB_MIX_LIGHT = 0.18f;


inline constexpr float SCRIM_GAIN = 0.42f;
inline constexpr float SCRIM_ALPHA_DARK = 0.56f;
inline constexpr float SCRIM_ALPHA_LIGHT = 0.30f;

inline constexpr float TEXT_LIFT_DARK = 0.96f;
inline constexpr float TEXT_LIFT_LIGHT = 0.08f;

inline constexpr float EDGE_GAIN = 1.0f;
inline constexpr float EDGE_MIX_DARK = 0.55f;
inline constexpr float EDGE_MIX_LIGHT = 0.62f;

inline constexpr float TRACK_MIX = 0.55f;

inline constexpr float HUE_BRIDGE = 0.35f;
inline constexpr float HUE_BRIDGE_REF = 0.35f;

inline constexpr float SPECTRUM_GAIN = 1.0f;
inline constexpr float SPECTRUM_MIX = 0.45f;
inline constexpr float REFRACT_GAIN = 0.22f;

inline constexpr float BRIGHTNESS_DARK = 0.0f;
inline constexpr float BRIGHTNESS_LIGHT = 0.04f;
inline constexpr float SATURATION_DARK = 1.0f;
inline constexpr float SATURATION_LIGHT = 1.0f;
inline constexpr float CONTRAST_DARK = 1.0f;
inline constexpr float CONTRAST_LIGHT = 1.0f;

inline constexpr float BUTTON_SHEET_GAIN = 0.55f;
inline constexpr float BUTTON_SHEET_MIX_DARK = 0.14f;
inline constexpr float BUTTON_SHEET_MIX_LIGHT = 0.62f;
inline constexpr float BUTTON_SHEET_PULL = 0.22f;
inline constexpr float BUTTON_SHEET_ALPHA_DARK = 0.22f;
inline constexpr float BUTTON_SHEET_ALPHA_LIGHT = 0.36f;
inline constexpr float BUTTON_SHEET_LIFT_DARK = 0.06f;

}

namespace Motion {

inline constexpr float SPRING_EPSILON = 0.000001f;
inline constexpr float STEP_DELTA_MAX = 0.064f;
inline constexpr float SUBSTEP_RATE = 720.0f;
inline constexpr int SUBSTEP_LIMIT = 64;
inline constexpr float VELOCITY_SETTLE_GAIN = 62.5f;
inline constexpr float FRAME_FALLBACK = 0.016666668f;

inline constexpr float PRESS_DAMPING = 0.5f;
inline constexpr float PRESS_STIFFNESS = 300.0f;
inline constexpr float PRESS_VISIBLE = 0.001f;
inline constexpr float POINTER_DAMPING = 0.5f;
inline constexpr float POINTER_STIFFNESS = 300.0f;
inline constexpr float POINTER_VISIBLE = 0.5f;

inline constexpr float DRAG_VALUE_DAMPING = 1.0f;
inline constexpr float DRAG_VALUE_STIFFNESS = 1000.0f;
inline constexpr float DRAG_TRACK_DAMPING = 0.5f;
inline constexpr float DRAG_TRACK_STIFFNESS = 300.0f;
inline constexpr float DRAG_PRESS_DAMPING = 1.0f;
inline constexpr float DRAG_PRESS_STIFFNESS = 1000.0f;
inline constexpr float DRAG_SCALE_STIFFNESS = 250.0f;
inline constexpr float DRAG_SCALE_X_DAMPING = 0.6f;
inline constexpr float DRAG_SCALE_Y_DAMPING = 0.7f;
inline constexpr float DRAG_VISIBLE = 0.001f;
inline constexpr float DRAG_SETTLE_SPAN = 0.025f;
inline constexpr float DRAG_SCALE = 1.5f;
inline constexpr float DRAG_SPEED_VISIBLE_GAIN = 10.0f;
inline constexpr float DRAG_STRETCH_X = 0.75f;
inline constexpr float DRAG_STRETCH_Y = 0.25f;
inline constexpr float DRAG_STRETCH_CLAMP = 0.2f;

inline constexpr float EASE_OUT_CONTROL = 0.58f;
inline constexpr float EASE_SOLVE_TOLERANCE = 0.00001f;
inline constexpr int EASE_SOLVE_ITERATIONS = 8;

inline constexpr float CURVE_X1 = 0.22f;
inline constexpr float CURVE_Y1 = 1.00f;
inline constexpr float CURVE_X2 = 0.36f;
inline constexpr float CURVE_Y2 = 1.00f;
inline constexpr float CURVE_TOLERANCE = 0.00001f;
inline constexpr float CURVE_SLOPE_FLOOR = 0.0001f;
inline constexpr int CURVE_ITERATIONS = 10;
inline constexpr float CURVE_HOLD = 0.62f;

inline constexpr float MENU_UNFOLD_STIFFNESS = 168.0f;
inline constexpr float MENU_WIDEN_STIFFNESS = 136.0f;
inline constexpr float MENU_DEEPEN_STIFFNESS = 114.0f;
inline constexpr float MENU_GREET_STIFFNESS = 248.0f;
inline constexpr float MENU_SPRING_DAMPING = 1.0f;
inline constexpr float MENU_SPRING_VISIBLE = 0.001f;
inline constexpr float MENU_REST_VELOCITY = 0.5f;
inline constexpr float MENU_DELTA_MAX = 0.05f;

inline constexpr float DIALOG_PROGRESS_DAMPING = 1.0f;
inline constexpr float DIALOG_PROGRESS_STIFFNESS = 230.0f;
inline constexpr float DIALOG_PROGRESS_STIFFNESS_OPEN = 125.0f;
inline constexpr float DIALOG_PROGRESS_STIFFNESS_CLOSE = 110.0f;
inline constexpr float DIALOG_POP_DAMPING = 0.86f;
inline constexpr float DIALOG_POP_STIFFNESS = 300.0f;
inline constexpr float DIALOG_POP_DAMPING_OPEN = 0.66f;
inline constexpr float DIALOG_POP_STIFFNESS_OPEN = 170.0f;
inline constexpr float DIALOG_POP_DAMPING_CLOSE = 1.0f;
inline constexpr float DIALOG_POP_STIFFNESS_CLOSE = 150.0f;
inline constexpr float DIALOG_GROW_SECONDS = 0.62f;
inline constexpr float DIALOG_GROW_WIDE_STIFFNESS = 160.0f;
inline constexpr float DIALOG_GROW_TALL_STIFFNESS = 108.0f;
inline constexpr float DIALOG_SPRING_VISIBLE = 0.001f;
inline constexpr float DIALOG_GLIDE_WEIGHT = 4.5f;
inline constexpr int DIALOG_NO_ACTION = 0;

inline constexpr float DIALOG_BLUR_RISE_SECONDS = 0.34f;
inline constexpr float DIALOG_BLUR_FALL_SECONDS = 0.12f;
inline constexpr float DIALOG_GLASS_RISE_SECONDS = 0.40f;
inline constexpr float DIALOG_GLASS_FALL_SECONDS = 0.16f;
inline constexpr float DIALOG_CLOSE_SCALE = 0.10f;
inline constexpr float DIALOG_FLARE_GAIN = 4.0f;
inline constexpr float DIALOG_READY_PROGRESS = 0.995f;
inline constexpr float DIALOG_READY_SETTLE = 0.96f;
inline constexpr float DIALOG_DRAIN_TO = 0.88f;
inline constexpr float DIALOG_LENS_KICK = 0.20f;
inline constexpr float DIALOG_GROW_LIMIT = 1.35f;
inline constexpr float DIALOG_SHADOW_ARRIVAL_MIN = 0.005f;
inline constexpr float DIALOG_DISPERSAL_MIN = 0.004f;
inline constexpr float MENU_RIM_ALPHA_MIN = 0.002f;
inline constexpr float MENU_CAP_TEXT_MIN = 0.004f;
inline constexpr float MENU_BAND_ALPHA_MIN = 0.002f;
inline constexpr float MENU_CAP_TOGGLE_AT = 0.1f;
inline constexpr float MENU_BAND_MOVE_MIN = 0.01f;
inline constexpr float SHELL_TARGET_NEAR = 0.999f;
inline constexpr float SHELL_REST_AT = 0.001f;
inline constexpr float SHELL_GREET_RISE_FROM = 0.42f;
inline constexpr float SHELL_GREET_RISE_SPAN = 0.30f;
inline constexpr float SHELL_GREET_FALL_FROM = 0.72f;
inline constexpr float SHELL_GREET_FALL_SPAN = 0.27f;
inline constexpr float SHELL_BLOOM_OPEN_FROM = 0.60f;
inline constexpr float SHELL_BLOOM_OPEN_SPAN = 0.26f;
inline constexpr float SHELL_BLOOM_CLOSE_FROM = 0.86f;
inline constexpr float SHELL_BLOOM_CLOSE_SPAN = 0.14f;
inline constexpr float SHELL_DROPLET_MAX_RATIO = 0.92f;
inline constexpr float SHELL_INNER_MIN = 0.001f;
inline constexpr float SHELL_PICK_FAR = 1.0e30f;
inline constexpr float TAB_ALPHA_EPSILON = 1.0e-4f;

inline constexpr float DIALOG_BLUR_OPEN_FROM = 0.80f;
inline constexpr float DIALOG_BLUR_OPEN_TO = 1.00f;
inline constexpr float DIALOG_BLUR_CLOSE_TO = 0.16f;
inline constexpr float DIALOG_GLASS_OPEN_FROM = 0.94f;
inline constexpr float DIALOG_GLASS_OPEN_TO = 1.00f;
inline constexpr float DIALOG_GLASS_CLOSE_TO = 0.14f;
inline constexpr float DIALOG_CLOSE_EASE_TO = 0.60f;
inline constexpr float DIALOG_DISPERSAL_FROM = 0.02f;
inline constexpr float DIALOG_DISPERSAL_TO = 0.86f;
inline constexpr float DIALOG_PRESENCE_FROM = 0.06f;
inline constexpr float DIALOG_PRESENCE_TO = 0.92f;
inline constexpr float DIALOG_MATERIAL_TO = 0.35f;
inline constexpr float DIALOG_SURFACE_FROM = 0.06f;
inline constexpr float DIALOG_SURFACE_TO = 0.80f;
inline constexpr float DIALOG_SPECTRAL_FROM = 0.55f;
inline constexpr float DIALOG_SPECTRAL_TO = 1.00f;
inline constexpr float DIALOG_LENS_FROM = 0.08f;
inline constexpr float DIALOG_LENS_TO = 0.78f;
inline constexpr float DIALOG_SHADOW_FROM = 0.80f;
inline constexpr float DIALOG_SHADOW_TO = 1.00f;
inline constexpr float DIALOG_TITLE_EXIT_FROM = 0.02f;
inline constexpr float DIALOG_TITLE_EXIT_TO = 0.36f;
inline constexpr float DIALOG_BODY_EXIT_FROM = 0.06f;
inline constexpr float DIALOG_BODY_EXIT_TO = 0.42f;
inline constexpr float DIALOG_BUTTON_EXIT_FROM = 0.10f;
inline constexpr float DIALOG_BUTTON_EXIT_TO = 0.48f;
inline constexpr float DIALOG_TITLE_OUT_FROM = 0.10f;
inline constexpr float DIALOG_TITLE_OUT_TO = 0.60f;
inline constexpr float DIALOG_BODY_OUT_FROM = 0.14f;
inline constexpr float DIALOG_BODY_OUT_TO = 0.66f;
inline constexpr float DIALOG_BUTTON_OUT_FROM = 0.18f;
inline constexpr float DIALOG_BUTTON_OUT_TO = 0.72f;
inline constexpr float DIALOG_TITLE_IN_FROM = 0.60f;
inline constexpr float DIALOG_TITLE_IN_TO = 0.84f;
inline constexpr float DIALOG_BODY_IN_FROM = 0.68f;
inline constexpr float DIALOG_BODY_IN_TO = 0.90f;
inline constexpr float DIALOG_BUTTON_IN_FROM = 0.76f;
inline constexpr float DIALOG_BUTTON_IN_TO = 0.96f;
inline constexpr float DIALOG_BUTTON_SCALE_FROM = 0.78f;
inline constexpr float DIALOG_BUTTON_SCALE_TO = 0.94f;

inline constexpr float TAB_INDICATOR_HELD_SCALE = 78.0f / 56.0f;
inline constexpr float TAB_PANEL_STIFFNESS = 300.0f;
inline constexpr float TAB_PANEL_VISIBLE = 0.5f;

inline constexpr float SHELL_VEIL_AT = 0.34f;
inline constexpr float SHELL_LIVE_AT = 0.78f;
inline constexpr float SHELL_DISMISS_AT = 0.30f;
inline constexpr float SHELL_VEIL_SPAN = 0.36f;
inline constexpr float SHELL_INNER_BASE = 0.66f;
inline constexpr float SHELL_INNER_SPAN = 0.62f;
inline constexpr float SHELL_BAND_SECONDS = 0.22f;
inline constexpr float SHELL_BAND_FADE_SECONDS = 0.16f;
inline constexpr float SHELL_PRESS_SECONDS = 0.15f;
inline constexpr float SHELL_FLOW_RISE_SECONDS = 0.09f;
inline constexpr float SHELL_FLOW_FALL_SECONDS = 0.22f;
inline constexpr float SHELL_FLOW_SPEED_REFERENCE = 2.0f;
inline constexpr float SHELL_ARM_SLOP_DP = 4.0f;

inline constexpr float SHELL_FLARE_GAIN = 1.5f;
inline constexpr float SHELL_SQUASH = 0.14f;
inline constexpr float SHELL_SQUASH_X = 0.08f;
inline constexpr float SHELL_STRETCH_ALONG = 0.10f;
inline constexpr float SHELL_STRETCH_CROSS = 0.06f;
inline constexpr float SHELL_GREET_STRETCH = 0.13f;
inline constexpr float SHELL_GREET_NARROW = 0.06f;
inline constexpr float SHELL_DROPLET_ROUND = 0.55f;
inline constexpr float SHELL_SETTLE_CURVE = 3.0f;
inline constexpr float SHELL_ROW_STAGGER = 0.30f;
inline constexpr float SHELL_ROW_SPAN = 0.22f;
inline constexpr float SHELL_MARK_POP = 0.12f;
inline constexpr float SHELL_SUB_LAG = 0.10f;
inline constexpr float SHELL_SUB_HOT = 0.06f;
inline constexpr float SHELL_BAND_STRETCH = 0.10f;
inline constexpr float SHELL_BAND_POP = 0.05f;
inline constexpr float SHELL_BAND_PRESS = 0.25f;
inline constexpr float SHELL_SETTLE_FLARE = 0.12f;
inline constexpr float SHELL_BLOOM_VELOCITY_GAIN = 1.0f;
inline constexpr float SHELL_ROW_SLIDE_DP = 6.0f;
inline constexpr float SHELL_THREAD_TAPER = 0.55f;
inline constexpr float SHELL_THREAD_FADE_DP = 14.0f;
inline constexpr float SHELL_THREAD_BLEND_GAIN = 1.35f;
inline constexpr float SHELL_FEED_BLEND_GAIN = 1.15f;
inline constexpr float SHELL_FEED_TAIL_DP = 2.0f;
inline constexpr float SHELL_TAIL_MIN = 0.22f;
inline constexpr float SHELL_DROP_ALIVE = 0.05f;
inline constexpr float SHELL_DROP_SPAN = 0.06f;
inline constexpr float SHELL_SHAPE_MIN_SPAN = 0.5f;
inline constexpr float SHELL_THREAD_MIN_RADIUS = 0.4f;

}

namespace Layout {

inline constexpr float BUTTON_TEXT_SP = 16.0f;
inline constexpr float BUTTON_LENS_HEIGHT_DP = 12.0f;
inline constexpr float BUTTON_LENS_AMOUNT_DP = 24.0f;
inline constexpr float BUTTON_BLUR_DP = 2.0f;
inline constexpr float GLASS_BLUR_DP = 4.0f;
inline constexpr float GLASS_BLUR_FLOOR_DP = 1.0f;
inline constexpr float BUTTON_GLOW_RISE_DP = 4.0f;
inline constexpr float BUTTON_DRAG_TANH = 0.05f;
inline constexpr float BUTTON_ICON_SCALE = 1.15f;
inline constexpr float BUTTON_ICON_GAP = 0.45f;
inline constexpr float BUTTON_VISUAL_SCALE_MAX = 1.5f;

inline constexpr float TOGGLE_TRACK_W_DP = 64.0f;
inline constexpr float TOGGLE_TRACK_H_DP = 28.0f;
inline constexpr float TOGGLE_THUMB_W_DP = 40.0f;
inline constexpr float TOGGLE_THUMB_H_DP = 24.0f;
inline constexpr float TOGGLE_PADDING_DP = 2.0f;
inline constexpr float TOGGLE_TRAVEL_DP = 20.0f;
inline constexpr float TOGGLE_DRAG_SCALE_DP = 20.0f;
inline constexpr float TOGGLE_DRAG_SLOP_DP = 3.0f;
inline constexpr float TOGGLE_LENS_HEIGHT_DP = 5.0f;
inline constexpr float TOGGLE_LENS_AMOUNT_DP = 10.0f;
inline constexpr float TOGGLE_BACKDROP_X_MIN = 2.0f / 3.0f;
inline constexpr float TOGGLE_BACKDROP_X_MAX = 0.75f;
inline constexpr float TOGGLE_BACKDROP_Y_MIN = 0.55f;
inline constexpr float TOGGLE_BACKDROP_Y_MAX = 0.75f;
inline constexpr unsigned TOGGLE_ACCENT = 0xFF30D158u;
inline constexpr unsigned TOGGLE_ACCENT_LIGHT = 0xFF34C759u;
inline constexpr unsigned TOGGLE_TRACK = 0x5C787880u;
inline constexpr float TOGGLE_INNER_DP = 4.0f;
inline constexpr float TOGGLE_INNER_REST_ALPHA = 0.10f;
inline constexpr float TOGGLE_INNER_REST_DP = 0.25f;
inline constexpr float TOGGLE_VELOCITY_GAIN = 1.0f / 50.0f;
inline constexpr float TOGGLE_DARK_GUARD = 1.0f;

inline constexpr float SLIDER_TRACK_H_DP = 6.0f;
inline constexpr float SLIDER_THUMB_W_DP = 40.0f;
inline constexpr float SLIDER_THUMB_H_DP = 24.0f;
inline constexpr float SLIDER_LENS_HEIGHT_DP = 12.0f;
inline constexpr float SLIDER_LENS_AMOUNT_DP = 14.0f;
inline constexpr float SLIDER_THUMB_EDGE_MIN = 0.25f;
inline constexpr float SLIDER_THUMB_EDGE_MAX = 0.75f;
inline constexpr float SLIDER_BACKDROP_X_MIN = 2.0f / 3.0f;
inline constexpr float SLIDER_BACKDROP_X_MAX = 1.0f;
inline constexpr float SLIDER_BACKDROP_Y_MIN = 0.55f;
inline constexpr float SLIDER_BACKDROP_Y_MAX = 1.0f;
inline constexpr float SLIDER_VELOCITY_GAIN = 0.1f;
inline constexpr unsigned SLIDER_ACCENT = 0xFF0091FFu;
inline constexpr unsigned SLIDER_ACCENT_LIGHT = 0xFF0088FFu;
inline constexpr unsigned SLIDER_TRACK = 0x5C787880u;
inline constexpr float SLIDER_SHADOW_DP = 4.0f;
inline constexpr float SLIDER_INNER_DP = 4.0f;

inline constexpr float TAB_PANEL_H_DP = 64.0f;
inline constexpr float TAB_INNER_H_DP = 56.0f;
inline constexpr float TAB_INSET_DP = 4.0f;
inline constexpr float TAB_SLOP_DP = 8.0f;
inline constexpr float TAB_PANEL_SHIFT_DP = 4.0f;
inline constexpr float TAB_CELL_SCALE = 1.2f;
inline constexpr float TAB_ICON_GAP_DP = 2.0f;
inline constexpr float TAB_ICON_SIZE_DP = 28.0f;
inline constexpr float TAB_LABEL_SIZE_DP = 12.0f;
inline constexpr float TAB_BLUR_DP = 8.0f;
inline constexpr float TAB_LENS_HEIGHT_DP = 24.0f;
inline constexpr float TAB_LENS_AMOUNT_DP = 24.0f;
inline constexpr float TAB_PRESS_LENS_HEIGHT_DP = 10.0f;
inline constexpr float TAB_PRESS_LENS_AMOUNT_DP = 14.0f;
inline constexpr float TAB_PANEL_GROW_DP = 16.0f;
inline constexpr float TAB_VELOCITY_GAIN = 0.1f;
inline constexpr float TAB_IDLE_ALPHA = 0.10f;
inline constexpr float TAB_PRESS_ALPHA = 0.03f;
inline constexpr float TAB_SPECTRAL = 1.0f;
inline constexpr unsigned TAB_ACCENT = 0xFF0091FFu;
inline constexpr unsigned TAB_ACCENT_LIGHT = 0xFF0088FFu;
inline constexpr unsigned TAB_CONTAINER = 0x66121212u;
inline constexpr unsigned TAB_CONTAINER_LIGHT = 0x66FAFAFAu;
inline constexpr float TAB_SHADOW_DP = 24.0f;
inline constexpr float TAB_SHADOW_ALPHA = 0.10f;
inline constexpr float TAB_PRESS_INNER_DP = 8.0f;

inline constexpr float DIALOG_MAX_W_DP = 420.0f;
inline constexpr float DIALOG_MIN_W_DP = 260.0f;
inline constexpr float DIALOG_SIDE_DP = 80.0f;
inline constexpr float DIALOG_H_DP = 252.0f;
inline constexpr float DIALOG_START_DP = 8.0f;
inline constexpr float DIALOG_START_ORIGIN_DP = 10.0f;
inline constexpr float DIALOG_PAD_X_DP = 28.0f;
inline constexpr float DIALOG_BODY_PAD_X_DP = 24.0f;
inline constexpr float DIALOG_TITLE_Y_DP = 24.0f;
inline constexpr float DIALOG_BODY_Y_DP = 68.0f;
inline constexpr float DIALOG_SETTLE_SLIDE_DP = 6.0f;
inline constexpr float DIALOG_TITLE_SIZE_DP = 24.0f;
inline constexpr float DIALOG_BODY_SIZE_DP = 15.0f;
inline constexpr float DIALOG_BODY_ALPHA = 0.68f;
inline constexpr float DIALOG_BUTTON_H_DP = 48.0f;
inline constexpr float DIALOG_BUTTON_PAD_DP = 24.0f;
inline constexpr float DIALOG_BUTTON_GAP_DP = 16.0f;
inline constexpr float DIALOG_BUTTON_SIZE_SP = 16.0f;
inline constexpr float DIALOG_CLOSE_RADIUS_DP = 48.0f;
inline constexpr float DIALOG_GLASS_BLUR_DP = 10.0f;
inline constexpr float DIALOG_LENS_HEIGHT_DP = 24.0f;
inline constexpr float DIALOG_LENS_AMOUNT_DP = 48.0f;
inline constexpr float DIALOG_FLARE_LENS_HEIGHT_DP = 16.0f;
inline constexpr float DIALOG_FLARE_LENS_AMOUNT_DP = 24.0f;
inline constexpr float DIALOG_SPECTRAL_BASE = 0.12f;
inline constexpr float DIALOG_SPECTRAL_FLARE = 0.60f;
inline constexpr float DIALOG_SPECTRAL_CLOSE = 0.55f;
inline constexpr float DIALOG_SPECTRAL_MIN = 0.002f;
inline constexpr float DIALOG_EDGE_ALPHA = 0.62f;
inline constexpr float DIALOG_EDGE_FLARE_ALPHA = 0.30f;
inline constexpr float DIALOG_SHEET_ALPHA_SCALE = 0.72f;
inline constexpr float DIALOG_SHADOW_DP = 19.0f;
inline constexpr float DIALOG_SHADOW_ALPHA = 0.11f;
inline constexpr float DIALOG_RING_BASE_DP = 3.0f;
inline constexpr float DIALOG_RING_STEP_DP = 8.0f;
inline constexpr float DIALOG_RING_ALPHA = 0.30f;
inline constexpr float DIALOG_RING_ALPHA_DECAY = 0.055f;
inline constexpr float DIALOG_RING_WIDTH_DP = 1.2f;
inline constexpr float DIALOG_HALO_DP = 26.0f;
inline constexpr float DIALOG_HALO_ALPHA = 0.05f;
inline constexpr float DIALOG_HALO_WIDTH_DP = 6.0f;
inline constexpr int DIALOG_RING_COUNT = 4;
inline constexpr float DIALOG_BUTTON_SCALE_IDLE = 0.94f;
inline constexpr float DIALOG_BUTTON_BLUR_IDLE_DP = 6.0f;
inline constexpr float DIALOG_BUTTON_BLUR_PRESS_DP = 2.0f;
inline constexpr float DIALOG_BUTTON_LENS_IDLE_DP = 9.0f;
inline constexpr float DIALOG_BUTTON_LENS_PRESS_DP = 4.0f;
inline constexpr float DIALOG_BUTTON_LENS_AMOUNT_IDLE_DP = 18.0f;
inline constexpr float DIALOG_BUTTON_LENS_AMOUNT_PRESS_DP = 8.0f;
inline constexpr float DIALOG_BUTTON_SPECTRAL_IDLE = 0.22f;
inline constexpr float DIALOG_BUTTON_SPECTRAL_PRESS = 0.45f;
inline constexpr float DIALOG_BUTTON_CANCEL_LIGHT_R = 0.97f;
inline constexpr float DIALOG_BUTTON_CANCEL_LIGHT_G = 0.98f;
inline constexpr float DIALOG_BUTTON_CANCEL_LIGHT_B = 0.99f;
inline constexpr float DIALOG_BUTTON_CANCEL_LIGHT_A = 0.52f;
inline constexpr float DIALOG_BUTTON_CANCEL_DARK_R = 0.12f;
inline constexpr float DIALOG_BUTTON_CANCEL_DARK_G = 0.13f;
inline constexpr float DIALOG_BUTTON_CANCEL_DARK_B = 0.16f;
inline constexpr float DIALOG_BUTTON_CANCEL_DARK_A = 0.46f;
inline constexpr float DIALOG_BUTTON_CONFIRM_R = 0.0f;
inline constexpr float DIALOG_BUTTON_CONFIRM_G = 0.568f;
inline constexpr float DIALOG_BUTTON_CONFIRM_B = 1.0f;
inline constexpr float DIALOG_BUTTON_CONFIRM_A = 0.92f;

inline constexpr float MENU_PAD_Y_DP = 12.0f;
inline constexpr float MENU_GAP_DP = 2.0f;
inline constexpr float MENU_SIDE_PAD_DP = 12.0f;
inline constexpr float MENU_TEXT_PAD_X_DP = 16.0f;
inline constexpr float MENU_SLOT_DP = 44.0f;
inline constexpr float MENU_CAPTION_DP = 17.0f;
inline constexpr float MENU_DETAIL_DP = 13.0f;
inline constexpr float MENU_MARK_DP = 20.0f;
inline constexpr float MENU_MARK_GAP_DP = 12.0f;
inline constexpr float MENU_HEADING_DP = 11.0f;
inline constexpr float MENU_HEADING_H_DP = 30.0f;
inline constexpr float MENU_RULE_H_DP = 12.0f;
inline constexpr float MENU_RULE_INSET_DP = 8.0f;
inline constexpr float MENU_TEXT_PAD_Y_DP = 8.0f;
inline constexpr float MENU_REACH_PAD_DP = 20.0f;
inline constexpr float MENU_TEXT_LINE = 1.2f;
inline constexpr float MENU_SIDE_MARGIN_DP = 16.0f;
inline constexpr float MENU_MIN_W_DP = 80.0f;
inline constexpr float MENU_MIN_H_DP = 80.0f;
inline constexpr float MENU_DEFAULT_W_DP = 200.0f;
inline constexpr float MENU_DEFAULT_CORNER_DP = 32.0f;
inline constexpr float MENU_DEFAULT_BAND_CORNER_DP = 24.0f;
inline constexpr float MENU_DEFAULT_THREAD_DP = 5.5f;
inline constexpr float MENU_DEFAULT_GAP_DP = 44.0f;
inline constexpr float MENU_DEFAULT_GREET_DP = 12.0f;
inline constexpr float MENU_DRAIN_START = 0.10f;
inline constexpr float MENU_DRAIN_SPAN = 0.80f;
inline constexpr float MENU_DRAIN_SPAN_MIN = 0.01f;
inline constexpr float MENU_JELLY_MAX = 2.0f;
inline constexpr float MENU_FEED_ALIVE = 0.02f;
inline constexpr float MENU_CAP_W_DP = 96.0f;
inline constexpr float MENU_CAP_TEXT_SP = 17.0f;
inline constexpr float MENU_CAP_BLUR_DP = 2.0f;
inline constexpr float MENU_CAP_LENS_HEIGHT_DP = 12.0f;
inline constexpr float MENU_CAP_LENS_AMOUNT_DP = 24.0f;
inline constexpr float MENU_CAP_SPECTRAL = 0.0f;
inline constexpr float MENU_CAP_EDGE_ALPHA = 0.50f;
inline constexpr float MENU_CAP_EDGE_WIDTH_DP = 0.6f;
inline constexpr float MENU_CAP_EDGE_SOFTNESS_DP = 0.30f;
inline constexpr float MENU_BLUR_HI_DP = 12.0f;
inline constexpr float MENU_LENS_HI_DP = 11.0f;
inline constexpr float MENU_LENS_AMOUNT_HI_DP = 22.0f;
inline constexpr float MENU_SPECTRAL_HI = 0.55f;
inline constexpr float MENU_FLOW_GATE_SPAN = 0.28f;
inline constexpr float MENU_RIM_WIDTH_DP = 1.0f;
inline constexpr float MENU_RIM_SOFTNESS_DP = 0.6f;
inline constexpr float MENU_RIM_ANGLE_DEGREES = -90.0f;
inline constexpr float MENU_RIM_FALLOFF = 1.35f;
inline constexpr float MENU_RIM_ALPHA = 0.52f;
inline constexpr float MENU_RIM_GREET_BOOST = 0.24f;
inline constexpr float MENU_RIM_FLARE_BOOST = 0.32f;
inline constexpr float MENU_FLARE_LENS_HEIGHT_DP = 16.0f;
inline constexpr float MENU_FLARE_LENS_AMOUNT_DP = 24.0f;
inline constexpr float MENU_FLARE_SPECTRAL = 0.85f;
inline constexpr float MENU_CAP_TEXT_DRAIN = 0.40f;
inline constexpr float MENU_CAP_TEXT_GREET = 0.06f;
inline constexpr float MENU_CAP_TEXT_LIFT_DP = 3.0f;
inline constexpr float MENU_HEADING_ALPHA = 0.45f;
inline constexpr float MENU_RULE_ALPHA = 0.45f;
inline constexpr float MENU_IDLE_ALPHA = 0.90f;
inline constexpr float MENU_DISABLED_ALPHA = 0.40f;
inline constexpr float MENU_SUB_ALPHA = 0.60f;
inline constexpr float MENU_HOT_ALPHA = 0.10f;
inline constexpr float MENU_BAND_IDLE = 0.62f;
inline constexpr float MENU_BAND_EDGE_ALPHA = 0.05f;
inline constexpr float MENU_RULE_MIN_DP = 0.5f;
inline constexpr int MENU_HEADING_LIMIT = 96;

inline constexpr int ARC_BEZIER_PIECES = 4;
inline constexpr float CORNER_ARC_TANGENT = 0.5522847498f;
inline constexpr float ICON_STAR_OUTER = 0.44f;
inline constexpr int STAR_ARMS = 5;
inline constexpr float STAR_INNER_RATIO = 0.42f;
inline constexpr int ICON_SEARCH_SEGMENTS = 40;
inline constexpr int ICON_PROFILE_SEGMENTS = 28;
inline constexpr int ICON_BELL_SEGMENTS = 28;
inline constexpr int ICON_GEAR_SEGMENTS = 32;
inline constexpr int ICON_DOT_SEGMENTS = 12;
inline constexpr int ICON_SMALL_DOT_SEGMENTS = 16;
inline constexpr int ICON_HEART_SEGMENTS = 20;
inline constexpr int ICON_RING_LIMIT = 96;
inline constexpr int ICON_MIN_SEGMENTS = 8;
inline constexpr float ICON_MIN_STROKE_PX = 1.0f;
inline constexpr int ICON_GEAR_TEETH = 8;
inline constexpr float ICON_HOME_APEX = 0.94f;
inline constexpr float ICON_HOME_EAVE = 0.02f;
inline constexpr float ICON_HOME_WALL = 0.86f;
inline constexpr float ICON_HOME_DOOR = 0.24f;
inline constexpr float ICON_HOME_LINTEL = 0.30f;
inline constexpr float ICON_HOME_SHOULDER = 0.70f;
inline constexpr float ICON_SEARCH_RING = 0.27f;
inline constexpr float ICON_SEARCH_HUB = 0.06f;
inline constexpr float ICON_SEARCH_HANDLE = 0.34f;
inline constexpr float ICON_SEARCH_TOUCH = 0.72f;
inline constexpr float ICON_SEARCH_STROKE = 0.13f;
inline constexpr float ICON_MESSAGE_BODY = 0.88f;
inline constexpr float ICON_MESSAGE_TOP = 0.86f;
inline constexpr float ICON_MESSAGE_BOTTOM = 0.34f;
inline constexpr float ICON_MESSAGE_TAIL = 0.84f;
inline constexpr float ICON_MESSAGE_TAIL_NEAR = 0.10f;
inline constexpr float ICON_MESSAGE_TAIL_MID = 0.32f;
inline constexpr float ICON_MESSAGE_TAIL_FAR = 0.54f;
inline constexpr float ICON_MESSAGE_ROUND = 0.44f;
inline constexpr float ICON_PROFILE_HEAD = 0.44f;
inline constexpr float ICON_PROFILE_HEAD_Y = 0.34f;
inline constexpr float ICON_PROFILE_SHOULDER = 0.78f;
inline constexpr float ICON_PROFILE_SHOULDER_Y = 0.96f;
inline constexpr float ICON_HEART_LOBE = 0.19f;
inline constexpr float ICON_HEART_SPREAD = 0.95f;
inline constexpr float ICON_HEART_TOP = 0.08f;
inline constexpr float ICON_HEART_TIP = 0.38f;
inline constexpr float ICON_HEART_BRIM = 1.86f;
inline constexpr float ICON_HEART_BASE = 0.03f;
inline constexpr float ICON_BELL_DOME = 0.70f;
inline constexpr float ICON_BELL_DOME_Y = 0.30f;
inline constexpr float ICON_BELL_LIP = 0.88f;
inline constexpr float ICON_BELL_LIP_TOP = 0.26f;
inline constexpr float ICON_BELL_LIP_BOTTOM = 0.48f;
inline constexpr float ICON_BELL_LIP_ROUND = 0.11f;
inline constexpr float ICON_BELL_CLAPPER = 0.15f;
inline constexpr float ICON_BELL_CLAPPER_Y = 0.68f;
inline constexpr float ICON_BELL_CROWN = 0.11f;
inline constexpr float ICON_BELL_CROWN_Y = 0.58f;
inline constexpr float ICON_GEAR_INNER = 0.52f;
inline constexpr float ICON_GEAR_OUTER = 0.94f;
inline constexpr float ICON_GEAR_HUB = 0.58f;
inline constexpr float ICON_GEAR_TOOTH_W = 0.30f;
inline constexpr float ICON_GEAR_HUB_W = 0.34f;
inline constexpr float ICON_CLOSE_ARM = 0.24f;
inline constexpr float ICON_CLOSE_WIDTH = 0.090f;
inline constexpr float ICON_CHECK_ARM = 0.28f;
inline constexpr float ICON_CHECK_WIDTH = 0.105f;
inline constexpr float ICON_CHECK_START = 0.05f;
inline constexpr float ICON_CHECK_ELBOW = 0.16f;
inline constexpr float ICON_CHECK_VALLEY = 0.78f;
inline constexpr float ICON_CHECK_END = 0.72f;
inline constexpr float ICON_CHEVRON_ARM = 0.22f;
inline constexpr float ICON_CHEVRON_WIDTH = 0.090f;
inline constexpr float ICON_CHEVRON_TOP = 0.50f;
inline constexpr float ICON_CHEVRON_TIP = 0.55f;

}

}
}
