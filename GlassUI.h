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

#include "GlassConfig.h"

namespace lgx {

inline constexpr int TAB_LIMIT = 8;
inline constexpr int MENU_ENTRY_LIMIT = 12;

struct Point {
    float x = 0.0f;
    float y = 0.0f;

    Point() = default;
    Point(float px, float py) : x(px), y(py) {}
};

struct Box {
    float left = 0.0f;
    float top = 0.0f;
    float right = 0.0f;
    float bottom = 0.0f;

    Box() = default;
    Box(float l, float t, float r, float b) : left(l), top(t), right(r), bottom(b) {}
    Box(const Point& min, const Point& max)
        : left(min.x), top(min.y), right(max.x), bottom(max.y) {}

    float width() const { return right - left; }
    float height() const { return bottom - top; }
    float centerX() const { return (left + right) * 0.5f; }
    float centerY() const { return (top + bottom) * 0.5f; }
    Point center() const { return Point(centerX(), centerY()); }

    bool holds(const Point& p) const {
        return p.x >= left && p.x <= right && p.y >= top && p.y <= bottom;
    }
    bool holds(float x, float y) const {
        return x >= left && x <= right && y >= top && y <= bottom;
    }
    Box grown(float amount) const {
        return Box(left - amount, top - amount, right + amount, bottom + amount);
    }
    Box shifted(float dx, float dy) const {
        return Box(left + dx, top + dy, right + dx, bottom + dy);
    }
};

struct Rgba {
    float r = 0.0f;
    float g = 0.0f;
    float b = 0.0f;
    float a = 0.0f;

    Rgba() = default;
    Rgba(float red, float green, float blue, float alpha)
        : r(red), g(green), b(blue), a(alpha) {}
};

struct Corners {
    float topLeft = config::CAPSULE_RADIUS;
    float topRight = config::CAPSULE_RADIUS;
    float bottomRight = config::CAPSULE_RADIUS;
    float bottomLeft = config::CAPSULE_RADIUS;

    Corners() = default;
    explicit Corners(float radius)
        : topLeft(radius), topRight(radius), bottomRight(radius), bottomLeft(radius) {}
    Corners(float tl, float tr, float br, float bl)
        : topLeft(tl), topRight(tr), bottomRight(br), bottomLeft(bl) {}

    static Corners capsule() { return Corners(); }
    static Corners rounded(float radius) { return Corners(radius); }
};

struct TextSize {
    float width = 0.0f;
    float height = 0.0f;
};

struct PointerState {
    float x = 0.0f;
    float y = 0.0f;
    bool down = false;
    bool pressed = false;
    bool released = false;
};

struct Surface {
    float width = 0.0f;
    float height = 0.0f;
    float pixelScaleX = 1.0f;
    float pixelScaleY = 1.0f;
};

using TextMeasurer = TextSize (*)(const char* content, float size);

unsigned int rgba(float alpha, unsigned int rgb);
Rgba rgbaOf(unsigned int argb);
unsigned int mixRgb(unsigned int from, unsigned int to, float t);

float saturate(float value);
float clampRange(float value, float low, float high);
float lerp(float from, float to, float t);
float easeOutCubic(float x);

float px(float value);
void setScale(float value);

struct GlassColors {
    Rgba primary;
    Rgba secondary;
};

void setGlassColors(const GlassColors& colors, bool withMotion = true);
bool glassIsLight();
float colorProgress();

void setBackdropEnabled(bool on);
void bindBackdropTexture(unsigned int texture, int width, int height);
void releaseBackdropTexture();
Rgba frostedFallbackColor();

void setPointer(const PointerState& state);

void setSurface(const Surface& surface);
void setHostBounds(const Box& box);

void setTextMeasurer(TextMeasurer measurer);

class Canvas;

using IconPainter = void (*)(Canvas& canvas, const Point& center, float size,
                             unsigned int color, void* tag);

struct PointerProbe {
    bool held = false;
    bool activated = false;
    bool clicked = false;
};

class CanvasBackend {
public:
    virtual ~CanvasBackend() = default;

    virtual Box clipRect() = 0;
    virtual void onFillRect(const Box& box, float radius, unsigned int color) = 0;
    virtual void onStrokeRect(const Box& box, float radius, float thickness,
                              unsigned int color) = 0;
    virtual void onFillPoly(const Point* points, int count, unsigned int color) = 0;
    virtual void onFillConcave(const Point* points, int count, unsigned int color) = 0;
    virtual void onStrokePoly(const Point* points, int count, float thickness,
                              unsigned int color) = 0;
    virtual void onLine(const Point& from, const Point& to, unsigned int color,
                        float thickness) = 0;
    virtual void onFillCircle(const Point& center, float radius, unsigned int color,
                              int segments) = 0;
    virtual void onFillTriangle(const Point& a, const Point& b, const Point& c,
                                unsigned int color) = 0;
    virtual void onFillArc(const Point& center, float radius, float fromRadians,
                           float toRadians, unsigned int color, int segments) = 0;
    virtual void onText(const Point& position, float size, unsigned int color,
                        const char* content, float wrapWidth) = 0;
    virtual void onImage(unsigned int texture, const Box& box, const Point& uvMin,
                         const Point& uvMax, unsigned int color) = 0;
    virtual void onPushClip(const Box& box) = 0;
    virtual void onPopClip() = 0;
    virtual void onPanelCapture(int index) = 0;
    virtual void onPanelPaint(int index) = 0;
    virtual PointerProbe onProbe(const Box& box, const void* owner, bool interactive) = 0;
    virtual void onBlockPointer(const Box& box) = 0;
    virtual void onClearPointerBlock() = 0;
};

void attachCanvasBackend(CanvasBackend* backend);

class Canvas {
public:
    explicit Canvas(CanvasBackend* backend) : surface_(backend) {}

    void fillRect(const Box& box, float radius, unsigned int color);
    void strokeRect(const Box& box, float radius, float thickness, unsigned int color);
    void fillPoly(const Point* points, int count, unsigned int color);
    void fillConcave(const Point* points, int count, unsigned int color);
    void strokePoly(const Point* points, int count, float thickness, unsigned int color);
    void line(const Point& from, const Point& to, unsigned int color, float thickness);
    void fillCircle(const Point& center, float radius, unsigned int color, int segments);
    void fillTriangle(const Point& a, const Point& b, const Point& c, unsigned int color);
    void fillArc(const Point& center, float radius, float fromRadians, float toRadians,
                 unsigned int color, int segments);
    void text(const Point& position, float size, unsigned int color, const char* content,
              float wrapWidth = 0.0f);
    void image(unsigned int texture, const Box& box, const Point& uvMin, const Point& uvMax,
               unsigned int color);
    void pushClip(const Box& box);
    void popClip();
    void icon(IconPainter painter, const Point& center, float size, unsigned int color,
              void* tag);

private:
    CanvasBackend* surface_ = nullptr;
};

bool startup();
void shutdown();
void beginFrame(float deltaSeconds);

enum class EdgeMode { Gradient, Plain, Ambient, Contour };

struct EdgeLight {
    EdgeMode mode = EdgeMode::Gradient;
    unsigned int color = config::Render::EDGE_TINT;
    float angleDegrees = 45.0f;
    float falloff = 1.0f;
    float widthDp = 0.5f;
    float softnessDp = 0.25f;
    float alpha = 1.0f;

    EdgeLight() = default;
    EdgeLight(EdgeMode m, unsigned int tint, float angle, float sharp, float width,
              float softness, float a)
        : mode(m), color(tint), angleDegrees(angle), falloff(sharp), widthDp(width),
          softnessDp(softness), alpha(a) {}

    static EdgeLight gradient() { return EdgeLight(); }
    static EdgeLight plain(unsigned int tint = config::Render::EDGE_TINT_PLAIN);
    static EdgeLight ambient(float strength = config::Render::EDGE_AMBIENT_ALPHA);
    static EdgeLight contour(float angleDegrees = -90.0f, float falloff = 1.35f,
                             unsigned int tint = 0xCCFFFFFFu);
};

struct DropShadow {
    float radiusDp = 24.0f;
    float offsetXDp = 0.0f;
    float offsetYDp = 4.0f;
    unsigned int color = 0x1A000000u;
    float alpha = 1.0f;

    DropShadow() = default;
    explicit DropShadow(float radius, unsigned int tint)
        : radiusDp(radius), offsetYDp(radius / config::Render::SHADOW_OFFSET_DIVISOR), color(tint) {}
    DropShadow(float radius, float offsetX, float offsetY, unsigned int tint, float a)
        : radiusDp(radius), offsetXDp(offsetX), offsetYDp(offsetY), color(tint), alpha(a) {}
};

struct InsetShadow {
    float radiusDp = 24.0f;
    float offsetXDp = 0.0f;
    float offsetYDp = 24.0f;
    unsigned int color = 0x26000000u;
    float alpha = 1.0f;

    InsetShadow() = default;
    InsetShadow(float radius, float a)
        : radiusDp(radius), offsetXDp(0.0f), offsetYDp(0.0f), alpha(a) {}
    InsetShadow(float radius, float offsetX, float offsetY, unsigned int tint, float a)
        : radiusDp(radius), offsetXDp(offsetX), offsetYDp(offsetY), color(tint), alpha(a) {}
};

struct Optics {
    bool colorBoost = false;
    float brightness = 0.0f;
    float contrast = 1.0f;
    float saturation = 1.0f;
    bool opacityOn = false;
    float opacity = 1.0f;
    float blurPx = 0.0f;
    float lensHeightPx = 0.0f;
    float lensAmountPx = 0.0f;
    float depthAmount = 0.0f;
    float darkGuard = 0.0f;
    float spectral = 0.0f;
};

enum class SurfaceMode { None, Hue, Overlay };

struct GlowSpot {
    bool on = false;
    float progress = 0.0f;
    float x = 0.0f;
    float y = 0.0f;
};

struct PanelStyle {
    Corners corners;
    Optics optics;

    EdgeLight edge;
    bool edgeOn = true;

    DropShadow shadow;
    bool shadowOn = false;

    InsetShadow inset;
    bool insetOn = false;

    Rgba surface;
    SurfaceMode surfaceMode = SurfaceMode::None;
    float alpha = 1.0f;

    bool backdropOn = true;
    bool opaqueBackdrop = false;
    float backdropScaleX = 1.0f;
    float backdropScaleY = 1.0f;
    float backdropBlend = 0.0f;
    Rgba fallback;

    bool trackOn = false;
    Box trackBox;
    float trackRadius = 0.0f;
    Rgba trackColor;

    GlowSpot glow;

    bool liquid = false;
    Point dropCenter;
    Point dropHalf;
    float dropRadius = 0.0f;
    Point feedCenter;
    Point feedHalf;
    float feedRadius = 0.0f;
    float feedBlend = 0.0f;
    Point strandCenter;
    Point strandHalf;
    float strandRadius = 0.0f;
    float strandBlend = 0.0f;
};

int submitPanel(const PanelStyle& style, const Box& box);

void runPanelCapture(int index, const Box& clip);
void runPanelPaint(int index, const Box& clip);
void drawPanel(const PanelStyle& style, const Box& box);
void blockPointer(const Box& box);
void clearPointerBlock();

class PillButton {
public:
    PillButton();
    ~PillButton();
    PillButton(const PillButton&) = delete;
    PillButton& operator=(const PillButton&) = delete;

    const char* label = nullptr;
    bool interactive = true;
    bool tinted = false;
    Rgba tint = Rgba(1.0f, 1.0f, 1.0f, 1.0f);
    bool surfaced = false;
    Rgba surface = Rgba(1.0f, 1.0f, 1.0f, 0.3f);
    unsigned int textColor = 0;
    float textSizeSp = config::Layout::BUTTON_TEXT_SP;
    float blurDp = config::Layout::BUTTON_BLUR_DP;
    float lensHeightDp = config::Layout::BUTTON_LENS_HEIGHT_DP;
    float lensAmountDp = config::Layout::BUTTON_LENS_AMOUNT_DP;
    float spectral = 0.0f;
    float visualScale = 1.0f;
    float alphaScale = 1.0f;
    bool shadowOn = true;
    bool contourEdge = false;
    bool glassOn = true;
    bool textOn = true;
    IconPainter icon = nullptr;
    void* iconTag = nullptr;
    int clicks = 0;

    bool draw(const Box& box);
    void resetGlow();
    bool glowVisible() const;
    float glowProgress() const;
    bool busy() const;

private:
    struct State;
    State* state_ = nullptr;
    State& state();
    const State& state() const;
};

enum class EntryKind { Action, Heading, Divider };
enum class MenuReach { Auto, DownRight, DownLeft, UpRight, UpLeft };

struct MenuEntry {
    EntryKind kind = EntryKind::Action;
    const char* caption = nullptr;
    const char* detail = nullptr;
    const char* shortcut = nullptr;
    unsigned int texture = 0;
    IconPainter painter = nullptr;
    void* tag = nullptr;
    bool selectable = true;
    bool danger = false;
    bool keepOpen = false;
    float heightDp = 0.0f;
};

class PopMenu {
public:
    PopMenu();
    ~PopMenu();
    PopMenu(const PopMenu&) = delete;
    PopMenu& operator=(const PopMenu&) = delete;

    bool expanded = false;
    bool interactive = true;
    int cursorRow = 0;
    int count = 0;
    int activated = -1;
    MenuEntry entries[MENU_ENTRY_LIMIT];

    float widthDp = config::Layout::MENU_DEFAULT_W_DP;
    float cornerDp = config::Layout::MENU_DEFAULT_CORNER_DP;
    float bandCornerDp = config::Layout::MENU_DEFAULT_BAND_CORNER_DP;
    MenuReach reach = MenuReach::Auto;
    bool clampToHost = true;
    bool closeOnOutside = true;
    float threadDp = config::Layout::MENU_DEFAULT_THREAD_DP;
    float gapDp = config::Layout::MENU_DEFAULT_GAP_DP;
    float drainStart = config::Layout::MENU_DRAIN_START;
    float drainSpan = config::Layout::MENU_DRAIN_SPAN;
    float greetDp = config::Layout::MENU_DEFAULT_GREET_DP;
    float stretchGain = 1.0f;
    bool materialFlare = true;
    float rimDp = 1.0f;
    float rimAlpha = 1.0f;
    float jelly = 1.0f;

    const char* capText = nullptr;
    unsigned int capTextColor = 0;
    float capTextSp = config::Layout::MENU_CAP_TEXT_SP;
    unsigned int bandColor = config::Render::MENU_BAND;

    bool draw(const Box& capBox);
    void drawOverlay();
    void setOpen(bool value, bool withMotion = true);
    bool isOpen() const;

private:
    struct State;
    State* state_ = nullptr;
    State& state();
    const State& state() const;
    int slotAt(const Point& pointer) const;
};

class Toggle {
public:
    Toggle();
    ~Toggle();
    Toggle(const Toggle&) = delete;
    Toggle& operator=(const Toggle&) = delete;

    bool on = false;
    unsigned int accentColor = config::Layout::TOGGLE_ACCENT;
    unsigned int trackColor = config::Layout::TOGGLE_TRACK;

    bool draw(const Box& box);
    void setOn(bool value, bool withMotion = true);
    bool busy() const;

private:
    struct State;
    State* state_ = nullptr;
    State& state();
    const State& state() const;
};

class Slider {
public:
    Slider();
    ~Slider();
    Slider(const Slider&) = delete;
    Slider& operator=(const Slider&) = delete;

    float minValue = 0.0f;
    float maxValue = 1.0f;
    float value = 0.5f;
    unsigned int accentColor = config::Layout::SLIDER_ACCENT;
    unsigned int trackColor = config::Layout::SLIDER_TRACK;

    bool draw(const Box& box);
    void setRange(float from, float to);
    float animatedValue() const;
    float displayValue() const;
    bool busy() const;

private:
    struct State;
    State* state_ = nullptr;
    State& state();
    const State& state() const;
};

class TabStrip {
public:
    TabStrip();
    ~TabStrip();
    TabStrip(const TabStrip&) = delete;
    TabStrip& operator=(const TabStrip&) = delete;

    int activeIndex = 0;
    int tabCount = 4;
    const char* titles[TAB_LIMIT] = {"Tab1", "Tab2", "Tab3", "Tab4",
                                     "Tab5", "Tab6", "Tab7", "Tab8"};
    unsigned int textures[TAB_LIMIT] = {};
    IconPainter painters[TAB_LIMIT] = {};
    void* tags[TAB_LIMIT] = {};
    float iconSizeDp = config::Layout::TAB_ICON_SIZE_DP;
    float labelSizeDp = config::Layout::TAB_LABEL_SIZE_DP;
    unsigned int accentColor = config::Layout::TAB_ACCENT;
    unsigned int containerColor = config::Layout::TAB_CONTAINER;
    unsigned int iconColor = 0;
    bool changed = false;

    bool appendTab(const char* title, unsigned int texture = 0);
    bool dropTab(int index);
    void resetTabs();

    bool draw(const Box& box);
    void select(int index, bool withMotion = true);
    float indicator() const;
    bool busy() const;

private:
    struct State;
    State* state_ = nullptr;
    State& state();
    const State& state() const;
};

class Modal {
public:
    Modal();
    ~Modal();
    Modal(const Modal&) = delete;
    Modal& operator=(const Modal&) = delete;

    bool visible = false;
    const char* title = nullptr;
    const char* message = nullptr;
    const char* cancelLabel = "Cancel";
    const char* confirmLabel = "OK";
    float glassBlurDp = config::Layout::DIALOG_GLASS_BLUR_DP;
    int result = 0;

    void open(const char* dialogTitle, const char* dialogMessage);
    void openFrom(const char* dialogTitle, const char* dialogMessage, const Box& origin);
    void close(int dialogResult);
    int draw(const Box& area);
    float sourceScale() const;

private:
    struct State;
    State* state_ = nullptr;
    State& state();
    const State& state() const;
    void advance(float dt);
};

void iconHome(Canvas& canvas, const Point& center, float size, unsigned int color, void* tag);
void iconSearch(Canvas& canvas, const Point& center, float size, unsigned int color, void* tag);
void iconMessage(Canvas& canvas, const Point& center, float size, unsigned int color, void* tag);
void iconProfile(Canvas& canvas, const Point& center, float size, unsigned int color, void* tag);
void iconHeart(Canvas& canvas, const Point& center, float size, unsigned int color, void* tag);
void iconStar(Canvas& canvas, const Point& center, float size, unsigned int color, void* tag);
void iconBell(Canvas& canvas, const Point& center, float size, unsigned int color, void* tag);
void iconGear(Canvas& canvas, const Point& center, float size, unsigned int color, void* tag);
void iconClose(Canvas& canvas, const Point& center, float size, unsigned int color, void* tag);
void iconCheck(Canvas& canvas, const Point& center, float size, unsigned int color, void* tag);
void iconChevron(Canvas& canvas, const Point& center, float size, unsigned int color, void* tag);

}
