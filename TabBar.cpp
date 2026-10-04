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

struct TabStrip::State {
    detail::TrackMotion motion;
    detail::Spring panelOffset;
    detail::GlowSpring halo;
    bool touched = false;
    bool moved = false;
    float downX = 0.0f;
    float downY = 0.0f;
    float lastX = 0.0f;
    float dragOffset = 0.0f;
    bool offsetActive = false;
    bool seeded = false;
    int observed = 0;

    State()
        : motion(0.0f, 0.0f, 3.0f, config::Motion::DRAG_VISIBLE, 1.0f,
                 config::Motion::TAB_INDICATOR_HELD_SCALE),
          panelOffset(0.0f, 1.0f, config::Motion::TAB_PANEL_STIFFNESS,
                      config::Motion::TAB_PANEL_VISIBLE) {}
};

TabStrip::TabStrip() : state_(new State()) {}

TabStrip::~TabStrip() { delete state_; }

TabStrip::State& TabStrip::state() { return *state_; }

const TabStrip::State& TabStrip::state() const { return *state_; }

bool TabStrip::appendTab(const char* title, unsigned int texture) {
    State& s = state();
    if (tabCount >= TAB_LIMIT) return false;
    titles[tabCount] = title;
    textures[tabCount] = texture;
    painters[tabCount] = nullptr;
    tags[tabCount] = nullptr;
    ++tabCount;
    if (activeIndex > tabCount - 1) activeIndex = tabCount - 1;
    s.seeded = false;
    return tabCount < TAB_LIMIT;
}

bool TabStrip::dropTab(int index) {
    State& s = state();
    if (index < 0 || index >= tabCount) return false;
    for (int i = index; i < tabCount - 1; ++i) {
        titles[i] = titles[i + 1];
        textures[i] = textures[i + 1];
        painters[i] = painters[i + 1];
        tags[i] = tags[i + 1];
    }
    --tabCount;
    if (tabCount <= 0) {
        tabCount = 0;
        activeIndex = 0;
    } else if (activeIndex > tabCount - 1) {
        activeIndex = tabCount - 1;
    }
    s.seeded = false;
    return tabCount <= TAB_LIMIT;
}

void TabStrip::resetTabs() {
    State& s = state();
    tabCount = 0;
    activeIndex = 0;
    s.seeded = false;
    for (int i = 0; i < TAB_LIMIT; ++i) {
        titles[i] = "";
        textures[i] = 0u;
        painters[i] = nullptr;
        tags[i] = nullptr;
    }
}

void TabStrip::select(int index, bool withMotion) {
    State& s = state();
    if (index < 0) index = 0;
    if (tabCount <= 0) index = 0;
    else if (index > tabCount - 1) index = tabCount - 1;
    activeIndex = index;
    s.observed = index;
    s.seeded = true;
    if (withMotion) {
        s.motion.animateTo((float)index);
    }
    else {
        s.motion.snapValue((float)index);
    }
}

float TabStrip::indicator() const { return state().motion.value(); }

bool TabStrip::busy() const {
    const State& s = state();
    return s.motion.moving() || s.halo.running() || s.offsetActive;
}

namespace {

void paintCells(const TabStrip& strip, int count, float iconPx, float labelPx, float gapPx,
                float minX, float minY, float inset, float cellW, float innerH,
                unsigned int color) {
    for (int i = 0; i < count; ++i) {
        const char* caption = strip.titles[i] != nullptr ? strip.titles[i] : "";
        const TextSize span = detail::textMetrics(caption, labelPx);
        const bool iconPresent = strip.textures[i] != 0u || strip.painters[i] != nullptr;
        const float stackHeight = (iconPresent ? iconPx + gapPx : 0.0f) + span.height;
        const float centerX = minX + inset + cellW * ((float)i + 0.5f);
        const float centerY = minY + inset + innerH * 0.5f;
        float top = centerY - stackHeight * 0.5f;
        if (iconPresent) {
            if (strip.textures[i] != 0u) {
                detail::drawTexture(strip.textures[i],
                                    Box(centerX - iconPx * 0.5f, top, centerX + iconPx * 0.5f,
                                        top + iconPx), color);
            }
            else {
                detail::paintIcon(strip.painters[i], Point(centerX, top + iconPx * 0.5f),
                                  iconPx, color, strip.tags[i]);
            }
            top += iconPx + gapPx;
        }
        detail::drawTextAt(Point(centerX - span.width * 0.5f, top), labelPx, color, caption,
                           0.0f);
    }
}

}

bool TabStrip::draw(const Box& box) {
    State& s = state();
    s.motion.step(detail::frameDelta());
    s.halo.step(detail::frameDelta());
    changed = false;

    const float width = box.width();
    const float height = box.height();
    if (width <= 0.0f || height <= 0.0f) return false;

    int count = tabCount < 1 ? 1 : (tabCount > TAB_LIMIT ? TAB_LIMIT : tabCount);
    s.motion.setRange(0.0f, (float)(count - 1));
    const int clamped = activeIndex < 0 ? 0 : (activeIndex > count - 1 ? count - 1 : activeIndex);
    if (activeIndex != clamped) activeIndex = clamped;
    if (!s.seeded) {
        s.seeded = true;
        s.observed = activeIndex;
        s.motion.snapValue((float)activeIndex);
    } else if (activeIndex != s.observed && !s.touched) {
        s.observed = activeIndex;
        s.motion.animateTo((float)activeIndex);
    }

    const float panelHeight = px(config::Layout::TAB_PANEL_H_DP);
    const float innerHeight = px(config::Layout::TAB_INNER_H_DP);
    const float inset = px(config::Layout::TAB_INSET_DP);
    const float panelY = box.top + (height - panelHeight) * 0.5f;
    const float cellWidth = (width - inset * 2.0f) / (float)count;

    const PointerProbe probe = detail::probeBox(box, this, true);
    const PointerState& pointer = detail::pointerRef();

    if (probe.activated) {
        s.downX = pointer.x;
        s.downY = pointer.y;
        s.lastX = pointer.x;
        s.moved = false;
        s.touched = true;
        s.dragOffset = 0.0f;
        s.panelOffset.snapTo(0.0f);
        s.halo.press(pointer.x - box.left, pointer.y - box.top);
        s.motion.press();
    }
    if (s.touched && (probe.held || pointer.down)) {
        const float step = pointer.x - s.lastX;
        s.lastX = pointer.x;
        const float offsetX = pointer.x - s.downX;
        const float offsetY = pointer.y - s.downY;
        const float slop = px(config::Layout::TAB_SLOP_DP);
        if (offsetX * offsetX + offsetY * offsetY > slop * slop) s.moved = true;
        if (step != 0.0f) {
            s.dragOffset += step;
            s.panelOffset.snapTo(s.dragOffset);
            s.motion.updateValue(clampRange(s.motion.targetValue() + step / cellWidth,
                                            0.0f, (float)(count - 1)));
            s.halo.move(pointer.x - box.left, pointer.y - box.top);
        }
    }
    if (s.touched && !pointer.down) {
        s.touched = false;
        s.halo.release();
        int target = (int)lroundf(s.motion.targetValue());
        if (!s.moved) {
            target = (int)floorf((pointer.x - (box.left + inset)) / cellWidth);
        }
        target = target < 0 ? 0 : (target > count - 1 ? count - 1 : target);
        if (target != activeIndex) {
            activeIndex = target;
            changed = true;
        }
        s.observed = activeIndex;
        s.motion.animateTo((float)activeIndex);
        s.panelOffset.animateTo(0.0f);
        s.offsetActive = true;
    }
    if (s.offsetActive) {
        if (!s.panelOffset.step(detail::frameDelta())) s.offsetActive = false;
        s.dragOffset = s.panelOffset.value;
    }

    const float press = s.motion.pressProgress();
    const float value = s.motion.value();
    float panelShift = 0.0f;
    if (width > 0.0f) {
        const float fraction = clampRange(s.dragOffset / width, -1.0f, 1.0f);
        const float direction = fraction < 0.0f ? -1.0f : (fraction > 0.0f ? 1.0f : 0.0f);
        panelShift = px(config::Layout::TAB_PANEL_SHIFT_DP) * direction
                   * easeOutCubic(fabsf(fraction));
    }

    const float haloLevel = s.halo.visible() ? saturate(s.halo.progress()) : 0.0f;
    const bool light = detail::lightFlag();
    const unsigned int container = light && containerColor == config::Layout::TAB_CONTAINER
        ? config::Layout::TAB_CONTAINER_LIGHT : containerColor;
    const unsigned int accent = light && accentColor == config::Layout::TAB_ACCENT
        ? config::Layout::TAB_ACCENT_LIGHT : accentColor;

    const float panelScale = lerp(1.0f, 1.0f + px(config::Layout::TAB_PANEL_GROW_DP)
                                             / (width > 1.0f ? width : 1.0f), press);
    const float panelWidth = width * panelScale;
    const float panelHeightScaled = panelHeight * panelScale;
    const float panelLeft = box.left + (width - panelWidth) * 0.5f + panelShift;
    const float panelTop = panelY + (panelHeight - panelHeightScaled) * 0.5f;
    const Box panelBox(panelLeft, panelTop, panelLeft + panelWidth,
                       panelTop + panelHeightScaled);

    {
        PanelStyle style;
        style.corners = Corners::capsule();
        style.optics.colorBoost = true;
        style.optics.blurPx = px(config::Layout::TAB_BLUR_DP);
        style.optics.lensHeightPx = px(config::Layout::TAB_LENS_HEIGHT_DP);
        style.optics.lensAmountPx = px(config::Layout::TAB_LENS_AMOUNT_DP);
        style.optics.depthAmount = 0.0f;
        style.surface = rgbaOf(container);
        style.surfaceMode = SurfaceMode::Overlay;
        style.edge = EdgeLight::gradient();
        style.edgeOn = true;
        style.shadowOn = true;
        style.shadow = DropShadow();
        if (haloLevel > config::Render::ALPHA_MIN) {
            style.glow.on = true;
            style.glow.progress = haloLevel;
            style.glow.x = clampRange(inset * panelScale
                                      + (value + 0.5f) * cellWidth * panelScale,
                                      0.0f, panelWidth);
            style.glow.y = clampRange(inset * panelScale
                                      + innerHeight * panelScale * 0.5f,
                                      0.0f, panelHeightScaled);
        }
        const int panel = submitPanel(style, panelBox);
        if (panel >= 0) {
            detail::panelCapture(panel);
            detail::panelPaint(panel);
        }
    }

    const float visualInset = inset * panelScale;
    const float visualInner = innerHeight * panelScale;
    const float visualWidth = width * panelScale;
    const float visualCell = (width - inset * 2.0f) * panelScale / (float)count;
    const float contentMinX = box.left + (width - visualWidth) * 0.5f + panelShift;
    const float contentMinY = panelY + (panelHeight - panelHeight * panelScale) * 0.5f;

    const float cellScale = lerp(1.0f, config::Layout::TAB_CELL_SCALE, press);
    const float iconPx = px(iconSizeDp) * cellScale;
    const float labelPx = px(labelSizeDp) * cellScale;
    const float gapPx = px(config::Layout::TAB_ICON_GAP_DP) * cellScale;
    const unsigned int idleColor = light ? rgba(1.0f, 0x000000u) : rgba(1.0f, 0xFFFFFFu);
    const unsigned int activeColor = accent;
    const unsigned int forcedColor = iconColor != 0u ? iconColor : 0u;

    paintCells(*this, count, iconPx, labelPx, gapPx, contentMinX, contentMinY, visualInset,
               visualCell, visualInner, forcedColor != 0u ? forcedColor : idleColor);

    const float velocity = s.motion.speed() * config::Layout::TAB_VELOCITY_GAIN;
    const float clamp = config::Motion::DRAG_STRETCH_CLAMP;
    const float scaleX = s.motion.scaleX()
        / (1.0f - clampRange(velocity * config::Motion::DRAG_STRETCH_X, -clamp, clamp));
    const float scaleY = s.motion.scaleY()
        * (1.0f - clampRange(velocity * config::Motion::DRAG_STRETCH_Y, -clamp, clamp));
    const float indicatorW = visualCell * scaleX;
    const float indicatorH = visualInner * scaleY;
    const float indicatorX = contentMinX + visualInset + value * visualCell
                           + (visualCell - indicatorW) * 0.5f;
    const float indicatorY = contentMinY + visualInset + (visualInner - indicatorH) * 0.5f;
    const Box indicatorBox(indicatorX, indicatorY, indicatorX + indicatorW,
                           indicatorY + indicatorH);

    detail::pushClipBox(indicatorBox);
    paintCells(*this, count, iconPx, labelPx, gapPx, contentMinX, contentMinY, visualInset,
               visualCell, visualInner, forcedColor != 0u ? forcedColor : activeColor);
    detail::popClipBox();

    {
        const float idleAlpha = config::Layout::TAB_IDLE_ALPHA * (1.0f - press);
        const float pressAlpha = config::Layout::TAB_PRESS_ALPHA * press;
        const float baseAlpha = idleAlpha * (1.0f - pressAlpha);
        const float totalAlpha = pressAlpha + baseAlpha;
        const float luma = light ? 0.0f
            : (totalAlpha > config::Motion::TAB_ALPHA_EPSILON ? baseAlpha / totalAlpha : 0.0f);

        PanelStyle style;
        style.corners = Corners::capsule();
        style.optics.colorBoost = false;
        style.optics.blurPx = 0.0f;
        style.optics.lensHeightPx = px(config::Layout::TAB_PRESS_LENS_HEIGHT_DP) * press;
        style.optics.lensAmountPx = px(config::Layout::TAB_PRESS_LENS_AMOUNT_DP) * press;
        style.optics.depthAmount = 0.0f;
        style.optics.spectral = config::Layout::TAB_SPECTRAL;
        style.surface = Rgba(luma, luma, luma, totalAlpha);
        style.surfaceMode = SurfaceMode::Overlay;
        style.edge = EdgeLight::gradient();
        style.edge.color = config::Render::EDGE_TINT;
        style.edge.alpha = press;
        style.edgeOn = press > config::Render::ALPHA_MIN;
        style.shadowOn = true;
        style.shadow = DropShadow(config::Layout::TAB_SHADOW_DP, 0.0f,
                                  config::Layout::TAB_SHADOW_DP / 6.0f,
                                  rgba(config::Layout::TAB_SHADOW_ALPHA * press,
                                       0x000000u), 1.0f);
        style.insetOn = press > config::Render::ALPHA_MIN;
        style.inset = InsetShadow(px(config::Layout::TAB_PRESS_INNER_DP) * press, press);

        const int panel = submitPanel(style, indicatorBox);
        if (panel >= 0) {
            detail::panelCapture(panel);
            detail::panelPaint(panel);
        }
    }
    return changed;
}

}
