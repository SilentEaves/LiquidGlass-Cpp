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

namespace {

float curve(float p, float from, float to) {
    float span = to - from;
    if (!(span > config::Render::SPAN_MIN)) span = config::Render::SPAN_MIN;
    const float t = saturate((p - from) / span);
    return t * t * (3.0f - 2.0f * t);
}

float glide(float p, float from, float to) {
    float span = to - from;
    if (!(span > config::Render::SPAN_MIN)) span = config::Render::SPAN_MIN;
    const float t = saturate((p - from) / span);
    const float w = config::Motion::DIALOG_GLIDE_WEIGHT;
    const float norm = 1.0f - (1.0f + w) * expf(-w);
    return (1.0f - (1.0f + w * t) * expf(-w * t)) / norm;
}

}

struct Modal::State {
    detail::Spring progress{0.0f, config::Motion::DIALOG_PROGRESS_DAMPING,
                            config::Motion::DIALOG_PROGRESS_STIFFNESS,
                            config::Motion::DIALOG_SPRING_VISIBLE};
    detail::Spring pop{0.0f, config::Motion::DIALOG_POP_DAMPING,
                       config::Motion::DIALOG_POP_STIFFNESS,
                       config::Motion::DIALOG_SPRING_VISIBLE};
    detail::Spring growWide{0.0f, config::Motion::DIALOG_PROGRESS_DAMPING,
                            config::Motion::DIALOG_GROW_WIDE_STIFFNESS,
                            config::Motion::DIALOG_SPRING_VISIBLE};
    detail::Spring growTall{0.0f, config::Motion::DIALOG_PROGRESS_DAMPING,
                            config::Motion::DIALOG_GROW_TALL_STIFFNESS,
                            config::Motion::DIALOG_SPRING_VISIBLE};
    Box origin;
    bool hasOrigin = false;
    bool closing = false;
    float growT = 1.0f;
    float sourceDrain = 0.0f;
    float receiveFrom = 0.0f;
    float blurLevel = 0.0f;
    float glassBlur = 0.0f;
    PillButton cancel;
    PillButton confirm;

    State() {
        cancel.shadowOn = true;
        cancel.contourEdge = true;
        confirm.shadowOn = true;
        confirm.contourEdge = true;
    }
};

Modal::Modal() : state_(new State()) {}

Modal::~Modal() { delete state_; }

Modal::State& Modal::state() { return *state_; }

const Modal::State& Modal::state() const { return *state_; }

void Modal::open(const char* dialogTitle, const char* dialogMessage) {
    State& s = state();
    title = dialogTitle;
    message = dialogMessage;
    result = 0;
    s.closing = false;
    visible = true;
    s.hasOrigin = false;
    s.progress.damping = config::Motion::DIALOG_PROGRESS_DAMPING;
    s.progress.stiffness = config::Motion::DIALOG_PROGRESS_STIFFNESS_OPEN;
    s.pop.damping = config::Motion::DIALOG_POP_DAMPING_OPEN;
    s.pop.stiffness = config::Motion::DIALOG_POP_STIFFNESS_OPEN;
    s.progress.snapTo(0.0f);
    s.progress.animateTo(1.0f);
    s.pop.snapTo(0.0f);
    s.pop.animateTo(1.0f);
    s.growT = 0.0f;
    s.growWide.snapTo(0.0f);
    s.growTall.snapTo(0.0f);
    s.sourceDrain = 0.0f;
    s.receiveFrom = 0.0f;
    s.blurLevel = 0.0f;
    s.glassBlur = 0.0f;
    s.cancel.resetGlow();
    s.confirm.resetGlow();
}

void Modal::openFrom(const char* dialogTitle, const char* dialogMessage, const Box& origin) {
    open(dialogTitle, dialogMessage);
    State& s = state();
    if (origin.width() > 1.0f && origin.height() > 1.0f) {
        s.origin = origin;
        s.hasOrigin = true;
    }
}

void Modal::close(int dialogResult) {
    State& s = state();
    result = dialogResult;
    s.closing = true;
    s.progress.damping = config::Motion::DIALOG_PROGRESS_DAMPING;
    s.progress.stiffness = config::Motion::DIALOG_PROGRESS_STIFFNESS_CLOSE;
    s.pop.damping = config::Motion::DIALOG_POP_DAMPING_CLOSE;
    s.pop.stiffness = config::Motion::DIALOG_POP_STIFFNESS_CLOSE;
    s.progress.animateTo(0.0f);
    s.pop.animateTo(0.0f);
    s.receiveFrom = saturate(1.0f - s.sourceDrain);
    s.cancel.resetGlow();
    s.confirm.resetGlow();
}

float Modal::sourceScale() const {
    const State& s = state();
    if (!visible) return 1.0f;
    if (s.closing) {
        const float closeP = saturate(1.0f - saturate(s.progress.value));
        return lerp(s.receiveFrom, 1.0f,
                    glide(closeP, config::Motion::DIALOG_PRESENCE_FROM,
                          config::Motion::DIALOG_PRESENCE_TO));
    }
    return 1.0f - saturate(s.sourceDrain);
}

void Modal::advance(float dt) {
    State& s = state();
    if (!visible) {
        s.sourceDrain = 0.0f;
        s.blurLevel = 0.0f;
        s.glassBlur = 0.0f;
    }
    else {
        s.progress.step(dt);
        s.pop.step(dt);
        if (!s.closing) {
            s.growT = saturate(s.growT + dt / config::Motion::DIALOG_GROW_SECONDS);
        }
        const float growEase = detail::curveAt(s.growT);
        s.growWide.animateTo(growEase);
        s.growTall.animateTo(growEase);
        s.growWide.step(dt);
        s.growTall.step(dt);

        const float growWide = clampRange(s.growWide.value, 0.0f, config::Motion::DIALOG_GROW_LIMIT);
        const float growTall = clampRange(s.growTall.value, 0.0f, config::Motion::DIALOG_GROW_LIMIT);
        const float settle = saturate(0.5f * (growWide + growTall));
        const float closeP = s.closing ? saturate(1.0f - saturate(s.progress.value)) : 0.0f;
        if (!s.closing) {
            s.sourceDrain = saturate(curve(settle, 0.0f, config::Motion::DIALOG_DRAIN_TO));
        }

        const float wantBlur = s.closing
            ? 1.0f - curve(closeP, 0.0f, config::Motion::DIALOG_BLUR_CLOSE_TO)
            : curve(settle, config::Motion::DIALOG_BLUR_OPEN_FROM,
                    config::Motion::DIALOG_BLUR_OPEN_TO);
        const float blurTau = (wantBlur > s.blurLevel)
            ? config::Motion::DIALOG_BLUR_RISE_SECONDS : config::Motion::DIALOG_BLUR_FALL_SECONDS;
        s.blurLevel += (wantBlur - s.blurLevel) * fminf(1.0f, dt / blurTau);
        s.blurLevel = saturate(s.blurLevel);

        const float wantGlass = s.closing
            ? 1.0f - curve(closeP, 0.0f, config::Motion::DIALOG_GLASS_CLOSE_TO)
            : curve(settle, config::Motion::DIALOG_GLASS_OPEN_FROM,
                    config::Motion::DIALOG_GLASS_OPEN_TO);
        const float glassTau = (wantGlass > s.glassBlur)
            ? config::Motion::DIALOG_GLASS_RISE_SECONDS
            : config::Motion::DIALOG_GLASS_FALL_SECONDS;
        s.glassBlur += (wantGlass - s.glassBlur) * fminf(1.0f, dt / glassTau);
        s.glassBlur = saturate(s.glassBlur);

        if (s.closing && s.progress.value == 0.0f && s.progress.target == 0.0f
                && s.pop.value == 0.0f && s.pop.target == 0.0f) {
            s.progress.snapTo(0.0f);
            s.pop.snapTo(0.0f);
            s.sourceDrain = 0.0f;
            s.receiveFrom = 0.0f;
            s.blurLevel = 0.0f;
            s.glassBlur = 0.0f;
            s.cancel.resetGlow();
            s.confirm.resetGlow();
            visible = false;
            s.closing = false;
        }
    }
}

int Modal::draw(const Box& area) {
    if (!visible) return 0;
    State& s = state();
    const float dt = detail::frameDelta();
    advance(dt);
    if (!visible) return result;

    const bool light = detail::lightFlag();
    const float progress = saturate(s.progress.value);
    const float closeP = s.closing ? saturate(1.0f - progress) : 0.0f;
    const float closeEase = s.closing ? curve(closeP, 0.0f, config::Motion::DIALOG_CLOSE_EASE_TO) : 0.0f;
    const float dispersal = s.closing ? curve(closeP, config::Motion::DIALOG_DISPERSAL_FROM, config::Motion::DIALOG_DISPERSAL_TO) : 0.0f;
    const float closePresence = s.closing ? 1.0f - curve(closeP, config::Motion::DIALOG_PRESENCE_FROM, config::Motion::DIALOG_PRESENCE_TO) : 1.0f;
    const float dimPresence = saturate(s.blurLevel);

    const float fullW = area.width() > 1.0f ? area.width() : 1.0f;
    const float fullH = area.height() > 1.0f ? area.height() : 1.0f;
    const Box full(area.left, area.top, area.left + fullW, area.top + fullH);

    const float baseW = fminf(fullW - px(config::Layout::DIALOG_SIDE_DP),
                              px(config::Layout::DIALOG_MAX_W_DP));
    const float dialogW = fmaxf(px(config::Layout::DIALOG_MIN_W_DP), baseW);
    const float dialogH = px(config::Layout::DIALOG_H_DP);
    const Point center(full.left + fullW * 0.5f, full.top + fullH * 0.5f);
    const float glassHalfW = dialogW * 0.5f;
    const float glassHalfH = dialogH * 0.5f;
    const Box glass(center.x - glassHalfW, center.y - glassHalfH,
                    center.x + glassHalfW, center.y + glassHalfH);

    const float startExtent = s.hasOrigin ? px(config::Layout::DIALOG_START_ORIGIN_DP)
                                          : px(config::Layout::DIALOG_START_DP);
    const Box start = s.hasOrigin
        ? s.origin
        : Box(center.x - startExtent, center.y - startExtent,
              center.x + startExtent, center.y + startExtent);

    const float growWide = clampRange(s.growWide.value, 0.0f, config::Motion::DIALOG_GROW_LIMIT);
    const float growTall = clampRange(s.growTall.value, 0.0f, config::Motion::DIALOG_GROW_LIMIT);
    const float settle = saturate(0.5f * (growWide + growTall));
    const float flare = s.closing ? 0.0f
                                  : saturate(fabsf(growWide - growTall)
                                             * config::Motion::DIALOG_FLARE_GAIN);
    const Box opening(lerp(start.left, glass.left, growWide),
                      lerp(start.top, glass.top, growTall),
                      lerp(start.right, glass.right, growWide),
                      lerp(start.bottom, glass.bottom, growTall));

    const float visualScale = s.closing
        ? 1.0f + config::Motion::DIALOG_CLOSE_SCALE * closeEase : 1.0f;
    const float visualHalfW = dialogW * visualScale * 0.5f;
    const float visualHalfH = dialogH * visualScale * 0.5f;
    const Box closing(center.x - visualHalfW, center.y - visualHalfH,
                      center.x + visualHalfW, center.y + visualHalfH);
    const Box dialog = s.closing ? closing : opening;

    const float startRadius = fminf(start.width(), start.height()) * 0.5f;
    const float radius = s.closing
        ? px(config::Layout::DIALOG_CLOSE_RADIUS_DP) * visualScale
        : fminf(lerp(startRadius, px(config::Layout::DIALOG_CLOSE_RADIUS_DP),
                     curve(fminf(growWide, growTall), 0.0f, 1.0f)),
                fminf(dialog.width(), dialog.height()) * 0.5f);

    const unsigned int dimRgb = detail::tintScrimRgb();
    const float dimAlpha = detail::tintScrimAlpha() * dimPresence;
    detail::fillRound(full, 0.0f, rgba(dimAlpha, dimRgb));

    const PointerState& pointer = detail::pointerRef();
    const bool dialogInteractive = !s.closing
        && s.progress.value >= config::Motion::DIALOG_READY_PROGRESS && s.progress.target == 1.0f
        && s.pop.value >= config::Motion::DIALOG_READY_PROGRESS && s.pop.target == 1.0f
        && settle >= config::Motion::DIALOG_READY_SETTLE
        && s.growT >= 1.0f;
    if (dialogInteractive && pointer.pressed) {
        if (!glass.holds(pointer.x, pointer.y)) close(1);
    }

    const float material = s.closing ? closePresence : curve(settle, 0.0f, config::Motion::DIALOG_MATERIAL_TO);
    const float surfaceStage = s.closing ? closePresence : curve(settle, config::Motion::DIALOG_SURFACE_FROM, config::Motion::DIALOG_SURFACE_TO);
    const float blurPx = px(glassBlurDp) * s.glassBlur;
    const float dispersion = s.closing
        ? config::Layout::DIALOG_SPECTRAL_CLOSE * dispersal
        : config::Layout::DIALOG_SPECTRAL_BASE * curve(settle, config::Motion::DIALOG_SPECTRAL_FROM, config::Motion::DIALOG_SPECTRAL_TO)
        + config::Layout::DIALOG_SPECTRAL_FLARE * flare;

    if (material > config::Render::SURFACE_ALPHA_MIN) {
        PanelStyle style;
        style.corners = Corners::rounded(radius);
        style.optics.colorBoost = false;
        const float lensStage = s.closing ? 1.0f : curve(settle, config::Motion::DIALOG_LENS_FROM, config::Motion::DIALOG_LENS_TO);
        const float lensKick = s.closing ? 1.0f + config::Motion::DIALOG_LENS_KICK * dispersal : 1.0f;
        style.optics.lensHeightPx = px(config::Layout::DIALOG_LENS_HEIGHT_DP) * lensStage
                                      * lensKick
                                  + px(config::Layout::DIALOG_FLARE_LENS_HEIGHT_DP) * flare;
        style.optics.lensAmountPx = px(config::Layout::DIALOG_LENS_AMOUNT_DP) * lensStage
                                      * lensKick
                                  + px(config::Layout::DIALOG_FLARE_LENS_AMOUNT_DP) * flare;
        style.optics.depthAmount = 1.0f;
        style.optics.spectral = dispersion > config::Layout::DIALOG_SPECTRAL_MIN ? dispersion : 0.0f;
        style.optics.blurPx = blurPx;
        style.edge = EdgeLight(EdgeMode::Gradient,
                               rgba(saturate(config::Layout::DIALOG_EDGE_ALPHA
                                             + config::Layout::DIALOG_EDGE_FLARE_ALPHA * flare),
                                    detail::tintEdgeArgb(config::Render::EDGE_TINT_DIALOG)) & 0x00FFFFFFu,
                               config::Render::EDGE_ANGLE_DEFAULT,
                               config::Render::EDGE_FALLOFF_DEFAULT,
                               config::Render::EDGE_WIDTH_MIN,
                               config::Render::EDGE_SOFTNESS_MIN,
                               saturate(config::Layout::DIALOG_EDGE_ALPHA
                                        + config::Layout::DIALOG_EDGE_FLARE_ALPHA * flare));
        style.edgeOn = true;
        const float shadowArrival = curve(settle, config::Motion::DIALOG_SHADOW_FROM, config::Motion::DIALOG_SHADOW_TO)
                                  * (s.closing ? closePresence : 1.0f);
        style.shadowOn = shadowArrival > config::Motion::DIALOG_SHADOW_ARRIVAL_MIN;
        style.shadow = DropShadow(config::Layout::DIALOG_SHADOW_DP, 0.0f,
                                  config::Layout::DIALOG_SHADOW_DP / 6.0f,
                                  rgba(config::Layout::DIALOG_SHADOW_ALPHA * shadowArrival,
                                       config::Render::SHADOW_TINT_DIALOG), 1.0f);
        style.surface = detail::tintSheet();
        style.surface.a = detail::tintSheetAlpha() * surfaceStage
                        * config::Layout::DIALOG_SHEET_ALPHA_SCALE;
        style.surfaceMode = SurfaceMode::Overlay;
        style.alpha = material;

        const int panel = submitPanel(style, dialog);
        if (panel >= 0) {
            detail::panelCapture(panel);
            detail::panelPaint(panel);
        }
    }

    if (s.closing && dispersal > config::Motion::DIALOG_DISPERSAL_MIN) {
        const unsigned int disperseRgb = detail::tintSpectrumRgb();
        const float ringFade = closePresence * dispersal;
        for (int i = 0; i < config::Layout::DIALOG_RING_COUNT; ++i) {
            const float band = (float)(i + 1);
            const float expand = px(config::Layout::DIALOG_RING_BASE_DP
                                    + config::Layout::DIALOG_RING_STEP_DP * band) * dispersal;
            const float bandAlpha = ringFade
                * (config::Layout::DIALOG_RING_ALPHA
                   - config::Layout::DIALOG_RING_ALPHA_DECAY * (float)i);
            detail::strokeRound(Box(dialog.left - expand, dialog.top - expand,
                                    dialog.right + expand, dialog.bottom + expand),
                                radius + expand, px(config::Layout::DIALOG_RING_WIDTH_DP),
                                rgba(bandAlpha, disperseRgb));
        }
        const float halo = px(config::Layout::DIALOG_HALO_DP) * dispersal;
        detail::strokeRound(Box(dialog.left - halo, dialog.top - halo,
                                dialog.right + halo, dialog.bottom + halo),
                            radius + halo, px(config::Layout::DIALOG_HALO_WIDTH_DP),
                            rgba(ringFade * config::Layout::DIALOG_HALO_ALPHA, disperseRgb));
    }

    const float titleExit = s.closing ? 1.0f - curve(closeP, config::Motion::DIALOG_TITLE_EXIT_FROM, config::Motion::DIALOG_TITLE_EXIT_TO) : 1.0f;
    const float bodyExit = s.closing ? 1.0f - curve(closeP, config::Motion::DIALOG_BODY_EXIT_FROM, config::Motion::DIALOG_BODY_EXIT_TO) : 1.0f;
    const float buttonExit = s.closing ? 1.0f - curve(closeP, config::Motion::DIALOG_BUTTON_EXIT_FROM, config::Motion::DIALOG_BUTTON_EXIT_TO) : 1.0f;
    const float titleAlpha = s.closing ? curve(progress, config::Motion::DIALOG_TITLE_OUT_FROM, config::Motion::DIALOG_TITLE_OUT_TO) * titleExit
                                       : curve(settle, config::Motion::DIALOG_TITLE_IN_FROM, config::Motion::DIALOG_TITLE_IN_TO);
    const float bodyAlpha = s.closing ? curve(progress, config::Motion::DIALOG_BODY_OUT_FROM, config::Motion::DIALOG_BODY_OUT_TO) * bodyExit
                                      : curve(settle, config::Motion::DIALOG_BODY_IN_FROM, config::Motion::DIALOG_BODY_IN_TO);
    const float buttonAlpha = s.closing ? curve(progress, config::Motion::DIALOG_BUTTON_OUT_FROM, config::Motion::DIALOG_BUTTON_OUT_TO) * buttonExit
                                        : curve(settle, config::Motion::DIALOG_BUTTON_IN_FROM, config::Motion::DIALOG_BUTTON_IN_TO);

    const float contentScale = s.closing ? visualScale : 1.0f;
    const Box content = s.closing ? dialog : glass;
    const float padX = px(config::Layout::DIALOG_PAD_X_DP) * contentScale;
    const float titleLift = s.closing
        ? 0.0f : (1.0f - titleAlpha) * px(config::Layout::DIALOG_SETTLE_SLIDE_DP);
    const float bodyLift = s.closing
        ? 0.0f : (1.0f - bodyAlpha) * px(config::Layout::DIALOG_SETTLE_SLIDE_DP);
    const float titleY = content.top + px(config::Layout::DIALOG_TITLE_Y_DP) * contentScale
                       + titleLift;
    const float bodyY = content.top + px(config::Layout::DIALOG_BODY_Y_DP) * contentScale
                      + bodyLift;
    const unsigned int textRgb = detail::tintTextRgb();

    detail::pushClipBox(dialog);
    if (title != nullptr && title[0] != 0) {
        detail::drawTextAt(Point(content.left + padX, titleY),
                           px(config::Layout::DIALOG_TITLE_SIZE_DP) * contentScale,
                           rgba(titleAlpha, textRgb), title, 0.0f);
    }
    if (message != nullptr && message[0] != 0) {
        detail::drawTextAt(Point(content.left
                                     + px(config::Layout::DIALOG_BODY_PAD_X_DP) * contentScale,
                                 bodyY),
                           px(config::Layout::DIALOG_BODY_SIZE_DP) * contentScale,
                           rgba(config::Layout::DIALOG_BODY_ALPHA * bodyAlpha, textRgb),
                           message,
                           content.width()
                               - px(config::Layout::DIALOG_BODY_PAD_X_DP * 2.0f)
                               * contentScale);
    }
    detail::popClipBox();

    const float buttonGap = px(config::Layout::DIALOG_BUTTON_GAP_DP) * contentScale;
    const float buttonHeight = px(config::Layout::DIALOG_BUTTON_H_DP) * contentScale;
    const float buttonPad = px(config::Layout::DIALOG_BUTTON_PAD_DP) * contentScale;
    const float buttonWidth = (content.width() - buttonPad * 2.0f - buttonGap) * 0.5f;
    const float buttonY = content.bottom - buttonPad - buttonHeight;
    const Box cancelBox(content.left + buttonPad, buttonY,
                        content.left + buttonPad + buttonWidth, buttonY + buttonHeight);
    const Box confirmBox(cancelBox.right + buttonGap, buttonY,
                         cancelBox.right + buttonGap + buttonWidth, buttonY + buttonHeight);

    if (buttonAlpha > config::Render::CONTENT_ALPHA_MIN && buttonWidth > 1.0f) {
        const float cancelPress = s.cancel.glowProgress();
        s.cancel.label = cancelLabel;
        s.cancel.textSizeSp = config::Layout::DIALOG_BUTTON_SIZE_SP * contentScale;
        s.cancel.alphaScale = buttonAlpha;
        s.cancel.interactive = dialogInteractive;
        s.cancel.tinted = false;
        s.cancel.surfaced = true;
        s.cancel.surface = light
            ? Rgba(config::Layout::DIALOG_BUTTON_CANCEL_LIGHT_R,
                   config::Layout::DIALOG_BUTTON_CANCEL_LIGHT_G,
                   config::Layout::DIALOG_BUTTON_CANCEL_LIGHT_B,
                   config::Layout::DIALOG_BUTTON_CANCEL_LIGHT_A)
            : Rgba(config::Layout::DIALOG_BUTTON_CANCEL_DARK_R,
                   config::Layout::DIALOG_BUTTON_CANCEL_DARK_G,
                   config::Layout::DIALOG_BUTTON_CANCEL_DARK_B,
                   config::Layout::DIALOG_BUTTON_CANCEL_DARK_A);
        s.cancel.contourEdge = true;
        s.cancel.visualScale = lerp(config::Layout::DIALOG_BUTTON_SCALE_IDLE, 1.0f,
                                    curve(settle, config::Motion::DIALOG_BUTTON_SCALE_FROM, config::Motion::DIALOG_BUTTON_SCALE_TO));
        s.cancel.blurDp = lerp(config::Layout::DIALOG_BUTTON_BLUR_IDLE_DP,
                               config::Layout::DIALOG_BUTTON_BLUR_PRESS_DP, cancelPress);
        s.cancel.lensHeightDp = lerp(config::Layout::DIALOG_BUTTON_LENS_IDLE_DP,
                                     config::Layout::DIALOG_BUTTON_LENS_PRESS_DP, cancelPress);
        s.cancel.lensAmountDp = lerp(config::Layout::DIALOG_BUTTON_LENS_AMOUNT_IDLE_DP,
                                     config::Layout::DIALOG_BUTTON_LENS_AMOUNT_PRESS_DP,
                                     cancelPress);
        s.cancel.spectral = lerp(config::Layout::DIALOG_BUTTON_SPECTRAL_IDLE,
                                 config::Layout::DIALOG_BUTTON_SPECTRAL_PRESS, cancelPress);

        const float confirmPress = s.confirm.glowProgress();
        s.confirm.label = confirmLabel;
        s.confirm.textSizeSp = config::Layout::DIALOG_BUTTON_SIZE_SP * contentScale;
        s.confirm.alphaScale = buttonAlpha;
        s.confirm.interactive = dialogInteractive;
        s.confirm.tinted = true;
        s.confirm.tint = Rgba(config::Layout::DIALOG_BUTTON_CONFIRM_R,
                              config::Layout::DIALOG_BUTTON_CONFIRM_G,
                              config::Layout::DIALOG_BUTTON_CONFIRM_B,
                              config::Layout::DIALOG_BUTTON_CONFIRM_A);
        s.confirm.surfaced = false;
        s.confirm.contourEdge = true;
        s.confirm.visualScale = lerp(config::Layout::DIALOG_BUTTON_SCALE_IDLE, 1.0f,
                                     curve(settle, config::Motion::DIALOG_BUTTON_SCALE_FROM, config::Motion::DIALOG_BUTTON_SCALE_TO));
        s.confirm.textColor = config::Render::DIALOG_CONFIRM_TEXT;
        s.confirm.blurDp = lerp(config::Layout::DIALOG_BUTTON_BLUR_IDLE_DP,
                                config::Layout::DIALOG_BUTTON_BLUR_PRESS_DP, confirmPress);
        s.confirm.lensHeightDp = lerp(config::Layout::DIALOG_BUTTON_LENS_IDLE_DP,
                                      config::Layout::DIALOG_BUTTON_LENS_PRESS_DP,
                                      confirmPress);
        s.confirm.lensAmountDp = lerp(config::Layout::DIALOG_BUTTON_LENS_AMOUNT_IDLE_DP,
                                      config::Layout::DIALOG_BUTTON_LENS_AMOUNT_PRESS_DP,
                                      confirmPress);
        s.confirm.spectral = lerp(config::Layout::DIALOG_BUTTON_SPECTRAL_IDLE,
                                  config::Layout::DIALOG_BUTTON_SPECTRAL_PRESS, confirmPress);

        const bool cancelHit = s.cancel.draw(cancelBox);
        const bool confirmHit = s.confirm.draw(confirmBox);
        if (dialogInteractive) {
            if (cancelHit) close(1);
            if (confirmHit) close(2);
        }
    }
    return config::Motion::DIALOG_NO_ACTION;
}

}
