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
namespace {

PanelJob g_jobs[config::Render::PANEL_JOB_LIMIT];
int g_jobCount = 0;

int g_panelJob[config::Render::PANEL_JOB_LIMIT] = {};
int g_panelShade[config::Render::PANEL_JOB_LIMIT] = {};
int g_panelCount = 0;

unsigned int g_backdropTexture = 0;
int g_backdropWidth = 0;
int g_backdropHeight = 0;

float alphaOf(unsigned int color) {
    return (float)((color >> 24) & 0xFFu) / config::COLOR_CHANNEL;
}

float channelOf(unsigned int color, int shift) {
    return (float)((color >> shift) & 0xFFu) / config::COLOR_CHANNEL;
}

bool backdropReady() {
    return backdropEnabledFlag() && g_backdropTexture != 0 && g_backdropWidth > 0
        && g_backdropHeight > 0;
}

float edgeModeValue(const EdgeLight& edge, bool enabled) {
    if (!enabled || edge.widthDp <= 0.0f) return 0.0f;
    if (edge.mode == EdgeMode::Plain) return 3.0f;
    if (edge.mode == EdgeMode::Ambient) return 2.0f;
    if (edge.mode == EdgeMode::Contour) return 4.0f;
    return 1.0f;
}

float blurCeiling() { return px(config::Render::BLUR_RADIUS_MAX_DP); }

float capturePadding(const PanelStyle& style, float uniformScale) {
    const Optics& optics = style.optics;
    const float amount = clampRange(optics.lensAmountPx, 0.0f, config::Render::LENS_REACH_MAX)
                       * uniformScale;
    const float blurCap = blurCeiling();
    const float blur = clampRange(clampRange(optics.blurPx, 0.0f, blurCap)
                                  * uniformScale, 0.0f, blurCap);
    const float spectral = clampRange(optics.spectral, 0.0f, config::Render::SPECTRAL_MAX);
    const float dispersionPad = spectral > 0.0f ? amount * spectral : 0.0f;
    const float blurGain = glGatherReady() ? config::Render::CAPTURE_PAD_BLUR_GAIN
                                           : config::Render::CAPTURE_PAD_BLUR_GAIN_SAFE;
    return clampRange(config::Render::CAPTURE_PAD_BASE + blur * blurGain + amount
                      + dispersionPad, config::Render::CAPTURE_PAD_BASE,
                      config::Render::CAPTURE_PAD_MAX);
}

void applyCorners(PanelJob& job, const Corners& corners, float boxWidth, float boxHeight,
                  float uniformScale) {
    const float shorter = boxWidth < boxHeight ? boxWidth : boxHeight;
    const float limit = shorter > 0.0f ? shorter * 0.5f : 0.0f;
    job.corners.topLeft = clampRange(corners.topLeft, 0.0f, limit) * uniformScale;
    job.corners.topRight = clampRange(corners.topRight, 0.0f, limit) * uniformScale;
    job.corners.bottomRight = clampRange(corners.bottomRight, 0.0f, limit) * uniformScale;
    job.corners.bottomLeft = clampRange(corners.bottomLeft, 0.0f, limit) * uniformScale;
}

void applyEdge(PanelJob& job, const PanelStyle& style, float uniformScale) {
    const EdgeLight& edge = style.edge;
    job.edgeMode = edgeModeValue(edge, style.edgeOn);
    if (job.edgeMode <= 0.0f) {
        job.edgeWidth = 0.0f;
        job.edgeBlur = 0.0f;
        job.edgeAlpha = 0.0f;
    }
    else {
        job.edgeTint = rgbaOf(edge.color);
        job.edgeTintAlpha = alphaOf(edge.color);
        const bool directed = edge.mode == EdgeMode::Gradient || edge.mode == EdgeMode::Contour;
        job.edgeAngle = (directed ? edge.angleDegrees : 45.0f) * config::DEGREES_TO_RADIANS;
        job.edgeFalloff = directed ? edge.falloff : 1.0f;
        job.edgeAlpha = edge.alpha * style.alpha;
        float widthPx = edge.widthDp * densityScale() * uniformScale;
        const float shorter = job.frame.width() < job.frame.height()
            ? job.frame.width() : job.frame.height();
        const float widthLimit = shorter * 0.5f;
        if (widthPx > widthLimit) widthPx = widthLimit;
        job.edgeWidth = ceilf(widthPx);
        job.edgeBlur = edge.softnessDp * densityScale() * uniformScale;
    }
}

void applyInset(PanelJob& job, const PanelStyle& style, float uniformScale) {
    if (!style.insetOn) {
        job.insetAlpha = 0.0f;
    }
    else {
        const InsetShadow& inset = style.inset;
        job.insetTint = rgbaOf(inset.color);
        job.insetShiftX = inset.offsetXDp * densityScale() * job.pixelScaleX;
        job.insetShiftY = inset.offsetYDp * densityScale() * job.pixelScaleY;
        job.insetReach = inset.radiusDp * densityScale() * uniformScale;
        job.insetAlpha = inset.alpha * style.alpha * alphaOf(inset.color);
    }
}

void applyLiquid(PanelJob& job, const PanelStyle& style, float uniformScale) {
    if (!style.liquid) {
        job.dropRect[2] = -1.0f;
        job.dropRect[3] = -1.0f;
        job.feedRect[2] = -1.0f;
        job.feedRect[3] = -1.0f;
        job.strandRect[2] = -1.0f;
        job.strandRect[3] = -1.0f;
    }
    else {
        job.dropRect[0] = style.dropCenter.x * job.pixelScaleX;
        job.dropRect[1] = style.dropCenter.y * job.pixelScaleY;
        job.dropRect[2] = style.dropHalf.x * uniformScale;
        job.dropRect[3] = style.dropHalf.y * uniformScale;
        job.dropRound = style.dropRadius * uniformScale;
        job.feedRect[0] = style.feedCenter.x * job.pixelScaleX;
        job.feedRect[1] = style.feedCenter.y * job.pixelScaleY;
        job.feedRect[2] = style.feedHalf.x * uniformScale;
        job.feedRect[3] = style.feedHalf.y * uniformScale;
        job.feedRound = style.feedRadius * uniformScale;
        job.feedMerge = style.feedBlend * uniformScale;
        job.strandRect[0] = style.strandCenter.x * job.pixelScaleX;
        job.strandRect[1] = style.strandCenter.y * job.pixelScaleY;
        job.strandRect[2] = style.strandHalf.x * uniformScale;
        job.strandRect[3] = style.strandHalf.y * uniformScale;
        job.strandRound = style.strandRadius * uniformScale;
        job.strandMerge = style.strandBlend * uniformScale;
    }
}

void applyTrack(PanelJob& job, const PanelStyle& style, float uniformScale) {
    if (!style.trackOn) {
        job.trackOn = 0.0f;
    }
    else {
        job.trackOn = 1.0f;
        job.trackRect[0] = style.trackBox.left * job.pixelScaleX - job.frame.left;
        job.trackRect[1] = style.trackBox.top * job.pixelScaleY - job.frame.top;
        job.trackRect[2] = style.trackBox.right * job.pixelScaleX - job.frame.left;
        job.trackRect[3] = style.trackBox.bottom * job.pixelScaleY - job.frame.top;
        job.trackRadius = style.trackRadius * uniformScale;
        job.trackTint = style.trackColor;
    }
}

void applyGlow(PanelJob& job, const PanelStyle& style) {
    if (!style.glow.on || style.glow.progress <= config::Render::GLOW_LEVEL_MIN) {
        job.glowOn = 0.0f;
    }
    else {
        job.glowOn = 1.0f;
        job.glowX = style.glow.x * job.pixelScaleX;
        job.glowY = style.glow.y * job.pixelScaleY;
        const float shorter = job.frame.width() < job.frame.height()
            ? job.frame.width() : job.frame.height();
        job.glowReach = shorter * config::Render::GLOW_REACH_SCALE;
        job.glowLevel = style.glow.progress;
    }
}

int appendJob(const PanelStyle& style, const Box& box, float pad) {
    if (g_jobCount >= config::Render::PANEL_JOB_LIMIT) return -1;
    const float scaleX = pixelScaleXOf();
    const float scaleY = pixelScaleYOf();
    const float left = box.left * scaleX;
    const float top = box.top * scaleY;
    const float right = box.right * scaleX;
    const float bottom = box.bottom * scaleY;
    if (right - left <= 0.0f || bottom - top <= 0.0f) return -1;

    const int jobIndex = g_jobCount;
    PanelJob& job = g_jobs[jobIndex];
    job = PanelJob();
    job.frame = Box(left, top, right, bottom);
    job.viewW = viewBounds().width() * scaleX;
    job.viewH = viewBounds().height() * scaleY;
    job.pixelScaleX = scaleX;
    job.pixelScaleY = scaleY;

    const float captureLeft = floorf(left - pad);
    const float captureTop = floorf(top - pad);
    job.sampleOriginX = captureLeft;
    job.sampleOriginY = captureTop;
    job.sampleSizeX = ceilf(right + pad) - captureLeft;
    job.sampleSizeY = ceilf(bottom + pad) - captureTop;
    job.sampleStrideX = job.sampleSizeX;
    job.sampleStrideY = job.sampleSizeY;

    const float uniformScale = uniformPixelScale();
    const float blurCap = blurCeiling();
    applyCorners(job, style.corners, box.width(), box.height(), uniformScale);

    const Optics& optics = style.optics;
    job.lensHeight = clampRange(optics.lensHeightPx, 0.0f, config::Render::LENS_REACH_MAX)
                   * uniformScale;
    job.lensAmount = clampRange(optics.lensAmountPx, 0.0f, config::Render::LENS_REACH_MAX)
                   * uniformScale;
    job.lensDepth = optics.depthAmount;
    job.darkGuard = optics.darkGuard * config::Render::DARK_GUARD_FLOOR;
    job.spectral = clampRange(optics.spectral, 0.0f, config::Render::SPECTRAL_MAX);
    job.blurRadius = clampRange(clampRange(optics.blurPx, 0.0f, blurCap)
                                * uniformScale, 0.0f, blurCap);
    job.panelOpacity = clampRange(optics.opacityOn ? optics.opacity : 1.0f, 0.0f,
                                  config::Render::PANEL_OPACITY_MAX);
    job.panelOpacity = clampRange(job.panelOpacity * clampRange(style.alpha, 0.0f, 1.0f), 0.0f,
                                  config::Render::PANEL_OPACITY_MAX);
    job.toneCtrl[0] = clampRange(optics.brightness + tintBrightness(), -1.0f, 1.0f);
    job.toneCtrl[1] = clampRange(optics.contrast * tintContrast(), 0.0f, 2.0f);
    job.toneCtrl[2] = clampRange(optics.saturation * tintSaturation(), 0.0f, 3.0f);
    job.toneBoost = optics.colorBoost ? 1.0f : 0.0f;
    job.refractTint = tintRefract();

    job.surface = Rgba(clampRange(style.surface.r, 0.0f, 1.0f),
                       clampRange(style.surface.g, 0.0f, 1.0f),
                       clampRange(style.surface.b, 0.0f, 1.0f),
                       clampRange(style.surface.a, 0.0f, 1.0f));
    job.surfaceMode = style.surfaceMode == SurfaceMode::Overlay ? 2.0f
                    : (style.surfaceMode == SurfaceMode::Hue ? 1.0f : 0.0f);
    job.sceneOn = (style.backdropOn && backdropEnabledFlag()) ? 1.0f : 0.0f;
    job.solidBackdrop = style.opaqueBackdrop ? 1.0f : 0.0f;
    job.stretchX = clampRange(style.backdropScaleX, 0.0f, 2.0f);
    job.stretchY = clampRange(style.backdropScaleY, config::Render::BACKDROP_SCALE_MIN, 2.0f);
    job.stretchMix = clampRange(style.backdropBlend, 0.0f, 1.0f);
    job.absentTint = Rgba(clampRange(style.fallback.r, 0.0f, 1.0f),
                          clampRange(style.fallback.g, 0.0f, 1.0f),
                          clampRange(style.fallback.b, 0.0f, 1.0f),
                          clampRange(style.fallback.a, 0.0f, 1.0f));

    applyEdge(job, style, uniformScale);
    applyInset(job, style, uniformScale);
    applyLiquid(job, style, uniformScale);
    applyTrack(job, style, uniformScale);
    applyGlow(job, style);
    ++g_jobCount;
    return jobIndex;
}

int appendShadeJob(const PanelStyle& style, const Box& box) {
    if (!style.shadowOn || style.shadow.alpha <= 0.0f) return -1;
    const DropShadow& shadow = style.shadow;
    const float radius = shadow.radiusDp * densityScale();
    const float offsetX = shadow.offsetXDp * densityScale();
    const float offsetY = shadow.offsetYDp * densityScale();
    const float extent = radius * config::Render::SHADOW_EXTENT_GAIN;
    const Box shadeBox(box.left - extent + offsetX, box.top - extent + offsetY,
                       box.right + extent + offsetX, box.bottom + extent + offsetY);

    PanelStyle shadeStyle;
    shadeStyle.corners = style.corners;
    shadeStyle.alpha = 1.0f;
    shadeStyle.backdropOn = false;
    shadeStyle.shadowOn = false;
    shadeStyle.edgeOn = false;
    shadeStyle.insetOn = false;
    shadeStyle.trackOn = false;
    shadeStyle.liquid = false;
    shadeStyle.surfaceMode = SurfaceMode::None;

    const int index = appendJob(shadeStyle, shadeBox, config::Render::CAPTURE_PAD_BASE);
    if (index < 0) return -1;

    PanelJob& job = g_jobs[index];
    job.shadeMode = 1.0f;
    job.shadeTint = Rgba(channelOf(shadow.color, 16), channelOf(shadow.color, 8),
                         channelOf(shadow.color, 0),
                         alphaOf(shadow.color) * shadow.alpha * style.alpha);
    job.shadeShiftX = offsetX * job.pixelScaleX;
    job.shadeShiftY = offsetY * job.pixelScaleY;
    job.shadeReach = radius * uniformPixelScale();
    job.shadeBodyW = box.width() * job.pixelScaleX;
    job.shadeBodyH = box.height() * job.pixelScaleY;
    return index;
}

void paintShade(int index, const Box& clip) {
    if (index < 0 || index >= g_jobCount) return;
    PanelJob& job = g_jobs[index];
    if (job.shadePainted) return;
    job.shadePainted = true;
    const GatherInfo gather;
    glPaintPanel(job, clip, 0u, gather);
}

void captureScene(int index) {
    if (index < 0 || index >= g_jobCount) return;
    PanelJob& job = g_jobs[index];
    if (job.sampleReady || job.shadeMode > 0.5f || job.sceneOn < 0.5f) return;

    if (backdropReady()) {
        job.sampleOriginX = 0.0f;
        job.sampleOriginY = 0.0f;
        job.sampleSizeX = (float)g_backdropWidth;
        job.sampleSizeY = (float)g_backdropHeight;
        job.sampleStrideX = (float)g_backdropWidth;
        job.sampleStrideY = (float)g_backdropHeight;
        job.sampleReady = true;
    }
    else {

        const int screenW = (int)job.viewW;
        const int screenH = (int)job.viewH;
        if (screenW <= 0 || screenH <= 0) return;
        int x = (int)floorf(job.sampleOriginX);
        int y = (int)floorf(job.sampleOriginY);
        int w = (int)ceilf(job.sampleSizeX);
        int h = (int)ceilf(job.sampleSizeY);
        if (x < 0) { w += x; x = 0; }
        if (y < 0) { h += y; y = 0; }
        if (x > screenW) w = 0;
        if (y > screenH) h = 0;
        if (x + w > screenW) w = screenW - x;
        if (y + h > screenH) h = screenH - y;
        if (w <= 0 || h <= 0) return;

        float originX = 0.0f;
        float originY = 0.0f;
        float sizeX = 0.0f;
        float sizeY = 0.0f;
        float strideX = 0.0f;
        float strideY = 0.0f;
        if (!glCaptureRegion(x, y, w, h, screenW, screenH, originX, originY, sizeX, sizeY,
                             strideX, strideY)) {
        }
        else {
            job.sampleOriginX = originX;
            job.sampleOriginY = originY;
            job.sampleSizeX = sizeX;
            job.sampleSizeY = sizeY;
            job.sampleStrideX = strideX;
            job.sampleStrideY = strideY;
            job.sampleReady = true;
        }
    }
}

void paintJob(int index, const Box& clip) {
    if (index < 0 || index >= g_jobCount) return;
    PanelJob& job = g_jobs[index];

    unsigned int sceneTexture = 0u;
    if (backdropReady()) {
        sceneTexture = g_backdropTexture;
    } else if (job.sampleReady) {
        sceneTexture = glCaptureTexture();
    }
    if (job.shadeMode <= 0.5f && sceneTexture == 0u) job.sceneOn = 0.0f;

    job.gatherActive = false;
    if (job.shadeMode <= 0.5f && !backdropReady() && sceneTexture != 0u
            && job.blurRadius >= config::Render::BLUR_MIN_SIGMA) {
        if (glRunGather((int)job.sampleSizeX, (int)job.sampleSizeY, sceneTexture,
                        glCaptureWidth(), glCaptureHeight(), job.blurRadius)) {
            const GatherInfo info = glGatherInfo();
            job.gatherActive = info.ready;
            job.gatherPitchX = info.pitchX;
            job.gatherPitchY = info.pitchY;
            job.gatherRows = info.rows;
            job.gatherTexelX = info.texelX;
            job.gatherTexelY = info.texelY;
            job.gatherLimitX = info.limitX;
            job.gatherLimitY = info.limitY;
        }
    }

    GatherInfo gather;
    gather.ready = job.gatherActive;
    gather.pitchX = job.gatherPitchX;
    gather.pitchY = job.gatherPitchY;
    gather.rows = job.gatherRows;
    gather.texelX = job.gatherTexelX;
    gather.texelY = job.gatherTexelY;
    gather.limitX = job.gatherLimitX;
    gather.limitY = job.gatherLimitY;
    glPaintPanel(job, clip, sceneTexture, gather);
}

}

float pixelScaleXOf() {
    const float value = surfaceRef().pixelScaleX;
    return value > 0.0f ? value : 1.0f;
}

float pixelScaleYOf() {
    const float value = surfaceRef().pixelScaleY;
    return value > 0.0f ? value : 1.0f;
}

void bindBackdrop(unsigned int texture, int width, int height) {
    g_backdropTexture = texture;
    g_backdropWidth = width > 0 ? width : 0;
    g_backdropHeight = height > 0 ? height : 0;
}

void releaseBackdrop() {
    g_backdropTexture = 0;
    g_backdropWidth = 0;
    g_backdropHeight = 0;
}

void resetJobs() {
    g_jobCount = 0;
    g_panelCount = 0;
}

int submitJob(const PanelStyle& style, const Box& box) {
    if (!glReady() || g_jobCount >= config::Render::PANEL_JOB_LIMIT) return -1;
    return appendJob(style, box, capturePadding(style, uniformPixelScale()));
}

int registerPanel(int jobIndex, int shadeIndex) {
    if (g_panelCount >= config::Render::PANEL_JOB_LIMIT) return -1;
    const int panelIndex = g_panelCount;
    g_panelJob[panelIndex] = jobIndex;
    g_panelShade[panelIndex] = shadeIndex;
    ++g_panelCount;
    return panelIndex;
}

int submitPanelJob(const PanelStyle& style, const Box& box) {
    if (!glReady() || g_panelCount >= config::Render::PANEL_JOB_LIMIT) return -1;
    if (box.width() <= 0.0f || box.height() <= 0.0f) return -1;
    const int jobMark = g_jobCount;
    const int shadeIndex = appendShadeJob(style, box);
    const int jobIndex = submitJob(style, box);
    if (jobIndex < 0) {
        g_jobCount = jobMark;
        return -1;
    }
    g_jobs[jobIndex].shadeIndex = shadeIndex;
    const int panelIndex = registerPanel(jobIndex, shadeIndex);
    if (panelIndex < 0) {
        g_jobCount = jobMark;
        return -1;
    }
    return panelIndex;
}

void capturePanelByIndex(int index, const Box& clip) {
    if (index < 0 || index >= g_panelCount) return;
    paintShade(g_panelShade[index], clip);
    captureScene(g_panelJob[index]);
}

void paintPanelByIndex(int index, const Box& clip) {
    if (index < 0 || index >= g_panelCount) return;
    paintShade(g_panelShade[index], clip);
    paintJob(g_panelJob[index], clip);
}

}

int submitPanel(const PanelStyle& style, const Box& box) {
    return detail::submitPanelJob(style, box);
}

void runPanelCapture(int index, const Box& clip) {
    detail::capturePanelByIndex(index, clip);
}

void runPanelPaint(int index, const Box& clip) {
    detail::paintPanelByIndex(index, clip);
}

}
