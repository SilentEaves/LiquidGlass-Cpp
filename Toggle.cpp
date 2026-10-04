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
#include "GlassInternal.h"

#include <math.h>

namespace lgx {

struct Toggle::State {
    detail::TrackMotion motion;
    bool touched = false;
    bool dragged = false;
    bool seeded = false;
    bool observed = false;
    float pressX = 0.0f;
    float lastX = 0.0f;

    State()
        : motion(0.0f, 0.0f, 1.0f, config::Motion::DRAG_VISIBLE, 1.0f,
                 config::Motion::DRAG_SCALE) {}
};

Toggle::Toggle() : state_(new State()) {}

Toggle::~Toggle() { delete state_; }

Toggle::State& Toggle::state() { return *state_; }

const Toggle::State& Toggle::state() const { return *state_; }

void Toggle::setOn(bool value, bool withMotion) {
    State& s = state();
    on = value;
    s.observed = value;
    s.seeded = true;
    if (withMotion) {
        s.motion.animateTo(value ? 1.0f : 0.0f);
    }
    else {
        s.motion.snapValue(value ? 1.0f : 0.0f);
    }
}

bool Toggle::busy() const { return state().motion.moving(); }

bool Toggle::draw(const Box& box) {
    State& s = state();
    s.motion.step(detail::frameDelta());

    if (!s.seeded) {
        s.seeded = true;
        s.observed = on;
        s.motion.snapValue(on ? 1.0f : 0.0f);
    } else if (on != s.observed && !s.touched) {
        s.observed = on;
        s.motion.animateTo(on ? 1.0f : 0.0f);
    }

    bool flipped = false;
    const float trackWidth = px(config::Layout::TOGGLE_TRACK_W_DP);
    const float trackHeight = px(config::Layout::TOGGLE_TRACK_H_DP);
    const float thumbWidth = px(config::Layout::TOGGLE_THUMB_W_DP);
    const float thumbHeight = px(config::Layout::TOGGLE_THUMB_H_DP);
    const float trackLeft = box.left;
    const float trackTop = box.top + (box.height() - trackHeight) * 0.5f;
    const float thumbTop = box.top + (box.height() - thumbHeight) * 0.5f;

    const PointerProbe probe = detail::probeBox(box, this, true);
    const PointerState& pointer = detail::pointerRef();

    if (probe.activated) {
        s.pressX = pointer.x;
        s.lastX = pointer.x;
        s.touched = true;
        s.dragged = false;
        s.motion.press();
    }
    if (s.touched && (probe.held || pointer.down)) {
        const float now = pointer.x;
        const float step = now - s.lastX;
        s.lastX = now;
        if (fabsf(now - s.pressX) > px(config::Layout::TOGGLE_DRAG_SLOP_DP)) s.dragged = true;
        const float travel = step / px(config::Layout::TOGGLE_DRAG_SCALE_DP);
        s.motion.updateValue(clampRange(s.motion.targetValue() + travel, 0.0f, 1.0f));
    }
    if (s.touched && !probe.held && !pointer.down) {
        s.touched = false;
        const bool next = s.dragged ? (s.motion.targetValue() >= 0.5f) : !on;
        if (next != on) {
            on = next;
            flipped = true;
        }
        s.observed = on;
        s.motion.updateValue(on ? 1.0f : 0.0f);
        s.dragged = false;
        s.motion.release();
    }

    const float value = s.motion.value();
    const float press = s.motion.pressProgress();
    const unsigned int accent = (accentColor == config::Layout::TOGGLE_ACCENT)
        ? detail::tintMixRgb(config::Layout::TOGGLE_ACCENT,
                              config::Layout::TOGGLE_ACCENT_LIGHT)
        : accentColor;
    const unsigned int neutral = (trackColor == config::Layout::TOGGLE_TRACK)
        ? detail::tintTrackArgb() : trackColor;
    const unsigned int blended = mixRgb(neutral, accent, saturate(value));

    const float padding = px(config::Layout::TOGGLE_PADDING_DP);
    const float travel = lerp(padding, padding + px(config::Layout::TOGGLE_TRAVEL_DP), value);
    const float velocity = s.motion.speed() * config::Layout::TOGGLE_VELOCITY_GAIN;
    const float clamp = config::Motion::DRAG_STRETCH_CLAMP;
    const float scaleX = s.motion.scaleX()
        / (1.0f - clampRange(velocity * config::Motion::DRAG_STRETCH_X, -clamp, clamp));
    const float scaleY = s.motion.scaleY()
        * (1.0f - clampRange(velocity * config::Motion::DRAG_STRETCH_Y, -clamp, clamp));
    const float width = thumbWidth * scaleX;
    const float height = thumbHeight * scaleY;
    const float thumbLeft = trackLeft + travel - (width - thumbWidth) * 0.5f;
    const float thumbY = thumbTop + (thumbHeight - height) * 0.5f;
    const Box thumb(thumbLeft, thumbY, thumbLeft + width, thumbY + height);

    PanelStyle style;
    style.corners = Corners::capsule();
    style.optics.colorBoost = false;
    style.optics.blurPx = px(lerp(config::Layout::GLASS_BLUR_FLOOR_DP,
                                   config::Layout::GLASS_BLUR_DP, 1.0f - press));
    style.optics.lensHeightPx = px(config::Layout::TOGGLE_LENS_HEIGHT_DP) * press;
    style.optics.lensAmountPx = px(config::Layout::TOGGLE_LENS_AMOUNT_DP) * press;
    style.optics.depthAmount = 0.0f;
    style.optics.darkGuard = config::Layout::TOGGLE_DARK_GUARD;
    style.optics.spectral = 1.0f;
    style.backdropScaleX = lerp(config::Layout::TOGGLE_BACKDROP_X_MIN,
                                config::Layout::TOGGLE_BACKDROP_X_MAX, press);
    style.backdropScaleY = lerp(config::Layout::TOGGLE_BACKDROP_Y_MIN,
                                config::Layout::TOGGLE_BACKDROP_Y_MAX, press);
    style.backdropBlend = 1.0f;
    const Rgba knob = detail::tintKnob();
    style.surface = Rgba(knob.r, knob.g, knob.b, saturate(1.0f - press));
    style.surfaceMode = SurfaceMode::Overlay;
    style.edge = EdgeLight::ambient(press);
    style.edgeOn = true;
    style.shadowOn = false;
    style.insetOn = true;
    style.inset = InsetShadow(config::Layout::TOGGLE_INNER_DP
                                  * fmaxf(press, config::Layout::TOGGLE_INNER_REST_DP),
                              saturate(config::Layout::TOGGLE_INNER_REST_ALPHA
                                       + (1.0f - config::Layout::TOGGLE_INNER_REST_ALPHA)
                                         * press));
    style.trackOn = true;
    style.trackBox = Box(trackLeft, trackTop, trackLeft + trackWidth, trackTop + trackHeight);
    style.trackRadius = trackHeight * 0.5f;
    style.trackColor = rgbaOf(blended);

    const int panel = submitPanel(style, thumb);
    if (panel >= 0) detail::panelCapture(panel);
    detail::fillRound(style.trackBox, style.trackRadius, blended);
    if (panel >= 0) detail::panelPaint(panel);
    return flipped;
}

}
