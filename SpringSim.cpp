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
#include "GlassContext.h"

#include <math.h>

namespace lgx {
namespace detail {

Spring::Spring(float initial, float dampingRatio, float stiffnessValue, float visibility)
    : value(initial), velocity(0.0f), target(initial), damping(dampingRatio),
      stiffness(stiffnessValue),
      threshold(visibility > config::Motion::SPRING_EPSILON ? visibility
                                                            : config::Motion::SPRING_EPSILON) {}

void Spring::snapTo(float v) {
    value = v;
    target = v;
    velocity = 0.0f;
}

void Spring::animateTo(float v) { target = v; }

bool Spring::step(float dt) {
    if (!(dt > 0.0f)) return false;
    if (dt > config::Motion::STEP_DELTA_MAX) dt = config::Motion::STEP_DELTA_MAX;

    const float omegaSquared = stiffness > config::Motion::SPRING_EPSILON
        ? stiffness : config::Motion::SPRING_EPSILON;
    const float drag = 2.0f * damping * sqrtf(omegaSquared);

    int substeps = (int)(dt * config::Motion::SUBSTEP_RATE + 0.5f);
    if (substeps < 1) substeps = 1;
    if (substeps > config::Motion::SUBSTEP_LIMIT) substeps = config::Motion::SUBSTEP_LIMIT;
    const float h = dt / (float)substeps;

    for (int i = 0; i < substeps; ++i) {
        const float offset = value - target;
        const float accel = -omegaSquared * offset - drag * velocity;
        velocity += accel * h;
        value += velocity * h;
    }

    if (!isfinite(value) || !isfinite(velocity)) {
        value = isfinite(target) ? target : 0.0f;
        if (!isfinite(target)) target = 0.0f;
        velocity = 0.0f;
    }

    if (fabsf(value - target) <= threshold
            && fabsf(velocity) <= threshold * config::Motion::VELOCITY_SETTLE_GAIN) {
        value = target;
        velocity = 0.0f;
    }
    return value != target || velocity != 0.0f;
}

bool Spring::moving() const { return value != target || velocity != 0.0f; }

TrackMotion::TrackMotion(float initialValue, float rangeStart, float rangeEnd, float visibility,
                         float idleScale, float heldScale, float valueStiffness,
                         float valueDamping)
    : rangeStart_(rangeStart), rangeEnd_(rangeEnd), idleScale_(idleScale),
      heldScale_(heldScale),
      valueSpring_(initialValue, valueDamping, valueStiffness, visibility),
      speedSpring_(0.0f, config::Motion::DRAG_TRACK_DAMPING,
                   config::Motion::DRAG_TRACK_STIFFNESS, visibility * config::Motion::DRAG_SPEED_VISIBLE_GAIN),
      pressSpring_(0.0f, config::Motion::DRAG_PRESS_DAMPING,
                   config::Motion::DRAG_PRESS_STIFFNESS, config::Motion::DRAG_VISIBLE),
      scaleXSpring_(idleScale, config::Motion::DRAG_SCALE_X_DAMPING,
                    config::Motion::DRAG_SCALE_STIFFNESS, config::Motion::DRAG_VISIBLE),
      scaleYSpring_(idleScale, config::Motion::DRAG_SCALE_Y_DAMPING,
                    config::Motion::DRAG_SCALE_STIFFNESS, config::Motion::DRAG_VISIBLE),
      lastValue_(initialValue) {}

float TrackMotion::clampToRange(float v) const {
    if (v < rangeStart_) return rangeStart_;
    return v > rangeEnd_ ? rangeEnd_ : v;
}

float TrackMotion::progress() const {
    const float span = rangeEnd_ - rangeStart_;
    return span == 0.0f ? 0.0f : (valueSpring_.value - rangeStart_) / span;
}

float TrackMotion::pressProgress() const { return saturate(pressSpring_.value); }

void TrackMotion::setRange(float start, float end) {
    rangeStart_ = start;
    rangeEnd_ = end;
    if (valueSpring_.target < start) valueSpring_.target = start;
    if (valueSpring_.target > end) valueSpring_.target = end;
    if (valueSpring_.value < start) {
        valueSpring_.value = start;
        lastValue_ = start;
    }
    if (valueSpring_.value > end) {
        valueSpring_.value = end;
        lastValue_ = end;
    }
}

void TrackMotion::press() {
    releasePending_ = false;
    releaseArmed_ = false;
    lastValue_ = valueSpring_.value;
    pressSpring_.animateTo(1.0f);
    scaleXSpring_.animateTo(heldScale_);
    scaleYSpring_.animateTo(heldScale_);
}

void TrackMotion::release() {
    releasePending_ = true;
    releaseArmed_ = true;
}

void TrackMotion::updateValue(float v) {
    trackSpeed_ = true;
    valueSpring_.animateTo(clampToRange(v));
}

void TrackMotion::snapValue(float v) {
    trackSpeed_ = true;
    valueSpring_.snapTo(clampToRange(v));
    measureSpeed();
}

void TrackMotion::animateTo(float v) {
    press();
    settleTo(v);
    release();
}

void TrackMotion::settleTo(float v) {
    trackSpeed_ = false;
    valueSpring_.animateTo(clampToRange(v));
    if (speedSpring_.value != 0.0f || speedSpring_.target != 0.0f) {
        speedSpring_.animateTo(0.0f);
    }
}

void TrackMotion::measureSpeed() {
    const float span = rangeEnd_ - rangeStart_;
    if (fabsf(span) > config::Render::SPAN_MIN) {
        speedSpring_.animateTo((valueSpring_.value - lastValue_) / lastStep_ / span);
    }
    lastValue_ = valueSpring_.value;
}

bool TrackMotion::step(float dt) {
    lastStep_ = dt > 0.0001f ? dt : config::Motion::FRAME_FALLBACK;
    bool stepping = false;
    stepping |= valueSpring_.step(dt);
    if (trackSpeed_) measureSpeed();
    stepping |= speedSpring_.step(dt);

    if (releasePending_) {
        if (releaseArmed_) {
            releaseArmed_ = false;
        }
        else {
            const float span = fabsf(rangeEnd_ - rangeStart_) * config::Motion::DRAG_SETTLE_SPAN;
            if (valueSpring_.value == valueSpring_.target
                    || fabsf(valueSpring_.value - valueSpring_.target) < span) {
                pressSpring_.animateTo(0.0f);
                scaleXSpring_.animateTo(idleScale_);
                scaleYSpring_.animateTo(idleScale_);
                releasePending_ = false;
            }
        }
    }

    stepping |= pressSpring_.step(dt);
    stepping |= scaleXSpring_.step(dt);
    stepping |= scaleYSpring_.step(dt);
    if (releasePending_) stepping = true;

    const float safeStart = isfinite(rangeStart_) ? rangeStart_ : 0.0f;
    const float safeTarget = isfinite(valueSpring_.target) ? valueSpring_.target : safeStart;
    if (!isfinite(valueSpring_.value)) valueSpring_.value = safeTarget;
    if (!isfinite(valueSpring_.velocity)) valueSpring_.velocity = 0.0f;
    if (!isfinite(valueSpring_.target)) valueSpring_.target = safeStart;
    if (!isfinite(speedSpring_.value)) speedSpring_.value = 0.0f;
    if (!isfinite(speedSpring_.target)) speedSpring_.target = 0.0f;
    if (!isfinite(pressSpring_.value)) pressSpring_.value = 0.0f;
    if (!isfinite(scaleXSpring_.value)) scaleXSpring_.value = idleScale_;
    if (!isfinite(scaleYSpring_.value)) scaleYSpring_.value = idleScale_;
    return stepping;
}

bool TrackMotion::moving() const {
    return valueSpring_.moving() || speedSpring_.moving() || pressSpring_.moving()
        || scaleXSpring_.moving() || scaleYSpring_.moving() || releasePending_;
}

void GlowSpring::press(float x, float y) {
    if (!enabled_) return;
    anchorX_ = x;
    anchorY_ = y;
    strength_.animateTo(1.0f);
    spotX_.snapTo(anchorX_);
    spotY_.snapTo(anchorY_);
    running_ = true;
}

void GlowSpring::move(float x, float y) {
    if (!enabled_) return;
    spotX_.snapTo(x);
    spotY_.snapTo(y);
    if (strength_.value != strength_.target) running_ = true;
}

void GlowSpring::release() {
    if (!enabled_) return;
    strength_.animateTo(0.0f);
    spotX_.animateTo(anchorX_);
    spotY_.animateTo(anchorY_);
    running_ = true;
}

void GlowSpring::abort() {
    strength_.snapTo(0.0f);
    spotX_.snapTo(anchorX_);
    spotY_.snapTo(anchorY_);
    running_ = false;
}

void GlowSpring::step(float dt) {
    bool stepping = false;
    stepping |= strength_.step(dt);
    stepping |= spotX_.step(dt);
    stepping |= spotY_.step(dt);
    running_ = stepping;
}

}
}
