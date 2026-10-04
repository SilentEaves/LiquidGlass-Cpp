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

struct PopMenu::State {
    detail::Spring unfold{0.0f, config::Motion::MENU_SPRING_DAMPING,
                          config::Motion::MENU_UNFOLD_STIFFNESS,
                          config::Motion::MENU_SPRING_VISIBLE};
    detail::Spring widen{0.0f, config::Motion::MENU_SPRING_DAMPING,
                         config::Motion::MENU_WIDEN_STIFFNESS,
                         config::Motion::MENU_SPRING_VISIBLE};
    detail::Spring deepen{0.0f, config::Motion::MENU_SPRING_DAMPING,
                          config::Motion::MENU_DEEPEN_STIFFNESS,
                          config::Motion::MENU_SPRING_VISIBLE};
    detail::Spring greet{0.0f, config::Motion::MENU_SPRING_DAMPING,
                         config::Motion::MENU_GREET_STIFFNESS,
                         config::Motion::MENU_SPRING_VISIBLE};

    detail::SpineFrame frame;
    PillButton cap;

    bool settled = true;
    bool wasOpen = false;
    float flow = 0.0f;

    bool held = false;
    int hover = -1;
    bool hoverArmed = false;
    Point armFrom;

    float squeeze = 0.0f;
    bool bandOn = false;
    float bandAlpha = 0.0f;
    float bandY = 0.0f;
    float bandH = 0.0f;
    float bandFromY = 0.0f;
    float bandFromH = 0.0f;
    float bandToY = 0.0f;
    float bandToH = 0.0f;
    float bandT = 1.0f;

    State() {
        cap.shadowOn = false;
        cap.contourEdge = true;
        cap.blurDp = config::Layout::BUTTON_BLUR_DP;
    }
};

PopMenu::PopMenu() : state_(new State()) {
    State& s = state();
    detail::snapSpine(s.unfold, s.widen, s.deepen, s.greet, 0.0f);
    s.settled = true;
    s.wasOpen = false;
    s.flow = 0.0f;
}

PopMenu::~PopMenu() { delete state_; }

PopMenu::State& PopMenu::state() { return *state_; }

const PopMenu::State& PopMenu::state() const { return *state_; }

bool PopMenu::isOpen() const { return state().unfold.value > config::Render::GEOMETRY_MIN; }

void PopMenu::setOpen(bool value, bool withMotion) {
    State& s = state();
    if (value) {
        expanded = true;
        s.hover = -1;
        s.bandOn = false;
        s.hoverArmed = false;
        const PointerState& arm = detail::pointerRef();
        s.armFrom = Point(arm.x, arm.y);
        s.bandAlpha = 0.0f;
        s.wasOpen = true;
        s.settled = false;
        if (withMotion) {
            s.flow = 1.0f;
            s.unfold.animateTo(1.0f);
            s.widen.animateTo(1.0f);
            s.deepen.animateTo(1.0f);
        }
        else {
            s.unfold.snapTo(1.0f);
            s.widen.snapTo(1.0f);
            s.deepen.snapTo(1.0f);
            s.greet.snapTo(0.0f);
            s.flow = 0.0f;
        }
    }
    else {
        expanded = false;
        s.hover = -1;
        s.wasOpen = false;
        if (withMotion) s.flow = 1.0f;
        if (!withMotion
                || (fabsf(s.unfold.value) <= config::Motion::MENU_SPRING_VISIBLE
                    && fabsf(s.widen.value) <= config::Motion::MENU_SPRING_VISIBLE
                    && fabsf(s.deepen.value) <= config::Motion::MENU_SPRING_VISIBLE)) {
            detail::snapSpine(s.unfold, s.widen, s.deepen, s.greet, 0.0f);
            s.settled = true;
        }
        else {
            s.unfold.animateTo(0.0f);
            s.widen.animateTo(0.0f);
            s.deepen.animateTo(0.0f);
            s.greet.snapTo(0.0f);
        }
    }
}

namespace {

detail::SpineTune tuneOf(const PopMenu& menu) {
    detail::SpineTune tune;
    tune.drainStart = menu.drainStart;
    tune.drainSpan = menu.drainSpan;
    tune.stretchGain = menu.stretchGain;
    tune.gapDp = menu.gapDp;
    tune.greetDp = menu.greetDp;
    tune.threadDp = menu.threadDp;
    tune.jelly = menu.jelly;
    return tune;
}

PanelStyle shellStyle(const PopMenu& menu, float radius, float material, float flare,
                      float flow, float greet) {
    const float clampedMaterial = saturate(material);
    const float clampedFlare = menu.materialFlare ? saturate(flare) : 0.0f;
    const float clampedFlow = saturate(flow);
    const float clampedGreet = saturate(greet);

    PanelStyle style;
    style.corners = Corners::rounded(radius);
    style.optics.colorBoost = true;
    const float gate = detail::smoothStep01(clampedMaterial
                                            / config::Layout::MENU_FLOW_GATE_SPAN);
    const float grade = detail::curveHold(clampedMaterial);
    style.optics.blurPx = px(lerp(config::Layout::MENU_CAP_BLUR_DP,
                                  config::Layout::MENU_BLUR_HI_DP, grade)
                             * (1.0f - clampedFlow * gate));
    style.optics.lensHeightPx = px(lerp(config::Layout::MENU_CAP_LENS_HEIGHT_DP,
                                        config::Layout::MENU_LENS_HI_DP, grade)
                                   + config::Layout::MENU_FLARE_LENS_HEIGHT_DP
                                     * clampedFlare);
    style.optics.lensAmountPx = px(lerp(config::Layout::MENU_CAP_LENS_AMOUNT_DP,
                                        config::Layout::MENU_LENS_AMOUNT_HI_DP, grade)
                                   + config::Layout::MENU_FLARE_LENS_AMOUNT_DP
                                     * clampedFlare);
    style.optics.depthAmount = grade;
    style.optics.spectral = lerp(config::Layout::MENU_CAP_SPECTRAL,
                                 config::Layout::MENU_SPECTRAL_HI, grade)
                          + config::Layout::MENU_FLARE_SPECTRAL * clampedFlare;
    const Rgba capSheet = detail::tintButtonSheet();
    const Rgba fullSheet = detail::tintSheet();
    style.surface = Rgba(lerp(capSheet.r, fullSheet.r, grade),
                         lerp(capSheet.g, fullSheet.g, grade),
                         lerp(capSheet.b, fullSheet.b, grade),
                         lerp(capSheet.a, detail::tintSheetAlpha(), grade));
    style.surfaceMode = SurfaceMode::Overlay;
    style.alpha = 1.0f;

    const float rimAlpha = saturate(menu.rimAlpha
                                    * (lerp(config::Layout::MENU_CAP_EDGE_ALPHA,
                                            config::Layout::MENU_RIM_ALPHA, grade)
                                       + config::Layout::MENU_RIM_GREET_BOOST * clampedGreet
                                       + config::Layout::MENU_RIM_FLARE_BOOST * clampedFlare));
    style.edgeOn = rimAlpha > config::Motion::MENU_RIM_ALPHA_MIN;
    style.edge = EdgeLight::contour(config::Layout::MENU_RIM_ANGLE_DEGREES,
                                    config::Layout::MENU_RIM_FALLOFF,
                                    detail::tintEdgeArgb(config::Render::EDGE_TINT_CONTOUR));
    style.edge.widthDp = lerp(config::Layout::MENU_CAP_EDGE_WIDTH_DP,
                              config::Layout::MENU_RIM_WIDTH_DP, grade) * menu.rimDp;
    style.edge.softnessDp = lerp(config::Layout::MENU_CAP_EDGE_SOFTNESS_DP,
                                 config::Layout::MENU_RIM_SOFTNESS_DP, grade) * menu.rimDp;
    style.edge.alpha = rimAlpha;
    style.shadowOn = false;
    style.insetOn = false;
    return style;
}

void drawCapText(const char* text, float basePx, unsigned int color, float fade, float drain,
                 float greet, const Point& center) {
    if (text == nullptr || text[0] == 0 || fade <= config::Motion::MENU_CAP_TEXT_MIN) return;
    const float size = basePx * (1.0f - config::Layout::MENU_CAP_TEXT_DRAIN * drain)
                     * (1.0f - config::Layout::MENU_CAP_TEXT_GREET * greet);
    const unsigned int rgb = color != 0u
        ? (color & 0x00FFFFFFu) : detail::tintTextRgb();
    detail::drawTextCentered(center, size, rgba(fade, rgb), text);
}

void drawBand(const Box& drop, const Point& dropMid, float inner, float bandY, float bandH,
              float bandRadius, float bandAlpha, unsigned int bandColor, float veil) {
    if (bandAlpha <= config::Motion::MENU_BAND_ALPHA_MIN) return;
    const float sidePad = px(config::Layout::MENU_SIDE_PAD_DP);
    const Point from = detail::zoomAbout(dropMid,
        Point(drop.left + sidePad, drop.top + bandY), inner);
    const Point to = detail::zoomAbout(dropMid,
        Point(drop.right - sidePad, drop.top + bandY + bandH), inner);
    const float radius = fminf(bandRadius, (to.y - from.y) * 0.5f);
    const float baseAlpha = (float)((bandColor >> 24) & 0xFFu) / config::COLOR_CHANNEL;
    const Box band(from, to);
    detail::fillRound(band, radius,
                      rgba(baseAlpha * veil * bandAlpha, bandColor & 0x00FFFFFFu));
    detail::strokeRound(band, radius, fmaxf(px(config::HALF), 1.0f),
                        rgba(config::Layout::MENU_BAND_EDGE_ALPHA * veil,
                             detail::tintTextRgb()));
}

}

bool PopMenu::draw(const Box& capBox) {
    State& s = state();
    const float dt = fminf(detail::frameDelta(), config::Motion::MENU_DELTA_MAX);
    s.unfold.step(dt);
    s.widen.step(dt);
    s.deepen.step(dt);
    s.greet.step(dt);
    activated = -1;

    const int rowCount = count < 0 ? 0 : (count > MENU_ENTRY_LIMIT ? MENU_ENTRY_LIMIT : count);
    if (rowCount > 0) {
        cursorRow = cursorRow < 0 ? 0 : (cursorRow > rowCount - 1 ? rowCount - 1 : cursorRow);
    }
    detail::tickSpine(s.unfold, s.widen, s.deepen, s.greet, s.flow, s.settled, s.wasOpen,
                      expanded, rowCount, dt);

    detail::SpineFrame& frame = s.frame;
    frame.capX = capBox.left;
    frame.capY = capBox.top;
    frame.capW = capBox.width() > 1.0f ? capBox.width() : px(config::Layout::MENU_CAP_W_DP);
    frame.capH = capBox.height() > 1.0f ? capBox.height()
                                        : px(config::Layout::MENU_SLOT_DP);
    frame.capRadius = frame.capH * 0.5f;
    frame.padY = px(config::Layout::MENU_PAD_Y_DP);
    frame.rowGap = px(config::Layout::MENU_GAP_DP);
    frame.sidePad = px(config::Layout::MENU_SIDE_PAD_DP);
    frame.textPadX = px(config::Layout::MENU_TEXT_PAD_X_DP);
    frame.capTextPx = px(capTextSp);
    frame.captionPx = px(config::Layout::MENU_CAPTION_DP);
    frame.detailPx = px(config::Layout::MENU_DETAIL_DP);
    frame.markPx = px(config::Layout::MENU_MARK_DP);
    frame.headingPx = px(config::Layout::MENU_HEADING_DP);
    frame.shellRadius = px(cornerDp);
    frame.bandRadius = px(bandCornerDp);
    frame.rows = rowCount;

    Box host = detail::hostBox();
    if (host.width() <= 0.0f || host.height() <= 0.0f) host = detail::viewBounds();
    const float hostWidth = host.width();
    const float hostHeight = host.height();
    const float sideMargin = px(config::Layout::MENU_SIDE_MARGIN_DP);
    const float minWidth = px(config::Layout::MENU_MIN_W_DP);
    const float minHeight = px(config::Layout::MENU_MIN_H_DP);

    frame.shellW = fminf(px(widthDp), fmaxf(hostWidth - sideMargin, minWidth));
    float stack = frame.padY;
    for (int i = 0; i < rowCount; ++i) {
        const MenuEntry& entry = entries[i];
        float rowHeight;
        if (entry.kind == EntryKind::Divider) {
            rowHeight = px(config::Layout::MENU_RULE_H_DP);
        } else if (entry.kind == EntryKind::Heading) {
            rowHeight = px(entry.heightDp > 1.0f ? entry.heightDp
                                                 : config::Layout::MENU_HEADING_H_DP);
        }
        else {
            float text = frame.captionPx * config::Layout::MENU_TEXT_LINE;
            if (entry.detail != nullptr) {
                text += frame.detailPx * config::Layout::MENU_TEXT_LINE;
            }
            rowHeight = fmaxf(px(entry.heightDp > 1.0f ? entry.heightDp
                                                       : config::Layout::MENU_SLOT_DP),
                              text + px(config::Layout::MENU_TEXT_PAD_Y_DP * 2.0f));
        }
        frame.rowHeight[i] = rowHeight;
        frame.rowTop[i] = stack;
        stack += rowHeight + frame.rowGap;
    }
    if (rowCount > 0) stack -= frame.rowGap;
    frame.shellH = fminf(stack + frame.padY, fmaxf(hostHeight - sideMargin, minHeight));

    const detail::SpinePlacement placement = detail::placeSpine(frame, frame.shellW,
                                                                frame.shellH, reach,
                                                                clampToHost);
    frame.shiftX = placement.shiftX;
    frame.shiftY = placement.shiftY;
    frame.fitX = placement.fitX;
    frame.fitY = placement.fitY;
    frame.valid = true;
    frame.stamp = detail::frameStamp();

    if (interactive && expanded && s.unfold.value > config::Motion::SHELL_DISMISS_AT) {
        const detail::NeckSpine shield = detail::evaluateSpine(frame, s.unfold, s.widen,
                                                               s.deepen, s.greet,
                                                               tuneOf(*this));
        const float pad = px(config::Layout::MENU_REACH_PAD_DP);
        detail::blockPointer(Box(shield.dropCenter.x - shield.dropHalf.x - pad,
                                 shield.dropCenter.y - shield.dropHalf.y - pad,
                                 shield.dropCenter.x + shield.dropHalf.x + pad,
                                 shield.dropCenter.y + shield.dropHalf.y + pad));
    }

    const bool capAbsorbed = rowCount > 0 && (!s.settled || expanded);
    s.cap.label = capText;
    s.cap.textSizeSp = capTextSp;
    s.cap.textColor = capTextColor;
    s.cap.interactive = interactive && s.unfold.value <= config::Motion::SHELL_LIVE_AT
                     && rowCount > 0;
    s.cap.alphaScale = capAbsorbed ? 0.0f : 1.0f;
    s.cap.visualScale = 1.0f;
    s.cap.tinted = false;
    s.cap.surfaced = false;
    s.cap.glassOn = !capAbsorbed;
    s.cap.textOn = !capAbsorbed;

    const bool capHit = s.cap.draw(capBox);
    if (capHit) {
        if (expanded || s.unfold.value > config::Motion::MENU_CAP_TOGGLE_AT) {
            setOpen(false, true);
        }
        else {
            setOpen(true, true);
        }
    }
    return capHit;
}

void PopMenu::drawOverlay() {
    State& s = state();
    detail::SpineFrame& frame = s.frame;
    if (!frame.valid || frame.stamp != detail::frameStamp()) return;

    const float dt = fminf(detail::frameDelta(), config::Motion::MENU_DELTA_MAX);
    const int rowCount = frame.rows;

    if ((s.settled && !expanded) || rowCount <= 0) {
        s.held = false;
        s.hover = -1;
        s.bandOn = false;
        if (rowCount <= 0) s.bandAlpha = 0.0f;
    }
    else {

        const detail::NeckSpine spine = detail::evaluateSpine(frame, s.unfold, s.widen,
                                                              s.deepen, s.greet,
                                                              tuneOf(*this));
        const float swing = saturate(spine.swing);
        const float size = spine.size;
        const Box body = spine.body;
        const Box drop(spine.dropCenter.x - spine.dropHalf.x,
                       spine.dropCenter.y - spine.dropHalf.y,
                       spine.dropCenter.x + spine.dropHalf.x,
                       spine.dropCenter.y + spine.dropHalf.y);
        const Point dropMid = drop.center();
        const float dropWidth = spine.dropHalf.x * 2.0f;
        const float dropHeight = spine.dropHalf.y * 2.0f;
        const float veil = saturate((size - config::Motion::SHELL_VEIL_AT)
                                    / config::Motion::SHELL_VEIL_SPAN);
        const float inner = detail::spineInner(size, config::Motion::SHELL_INNER_BASE);

        const PointerState& pointer = detail::pointerRef();
        const float reachPad = px(config::Layout::MENU_REACH_PAD_DP);
        const Box reach = drop.grown(reachPad);
        const bool overBody = reach.holds(pointer.x, pointer.y);
        const bool live = interactive && expanded && swing > config::Motion::SHELL_LIVE_AT;
        const bool dismissible = interactive && expanded && closeOnOutside
                              && swing > config::Motion::SHELL_DISMISS_AT;

        if (pointer.pressed) {
            if (live && overBody) {
                s.held = true;
                s.hover = slotAt(Point(pointer.x, pointer.y));
            } else if (interactive && expanded && overBody
                       && swing > config::Motion::SHELL_DISMISS_AT) {
                s.hover = -1;
            } else if (dismissible && !overBody) {
                setOpen(false, true);
            }
        }

        if (s.held) {
            if (pointer.down) {
                const int row = slotAt(Point(pointer.x, pointer.y));
                if (row >= 0) s.hover = row;
                else if (!overBody) s.hover = -1;
            } else if (pointer.released) {
                int pick = slotAt(Point(pointer.x, pointer.y));
                if (pick < 0) pick = s.hover;
                const bool valid = pick >= 0 && pick < rowCount
                                && entries[pick].kind == EntryKind::Action
                                && entries[pick].selectable;
                if (valid) {
                    cursorRow = pick;
                    activated = pick;
                    if (!entries[pick].keepOpen) setOpen(false, true);
                }
                s.held = false;
                s.hover = -1;
            }
        } else if (live) {
            if (!s.hoverArmed) {
                const float dx = pointer.x - s.armFrom.x;
                const float dy = pointer.y - s.armFrom.y;
                const float slop = px(config::Motion::SHELL_ARM_SLOP_DP);
                if (dx * dx + dy * dy >= slop * slop) s.hoverArmed = true;
            }
            if (s.hoverArmed && overBody) {
                const int row = slotAt(Point(pointer.x, pointer.y));
                if (row >= 0) s.hover = row;
            }
            else {
                s.hover = -1;
            }
        }
        if (!expanded) {
            s.held = false;
            s.hover = -1;
        }

        if (detail::spineVisible(spine)) {
            PanelStyle style = shellStyle(*this, spine.dropRound, size, spine.flare, s.flow,
                                          spine.greet);
            detail::fillLiquid(style, spine, body);
            const int panel = submitPanel(style, body);
            if (panel >= 0) {
                detail::panelCapture(panel);
                detail::panelPaint(panel);
            }
        }

        drawCapText(capText, frame.capTextPx, capTextColor, 1.0f - veil, spine.drain, spine.greet,
                    Point(spine.feedCenter.x,
                          spine.feedCenter.y - px(config::Layout::MENU_CAP_TEXT_LIFT_DP) * veil));

        if (size <= config::Motion::SHELL_VEIL_AT || dropWidth <= 2.0f || dropHeight <= 2.0f) {
            s.bandOn = false;
            s.bandAlpha = 0.0f;
        }
        else {

            const bool hasHover = s.hover >= 0 && s.hover < rowCount;
            const int bandRow = hasHover ? s.hover : -1;
            if (bandRow >= 0) {
                if (!s.bandOn) {
                    s.bandY = s.bandFromY = s.bandToY = frame.rowTop[bandRow];
                    s.bandH = s.bandFromH = s.bandToH = frame.rowHeight[bandRow];
                    s.bandT = 1.0f;
                    s.bandOn = true;
                } else if (fabsf(frame.rowTop[bandRow] - s.bandToY) > config::Motion::MENU_BAND_MOVE_MIN
                        || fabsf(frame.rowHeight[bandRow] - s.bandToH) > config::Motion::MENU_BAND_MOVE_MIN) {
                    s.bandFromY = s.bandY;
                    s.bandFromH = s.bandH;
                    s.bandToY = frame.rowTop[bandRow];
                    s.bandToH = frame.rowHeight[bandRow];
                    s.bandT = 0.0f;
                }
            }
            else {
                s.bandOn = false;
            }
            if (s.bandOn && s.bandT < 1.0f) {
                s.bandT = fminf(1.0f, s.bandT + dt / config::Motion::SHELL_BAND_SECONDS);
                const float eased = detail::curveAt(s.bandT);
                s.bandY = lerp(s.bandFromY, s.bandToY, eased);
                s.bandH = lerp(s.bandFromH, s.bandToH, eased);
            }
            const float wantBand = s.bandOn ? (hasHover ? 1.0f : config::Layout::MENU_BAND_IDLE) : 0.0f;
            const float bandStep = dt / config::Motion::SHELL_BAND_FADE_SECONDS;
            s.bandAlpha += clampRange(wantBand - s.bandAlpha, -bandStep, bandStep);

            const float pressStep = dt / config::Motion::SHELL_PRESS_SECONDS;
            const float wantSqueeze = (s.held && s.hover >= 0) ? 1.0f : 0.0f;
            s.squeeze += clampRange(wantSqueeze - s.squeeze, -pressStep, pressStep);
            const float slotZoom = 1.0f - config::Motion::SHELL_SQUASH * s.squeeze;

            const unsigned int foreRgb = detail::tintTextRgb();
            const unsigned int alertRgb = config::Render::ALERT;

            const float bandGrow = (1.0f - config::Motion::SHELL_BAND_STRETCH * s.squeeze)
                                 * (1.0f - config::Motion::SHELL_BAND_POP * (1.0f - s.bandAlpha));
            const float bandDrawH = s.bandH * bandGrow;
            const float bandDrawY = s.bandY - (bandDrawH - s.bandH) * 0.5f;

            detail::pushClipBox(drop);
            drawBand(drop, dropMid, inner, bandDrawY, bandDrawH, frame.bandRadius,
                     s.bandAlpha * (1.0f + config::Motion::SHELL_BAND_PRESS * s.squeeze), bandColor,
                     veil);

            char heading[config::Layout::MENU_HEADING_LIMIT];
            for (int i = 0; i < rowCount; ++i) {
                const MenuEntry& entry = entries[i];
                const float top = frame.rowTop[i];
                const float rowHeight = frame.rowHeight[i];
                const float rowMidY = drop.top + top + rowHeight * 0.5f;
                const Point rowMid(drop.left + dropWidth * 0.5f, rowMidY);
                const float rowSpan = rowCount > 1 ? (float)i / (float)(rowCount - 1) : 0.0f;
                const float rowVeil = saturate((size
                    - (config::Motion::SHELL_VEIL_AT
                       + rowSpan * config::Motion::SHELL_ROW_STAGGER))
                    / config::Motion::SHELL_ROW_SPAN);
                const float subVeil = saturate((size
                    - (config::Motion::SHELL_VEIL_AT
                       + rowSpan * config::Motion::SHELL_ROW_STAGGER
                       + config::Motion::SHELL_SUB_LAG))
                    / (config::Motion::SHELL_ROW_SPAN - config::Motion::SHELL_SUB_LAG));
                const float rowEase = detail::smoothStep01(rowVeil);
                const float settledY = rowMidY
                    + (1.0f - rowEase) * px(config::Motion::SHELL_ROW_SLIDE_DP);

                if (entry.kind == EntryKind::Divider) {
                    const float inset = px(config::Layout::MENU_RULE_INSET_DP);
                    detail::strokeSegment(
                        detail::zoomAbout(dropMid, Point(drop.left + inset, settledY), inner),
                        detail::zoomAbout(dropMid, Point(drop.right - inset, settledY), inner),
                        rgba(rowVeil * config::Layout::MENU_RULE_ALPHA, foreRgb),
                        fmaxf(px(config::Layout::MENU_RULE_MIN_DP), 1.0f));
                }
                else {

                    const float zoom = (s.held && s.hover == i) ? slotZoom : 1.0f;
                    const bool picked = (i == cursorRow) && !hasHover && entry.selectable
                                     && entry.kind == EntryKind::Action;
                    const float hot = (hasHover && s.hover == i) ? s.bandAlpha : 0.0f;
                    const float rowAlpha = rowVeil * (entry.selectable ? 1.0f
                                                                    : config::Layout::MENU_DISABLED_ALPHA);
                    const float subAlpha = saturate(subVeil * (entry.selectable ? 1.0f
                                                                            : config::Layout::MENU_DISABLED_ALPHA)
                                                    + hot * config::Motion::SHELL_SUB_HOT);
                    const unsigned int rgb = entry.danger ? alertRgb : foreRgb;

                    if (entry.kind == EntryKind::Heading) {
                        detail::upperAscii(entry.caption, heading, config::Layout::MENU_HEADING_LIMIT);
                        const float size = frame.headingPx * inner;
                        detail::drawTextAt(
                            detail::zoomAbout(dropMid,
                                              Point(drop.left + frame.textPadX,
                                                    settledY - detail::textMetrics(heading, size).height
                                                        * 0.5f),
                                              inner),
                            size, rgba(rowVeil * config::Layout::MENU_HEADING_ALPHA, foreRgb), heading,
                            0.0f);
                    }
                    else {

                        const float placeX = frame.sidePad + frame.textPadX;
                        float cursorX = placeX;
                        if (entry.texture != 0u || entry.painter != nullptr) {
                            const float pop = 1.0f - config::Motion::SHELL_MARK_POP * (1.0f - rowEase);
                            const float markPx = frame.markPx * inner * pop;
                            const Point markCenter = detail::zoomAbout(dropMid,
                                Point(drop.left + cursorX + frame.markPx * 0.5f, settledY), inner);
                            const Point placed = detail::zoomAbout(rowMid, markCenter, zoom);
                            const unsigned int markColor = rgba(rowAlpha * (entry.selectable ? 1.0f : 0.5f), rgb);
                            if (entry.texture != 0u) {
                                detail::drawTexture(entry.texture,
                                                    Box(placed.x - markPx * 0.5f, placed.y - markPx * 0.5f,
                                                        placed.x + markPx * 0.5f, placed.y + markPx * 0.5f),
                                                    markColor);
                            }
                            else {
                                detail::paintIcon(entry.painter, placed, markPx, markColor, entry.tag);
                            }
                            cursorX += frame.markPx + px(config::Layout::MENU_MARK_GAP_DP);
                        }

                        const float captionPx = frame.capTextPx * inner;
                        const char* caption = entry.caption != nullptr ? entry.caption : "";
                        const TextSize captionSize = detail::textMetrics(caption, captionPx);
                        const bool hasDetail = entry.detail != nullptr && entry.detail[0] != 0;
                        const float detailPx = frame.detailPx * inner;
                        const TextSize detailSize = hasDetail ? detail::textMetrics(entry.detail, detailPx)
                                                              : TextSize();
                        const float blockHeight = captionSize.height + (hasDetail ? detailSize.height : 0.0f);
                        const float textTop = settledY - blockHeight * 0.5f;
                        const float captionAlpha = saturate(rowAlpha
                            * (picked ? 1.0f : config::Layout::MENU_IDLE_ALPHA)
                            + hot * config::Layout::MENU_HOT_ALPHA);

                        detail::drawTextAt(
                            detail::zoomAbout(rowMid,
                                      detail::zoomAbout(dropMid, Point(drop.left + cursorX, textTop), inner), zoom),
                            captionPx, rgba(captionAlpha, rgb), caption, 0.0f);
                        if (hasDetail) {
                            detail::drawTextAt(
                                detail::zoomAbout(rowMid,
                                          detail::zoomAbout(dropMid,
                                                    Point(drop.left + cursorX, textTop + captionSize.height),
                                                    inner),
                                          zoom),
                                detailPx, rgba(subAlpha * config::Layout::MENU_SUB_ALPHA, rgb), entry.detail,
                                0.0f);
                        }
                        if (entry.shortcut != nullptr && entry.shortcut[0] != 0) {
                            const TextSize shortcutSize = detail::textMetrics(entry.shortcut, detailPx);
                            detail::drawTextAt(
                                detail::zoomAbout(rowMid,
                                          detail::zoomAbout(dropMid,
                                                    Point(drop.right - frame.sidePad - frame.textPadX
                                                              - shortcutSize.width, textTop),
                                                    inner),
                                          zoom),
                                detailPx, rgba(subAlpha * config::Layout::MENU_SUB_ALPHA, rgb),
                                entry.shortcut, 0.0f);
                        }
                    }
                }
            }
            detail::popClipBox();
        }
    }
}

int PopMenu::slotAt(const Point& pointer) const {
    const State& s = state();
    const detail::SpineFrame& frame = s.frame;
    if (!frame.valid || frame.rows <= 0) return -1;
    const detail::NeckSpine spine = detail::evaluateSpine(frame, s.unfold, s.widen, s.deepen,
                                                          s.greet, tuneOf(*this));
    float localY = 0.0f;
    const float inner = detail::spineInner(spine.size, config::Motion::SHELL_INNER_BASE);
    if (!detail::spineLocalY(spine, pointer, inner, localY)) return -1;
    return detail::pickRow(frame, entries, localY);
}

}
