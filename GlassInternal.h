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

#include "GlassUI.h"
#include "ColorAdapter.h"

namespace lgx {
namespace detail {

class Spring {
public:
    Spring() = default;
    Spring(float initial, float damping, float stiffness, float threshold);

    float value = 0.0f;
    float velocity = 0.0f;
    float target = 0.0f;
    float damping = config::Motion::PRESS_DAMPING;
    float stiffness = config::Motion::PRESS_STIFFNESS;
    float threshold = config::Motion::SPRING_EPSILON;

    void snapTo(float v);
    void animateTo(float v);
    bool step(float dt);
    bool moving() const;
};

class TrackMotion {
public:
    TrackMotion(float initialValue, float rangeStart, float rangeEnd, float visibility,
                float idleScale, float heldScale,
                float valueStiffness = config::Motion::DRAG_VALUE_STIFFNESS,
                float valueDamping = config::Motion::DRAG_VALUE_DAMPING);

    float value() const { return valueSpring_.value; }
    float targetValue() const { return valueSpring_.target; }
    float progress() const;
    float pressProgress() const;
    float scaleX() const { return scaleXSpring_.value; }
    float scaleY() const { return scaleYSpring_.value; }
    float speed() const { return speedSpring_.value; }
    bool moving() const;

    void setRange(float start, float end);
    void press();
    void release();
    void updateValue(float v);
    void snapValue(float v);
    void animateTo(float v);
    void settleTo(float v);
    bool step(float dt);

private:
    float clampToRange(float v) const;
    void measureSpeed();

    float rangeStart_ = 0.0f;
    float rangeEnd_ = 1.0f;
    float idleScale_ = 1.0f;
    float heldScale_ = 1.0f;

    Spring valueSpring_;
    Spring speedSpring_;
    Spring pressSpring_;
    Spring scaleXSpring_;
    Spring scaleYSpring_;

    float lastValue_ = 0.0f;
    float lastStep_ = config::Motion::FRAME_FALLBACK;
    bool releasePending_ = false;
    bool releaseArmed_ = false;
    bool trackSpeed_ = false;
};

class GlowSpring {
public:
    GlowSpring() = default;

    void press(float x, float y);
    void move(float x, float y);
    void release();
    void abort();
    void step(float dt);

    float progress() const { return strength_.value; }
    float offsetX() const { return spotX_.value - anchorX_; }
    float offsetY() const { return spotY_.value - anchorY_; }
    float pointerX() const { return spotX_.value; }
    float pointerY() const { return spotY_.value; }
    bool visible() const { return enabled_ && strength_.value > 0.0f; }
    bool running() const { return running_; }
    bool enabled() const { return enabled_; }

private:
    Spring strength_{0.0f, config::Motion::PRESS_DAMPING, config::Motion::PRESS_STIFFNESS,
                     config::Motion::PRESS_VISIBLE};
    Spring spotX_{0.0f, config::Motion::POINTER_DAMPING, config::Motion::POINTER_STIFFNESS,
                  config::Motion::POINTER_VISIBLE};
    Spring spotY_{0.0f, config::Motion::POINTER_DAMPING, config::Motion::POINTER_STIFFNESS,
                  config::Motion::POINTER_VISIBLE};
    float anchorX_ = 0.0f;
    float anchorY_ = 0.0f;
    bool enabled_ = true;
    bool running_ = false;
};

struct PanelJob {
    Box frame;
    float viewW = 1.0f;
    float viewH = 1.0f;
    float pixelScaleX = 1.0f;
    float pixelScaleY = 1.0f;

    float sampleOriginX = 0.0f;
    float sampleOriginY = 0.0f;
    float sampleSizeX = 0.0f;
    float sampleSizeY = 0.0f;
    float sampleStrideX = 1.0f;
    float sampleStrideY = 1.0f;
    bool sampleReady = false;

    float gatherPitchX = 1.0f;
    float gatherPitchY = 1.0f;
    float gatherRows = 0.0f;
    float gatherTexelX = 1.0f;
    float gatherTexelY = 1.0f;
    float gatherLimitX = 1.0f;
    float gatherLimitY = 1.0f;
    bool gatherActive = false;

    Corners corners;
    float dropRect[4] = {0.0f, 0.0f, -1.0f, -1.0f};
    float dropRound = 0.0f;
    float feedRect[4] = {0.0f, 0.0f, -1.0f, -1.0f};
    float feedRound = 0.0f;
    float feedMerge = 0.0f;
    float strandRect[4] = {0.0f, 0.0f, -1.0f, -1.0f};
    float strandRound = 0.0f;
    float strandMerge = 0.0f;

    float lensHeight = 0.0f;
    float lensAmount = 0.0f;
    float lensDepth = 0.0f;
    float darkGuard = 0.0f;
    float spectral = 0.0f;
    float blurRadius = 0.0f;
    float panelOpacity = 1.0f;
    float toneCtrl[3] = {0.0f, 1.0f, 1.0f};
    float toneBoost = 0.0f;
    Rgba refractTint;
    Rgba surface;
    float surfaceMode = 0.0f;

    float edgeMode = 0.0f;
    Rgba edgeTint;
    float edgeTintAlpha = 1.0f;
    float edgeAngle = 0.0f;
    float edgeFalloff = 1.0f;
    float edgeAlpha = 0.0f;
    float edgeWidth = 0.0f;
    float edgeBlur = 0.0f;

    Rgba insetTint;
    float insetShiftX = 0.0f;
    float insetShiftY = 0.0f;
    float insetReach = 0.0f;
    float insetAlpha = 0.0f;

    float glowOn = 0.0f;
    float glowX = 0.0f;
    float glowY = 0.0f;
    float glowReach = 0.0f;
    float glowLevel = 0.0f;

    float sceneOn = 0.0f;
    float solidBackdrop = 0.0f;
    float stretchX = 1.0f;
    float stretchY = 1.0f;
    float stretchMix = 0.0f;
    Rgba absentTint;

    float trackOn = 0.0f;
    float trackRect[4] = {0.0f, 0.0f, 0.0f, 0.0f};
    float trackRadius = 0.0f;
    Rgba trackTint;

    float shadeMode = 0.0f;
    Rgba shadeTint;
    float shadeShiftX = 0.0f;
    float shadeShiftY = 0.0f;
    float shadeReach = 0.0f;
    float shadeBodyW = 0.0f;
    float shadeBodyH = 0.0f;

    int shadeIndex = -1;
    bool shadePainted = false;
};

void resetJobs();

float pixelScaleXOf();
float pixelScaleYOf();
void bindBackdrop(unsigned int texture, int width, int height);
void releaseBackdrop();

float frameDelta();
int frameStamp();
float densityScale();
float uniformPixelScale();
bool lightFlag();
bool backdropEnabledFlag();

void setDensity(float value);

void setBackdropFlag(bool on);
void setPointerState(const PointerState& state);
void setSurfaceSize(const Surface& value);
void setTextMeasurerHook(TextMeasurer measurer);
void setHostBox(const Box& box);
void setCanvasBackend(CanvasBackend* backend);
void beginStamp();
void advanceStamp(float deltaSeconds);

CanvasBackend* canvasBackendRef();
const PointerState& pointerRef();
const Surface& surfaceRef();
Box hostBox();

PointerProbe probeBox(const Box& box, const void* owner, bool interactive);
void blockPointer(const Box& box);
void clearPointerBlock();
void panelCapture(int panelIndex);
void panelPaint(int panelIndex);

Box viewBounds();

TextSize textMetrics(const char* content, float size);
void fillRound(const Box& box, float radius, unsigned int color);
void strokeRound(const Box& box, float radius, float thickness, unsigned int color);
void strokeSegment(const Point& from, const Point& to, unsigned int color, float thickness);
void drawTextAt(const Point& position, float size, unsigned int color, const char* content,
                float wrapWidth);
void drawTextCentered(const Point& center, float size, unsigned int color,
                      const char* content);
void drawTexture(unsigned int texture, const Box& box, unsigned int color);
void pushClipBox(const Box& box);
void popClipBox();
void paintIcon(IconPainter painter, const Point& center, float size, unsigned int color,
               void* tag);

float smoothStep01(float x);
float curveAt(float t);
float curveHold(float t);
float spineInner(float size, float base);
Point zoomAbout(const Point& center, const Point& p, float s);
void upperAscii(const char* src, char* dst, int capacity);

int arcOutline(const Point& center, float radius, float fromRadians, float toRadians,
               int samples, Point* out, int capacity);

struct SpineFrame {
    float capX = 0.0f;
    float capY = 0.0f;
    float capW = 0.0f;
    float capH = 0.0f;
    float capRadius = 0.0f;
    float shellW = 0.0f;
    float shellH = 0.0f;
    float shiftX = 0.0f;
    float shiftY = 0.0f;
    float fitX = 0.0f;
    float fitY = 0.0f;
    int rows = 0;
    float rowTop[MENU_ENTRY_LIMIT] = {};
    float rowHeight[MENU_ENTRY_LIMIT] = {};
    float padY = 0.0f;
    float rowGap = 0.0f;
    float sidePad = 0.0f;
    float textPadX = 0.0f;
    float shellRadius = 0.0f;
    float bandRadius = 0.0f;
    float capTextPx = 0.0f;
    float captionPx = 0.0f;
    float detailPx = 0.0f;
    float markPx = 0.0f;
    float headingPx = 0.0f;
    bool valid = false;
    int stamp = -1;
};

struct SpineTune {
    float drainStart = config::Layout::MENU_DRAIN_START;
    float drainSpan = config::Layout::MENU_DRAIN_SPAN;
    float stretchGain = 1.0f;
    float gapDp = config::Layout::MENU_DEFAULT_GAP_DP;
    float greetDp = config::Layout::MENU_DEFAULT_GREET_DP;
    float threadDp = config::Layout::MENU_DEFAULT_THREAD_DP;
    float jelly = 1.0f;
};

struct NeckSpine {
    Point dropCenter;
    Point dropHalf;
    float dropRound = 0.0f;
    Point feedCenter;
    Point feedHalf;
    float feedRound = 0.0f;
    bool feedLive = false;
    Point strandCenter;
    Point strandHalf;
    float strandRadius = 0.0f;
    float strandRound = 0.0f;
    float strandMerge = 0.0f;
    float feedMerge = 0.0f;
    float swing = 0.0f;
    float spread = 0.0f;
    float size = 0.0f;
    float drain = 0.0f;
    float flare = 0.0f;
    float greet = 0.0f;
    Box body;
};

struct SpinePlacement {
    float shiftX = 0.0f;
    float shiftY = 0.0f;
    float fitX = 0.0f;
    float fitY = 0.0f;
};

NeckSpine evaluateSpine(const SpineFrame& frame, const Spring& unfold, const Spring& widen,
                        const Spring& deepen, const Spring& greet, const SpineTune& tune);
bool spineVisible(const NeckSpine& spine);
bool spineLocalY(const NeckSpine& spine, const Point& p, float inner, float& localY);
int pickRow(const SpineFrame& frame, const MenuEntry* entries, float localY);
void fillLiquid(PanelStyle& style, const NeckSpine& spine, const Box& body);
SpinePlacement placeSpine(const SpineFrame& frame, float shellW, float shellH, MenuReach reach,
                          bool clampToHost);
void snapSpine(Spring& unfold, Spring& widen, Spring& deepen, Spring& greet, float value);
void tickSpine(Spring& unfold, Spring& widen, Spring& deepen, Spring& greet, float& flow,
               bool& settled, bool& wasOpen, bool expanded, int rows, float dt);

}
}
