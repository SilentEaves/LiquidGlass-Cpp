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
namespace detail {

void snapSpine(Spring& unfold, Spring& widen, Spring& deepen, Spring& greet, float value) {
    unfold.snapTo(value);
    widen.snapTo(value);
    deepen.snapTo(value);
    greet.snapTo(0.0f);
}

void tickSpine(Spring& unfold, Spring& widen, Spring& deepen, Spring& greet, float& flow,
               bool& settled, bool& wasOpen, bool expanded, int rows, float dt) {
    if (expanded != wasOpen) {
        if (expanded && rows > 0) {
            wasOpen = true;
            settled = false;
            flow = 1.0f;
            unfold.animateTo(1.0f);
            widen.animateTo(1.0f);
            deepen.animateTo(1.0f);
        }
        else {
            wasOpen = false;
            flow = 1.0f;
            unfold.animateTo(0.0f);
            widen.animateTo(0.0f);
            deepen.animateTo(0.0f);
            greet.snapTo(0.0f);
        }
    } else if (expanded && rows > 0 && unfold.target < config::Motion::SHELL_TARGET_NEAR) {
        unfold.animateTo(1.0f);
        widen.animateTo(1.0f);
        deepen.animateTo(1.0f);
    } else if (!expanded && unfold.target > config::Motion::SHELL_REST_AT) {
        wasOpen = false;
        flow = 1.0f;
        unfold.animateTo(0.0f);
        widen.animateTo(0.0f);
        deepen.animateTo(0.0f);
        greet.snapTo(0.0f);
    } else if (!expanded
            && fabsf(unfold.value) <= config::Motion::SHELL_REST_AT
            && fabsf(widen.value) <= config::Motion::SHELL_REST_AT
            && fabsf(deepen.value) <= config::Motion::SHELL_REST_AT
            && fabsf(unfold.velocity) < config::Motion::MENU_REST_VELOCITY
            && fabsf(widen.velocity) < config::Motion::MENU_REST_VELOCITY
            && fabsf(deepen.velocity) < config::Motion::MENU_REST_VELOCITY) {
        unfold.snapTo(0.0f);
        widen.snapTo(0.0f);
        deepen.snapTo(0.0f);
        settled = true;
    }

    const float t = saturate(1.0f - smoothStep01(deepen.value));
    const float rise = smoothStep01((t - config::Motion::SHELL_GREET_RISE_FROM)
                                      / config::Motion::SHELL_GREET_RISE_SPAN);
    const float fall = smoothStep01((t - config::Motion::SHELL_GREET_FALL_FROM)
                                      / config::Motion::SHELL_GREET_FALL_SPAN);
    greet.animateTo((!expanded && !settled) ? rise * (1.0f - fall) : 0.0f);

    const float speed = fabsf(unfold.velocity) + fabsf(widen.velocity)
                      + fabsf(deepen.velocity);
    const float now = saturate(speed / config::Motion::SHELL_FLOW_SPEED_REFERENCE);
    const float tau = (now > flow) ? config::Motion::SHELL_FLOW_RISE_SECONDS
                                   : config::Motion::SHELL_FLOW_FALL_SECONDS;
    flow = saturate(flow + (now - flow) * fminf(1.0f, dt / tau));
}

SpinePlacement placeSpine(const SpineFrame& frame, float shellW, float shellH, MenuReach reach,
                          bool clampToHost) {
    SpinePlacement placement;
    float axisX = -1.0f;
    float axisY = -1.0f;
    Box host = hostBox();
    if (host.width() <= 0.0f || host.height() <= 0.0f) host = viewBounds();
    const float hostWidth = host.width();
    const float hostHeight = host.height();

    if (reach == MenuReach::Auto) {
        const bool rightHalf = (frame.capX - host.left) > hostWidth * 0.5f;
        const float below = (host.top + hostHeight) - (frame.capY + frame.capH);
        const float above = frame.capY - host.top;
        axisY = (below < shellH && above > shellH) ? 1.0f : -1.0f;
        axisX = rightHalf ? 1.0f : -1.0f;
    } else if (reach == MenuReach::DownLeft) {
        axisX = 1.0f;
        axisY = -1.0f;
    } else if (reach == MenuReach::UpRight) {
        axisX = -1.0f;
        axisY = 1.0f;
    } else if (reach == MenuReach::UpLeft) {
        axisX = 1.0f;
        axisY = 1.0f;
    }
    placement.shiftX = -axisX * (shellW - frame.capW) * 0.5f;
    placement.shiftY = -axisY * (shellH - frame.capH) * 0.5f;
    if (!clampToHost) return placement;

    const float anchorX = frame.capX + (1.0f + axisX) * frame.capW * 0.5f;
    const float left = anchorX - (1.0f + axisX) * shellW * 0.5f;
    const float top = (axisY < 0.0f) ? (frame.capY + frame.capH) : (frame.capY - shellH);
    if (left < host.left) placement.fitX = host.left - left;
    else if (left + shellW > host.left + hostWidth) {
        placement.fitX = (host.left + hostWidth) - (left + shellW);
    }
    if (top < host.top) placement.fitY = host.top - top;
    else if (top + shellH > host.top + hostHeight) {
        placement.fitY = (host.top + hostHeight) - (top + shellH);
    }
    return placement;
}

NeckSpine evaluateSpine(const SpineFrame& frame, const Spring& unfold, const Spring& widen,
                        const Spring& deepen, const Spring& greet, const SpineTune& tune) {
    NeckSpine spine;
    spine.swing = unfold.value;
    spine.spread = saturate(widen.value);
    spine.size = smoothStep01(deepen.value);

    const float jelly = clampRange(tune.jelly, 0.0f, config::Layout::MENU_JELLY_MAX);
    const float span = tune.drainSpan > config::Layout::MENU_DRAIN_SPAN_MIN
        ? tune.drainSpan : config::Layout::MENU_DRAIN_SPAN_MIN;
    spine.drain = saturate((spine.size - tune.drainStart) / span);
    const float feedScale = 1.0f - spine.drain;
    spine.feedLive = feedScale > config::Layout::MENU_FEED_ALIVE;

    const float lead = saturate(fabsf(unfold.value - deepen.value));
    const float stretch = tune.stretchGain > 0.0f
        ? saturate(lead * config::Motion::SHELL_FLARE_GAIN * tune.stretchGain * jelly) : 0.0f;
    const float bloomGate = saturate(0.5f + unfold.velocity
                                            * config::Motion::SHELL_BLOOM_VELOCITY_GAIN);
    const float bloom = config::Motion::SHELL_SETTLE_FLARE * saturate(tune.stretchGain)
        * bloomGate
        * smoothStep01((unfold.value - config::Motion::SHELL_BLOOM_OPEN_FROM)
                           / config::Motion::SHELL_BLOOM_OPEN_SPAN)
        * (1.0f - smoothStep01((unfold.value - config::Motion::SHELL_BLOOM_CLOSE_FROM)
                       / config::Motion::SHELL_BLOOM_CLOSE_SPAN));
    spine.flare = saturate(stretch + bloom);

    const float capCenterX = frame.capX + frame.capW * 0.5f;
    const float capCenterY = frame.capY + frame.capH * 0.5f;
    const float side = (frame.shiftY >= 0.0f) ? 1.0f : -1.0f;

    const float greetAmount = saturate(greet.value);
    spine.greet = greetAmount;

    spine.dropHalf.x = fmaxf(frame.shellW * spine.spread, 1.0f) * 0.5f;
    spine.dropHalf.y = fmaxf(frame.shellH * spine.size, 1.0f) * 0.5f;
    spine.dropHalf.y *= 1.0f
        + (config::Motion::SHELL_STRETCH_ALONG * spine.flare
           + config::Motion::SHELL_GREET_STRETCH * greetAmount) * jelly;
    spine.dropHalf.x *= 1.0f
        - (config::Motion::SHELL_STRETCH_CROSS * spine.flare
           + config::Motion::SHELL_GREET_NARROW * greetAmount) * jelly;

    const float mouthY = capCenterY + side * frame.capH * 0.5f;
    const float settleY = capCenterY - side * frame.capH * 0.5f;
    const float eat = powf(spine.size, config::Motion::SHELL_SETTLE_CURVE);
    const float fling = clampRange(unfold.value - deepen.value, 0.0f, 1.0f);
    const float hang = px(tune.gapDp) * fling;
    const float dropNearY = lerp(mouthY, settleY, eat) + side * hang;
    const float fit = saturate(unfold.value);
    spine.dropCenter.x = capCenterX + frame.shiftX * spine.size + frame.fitX * fit;
    spine.dropCenter.y = dropNearY + side * spine.dropHalf.y + frame.fitY * fit;

    const float maxRound = fminf(spine.dropHalf.x, spine.dropHalf.y);
    float dropRound = lerp(fminf(frame.capRadius, maxRound),
                           fminf(frame.shellRadius, maxRound), smoothStep01(spine.size));
    dropRound = lerp(dropRound, maxRound * config::Motion::SHELL_DROPLET_MAX_RATIO,
                     spine.flare * config::Motion::SHELL_DROPLET_ROUND * jelly);
    spine.dropRound = fminf(dropRound, maxRound);

    spine.feedCenter.x = capCenterX;
    spine.feedCenter.y = capCenterY - px(tune.greetDp) * greetAmount;
    const float swallow = greetAmount * jelly;
    const float feedHalfY = frame.capH * 0.5f * feedScale
                          * (1.0f + config::Motion::SHELL_SQUASH * swallow);
    const float feedHalfX = frame.capW * 0.5f * feedScale
                          * (1.0f - config::Motion::SHELL_SQUASH_X * swallow);
    spine.feedHalf.x = feedHalfX;
    spine.feedHalf.y = feedHalfY;
    spine.feedRound = fminf(frame.capRadius * feedScale, fminf(feedHalfX, feedHalfY));

    const float dropNear = dropNearY + frame.fitY * fit;
    const float feedFarY = spine.feedCenter.y + side * feedHalfY;
    const float strand = (dropNear - feedFarY) * side;
    const float threadRadius = px(tune.threadDp)
                             * (1.0f - config::Motion::SHELL_THREAD_TAPER * spine.drain);
    const bool dropAlive = spine.size > config::Motion::SHELL_DROP_ALIVE;
    const float tailFade = (unfold.target < 0.5f)
        ? lerp(config::Motion::SHELL_TAIL_MIN, 1.0f,
               saturate(spine.size / config::Motion::SHELL_DROP_SPAN))
        : 1.0f;
    if (spine.feedLive && dropAlive && strand > config::Motion::SHELL_SHAPE_MIN_SPAN && threadRadius > config::Motion::SHELL_THREAD_MIN_RADIUS) {
        const float threadFade = saturate(strand
            / px(config::Motion::SHELL_THREAD_FADE_DP));
        spine.strandRadius = threadRadius;
        spine.strandHalf.x = threadRadius;
        spine.strandHalf.y = strand * 0.5f + threadRadius;
        spine.strandCenter.x = (spine.feedCenter.x + spine.dropCenter.x) * 0.5f;
        spine.strandCenter.y = feedFarY + side * strand * 0.5f;
        spine.strandMerge = threadRadius * config::Motion::SHELL_THREAD_BLEND_GAIN
                          * threadFade;
        spine.feedMerge = threadRadius * config::Motion::SHELL_FEED_BLEND_GAIN * tailFade;
    } else if (spine.feedLive) {
        spine.feedMerge = px(config::Motion::SHELL_FEED_TAIL_DP) * tailFade;
    }

    float lowX = spine.dropCenter.x - spine.dropHalf.x;
    float highX = spine.dropCenter.x + spine.dropHalf.x;
    float lowY = spine.dropCenter.y - spine.dropHalf.y;
    float highY = spine.dropCenter.y + spine.dropHalf.y;
    float room = 0.0f;
    if (spine.feedLive) {
        lowX = fminf(lowX, spine.feedCenter.x - spine.feedHalf.x);
        highX = fmaxf(highX, spine.feedCenter.x + spine.feedHalf.x);
        lowY = fminf(lowY, spine.feedCenter.y - spine.feedHalf.y);
        highY = fmaxf(highY, spine.feedCenter.y + spine.feedHalf.y);
        room = spine.feedMerge;
    }
    if (spine.strandMerge > 0.0f) {
        lowX = fminf(lowX, spine.strandCenter.x - spine.strandHalf.x);
        highX = fmaxf(highX, spine.strandCenter.x + spine.strandHalf.x);
        lowY = fminf(lowY, spine.strandCenter.y - spine.strandHalf.y);
        highY = fmaxf(highY, spine.strandCenter.y + spine.strandHalf.y);
        room = fmaxf(room, spine.strandMerge);
    }
    spine.body = Box(lowX - room, lowY - room, highX + room, highY + room);
    return spine;
}

bool spineVisible(const NeckSpine& spine) {
    const bool drop = spine.dropHalf.x > 0.5f && spine.dropHalf.y > 0.5f;
    const bool feed = spine.feedLive && spine.feedHalf.x > 0.5f && spine.feedHalf.y > 0.5f;
    return drop || feed;
}

bool spineLocalY(const NeckSpine& spine, const Point& p, float inner, float& localY) {
    const float pad = px(config::Layout::MENU_REACH_PAD_DP);
    const float reachX = spine.dropHalf.x + pad;
    const float reachY = spine.dropHalf.y + pad;
    const bool outsideX = fabsf(p.x - spine.dropCenter.x) > reachX;
    const bool outsideY = fabsf(p.y - spine.dropCenter.y) > reachY;
    if (outsideX || outsideY) return false;
    const float inverse = inner > config::Motion::SHELL_INNER_MIN ? 1.0f / inner : 1.0f;
    localY = (spine.dropCenter.y + (p.y - spine.dropCenter.y) * inverse)
           - (spine.dropCenter.y - spine.dropHalf.y);
    return !outsideX && !outsideY;
}

int pickRow(const SpineFrame& frame, const MenuEntry* entries, float localY) {
    int nearest = -1;
    float best = config::Motion::SHELL_PICK_FAR;
    for (int i = 0; i < frame.rows; ++i) {
        const float top = frame.rowTop[i];
        const float bottom = top + frame.rowHeight[i];
        const bool pickable = entries[i].kind == EntryKind::Action && entries[i].selectable;
        if (localY >= top && localY <= bottom) return pickable ? i : -1;
        if (!pickable) continue;
        const float distance = localY < top ? (top - localY) : (localY - bottom);
        if (distance < best) {
            best = distance;
            nearest = i;
        }
    }
    return (nearest >= 0 && best <= frame.rowGap) ? nearest : -1;
}

void fillLiquid(PanelStyle& style, const NeckSpine& spine, const Box& body) {
    style.liquid = true;
    style.dropCenter = Point(spine.dropCenter.x - body.left, spine.dropCenter.y - body.top);
    style.dropHalf = spine.dropHalf;
    style.dropRadius = spine.dropRound;
    style.feedCenter = Point(spine.feedCenter.x - body.left, spine.feedCenter.y - body.top);
    style.feedHalf = spine.feedHalf;
    style.feedRadius = spine.feedRound;
    style.feedBlend = spine.feedMerge;
    if (spine.strandMerge > 0.0f) {
        style.strandCenter = Point(spine.strandCenter.x - body.left,
                                   spine.strandCenter.y - body.top);
        style.strandHalf = spine.strandHalf;
        style.strandRadius = spine.strandRadius;
        style.strandBlend = spine.strandMerge;
    }
}

}
}
