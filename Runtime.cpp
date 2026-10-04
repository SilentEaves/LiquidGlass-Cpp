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

float g_scale = 2.0f;
bool g_backdrop = true;
PointerState g_pointer;
Surface g_surface;
Box g_hostBounds;
TextMeasurer g_measurer = nullptr;
CanvasBackend* g_backend = nullptr;
Canvas g_canvas(nullptr);

float g_delta = config::Motion::FRAME_FALLBACK;
int g_stamp = 0;

}

float frameDelta() { return g_delta; }
int frameStamp() { return g_stamp; }
float densityScale() { return g_scale; }
bool backdropEnabledFlag() { return g_backdrop; }

void setDensity(float value) { g_scale = value; }
void setBackdropFlag(bool on) { g_backdrop = on; }
void setPointerState(const PointerState& state) { g_pointer = state; }
void setSurfaceSize(const Surface& value) { g_surface = value; }
void setTextMeasurerHook(TextMeasurer measurer) { g_measurer = measurer; }
void setHostBox(const Box& box) { g_hostBounds = box; }

void setCanvasBackend(CanvasBackend* backend) {
    g_backend = backend;
    g_canvas = Canvas(backend);
}

CanvasBackend* canvasBackendRef() { return g_backend; }
const PointerState& pointerRef() { return g_pointer; }
const Surface& surfaceRef() { return g_surface; }
Box hostBox() { return g_hostBounds; }

void beginStamp() {
    g_stamp = 0;
    g_delta = config::Motion::FRAME_FALLBACK;
    resetTint();
}

void advanceStamp(float deltaSeconds) {
    ++g_stamp;
    if (deltaSeconds > 0.0f) {
        g_delta = deltaSeconds > config::Motion::STEP_DELTA_MAX
            ? config::Motion::STEP_DELTA_MAX : deltaSeconds;
    }
    else {
        g_delta = config::Motion::FRAME_FALLBACK;
    }
}

float uniformPixelScale() {
    const float value = 0.5f * (pixelScaleXOf() + pixelScaleYOf());
    if (!isfinite(value)) return 1.0f;
    if (value < 0.5f) return 0.5f;
    return value > 2.0f ? 2.0f : value;
}

PointerProbe probeBox(const Box& box, const void* owner, bool interactive) {
    if (g_backend == nullptr) return PointerProbe();
    return g_backend->onProbe(box, owner, interactive);
}

void blockPointer(const Box& box) {
    if (g_backend == nullptr) return;
    if (!box.holds(g_pointer.x, g_pointer.y)) return;
    g_backend->onBlockPointer(box);
}

void clearPointerBlock() {
    if (g_backend == nullptr) return;
    g_backend->onClearPointerBlock();
}

void panelCapture(int panelIndex) {
    if (g_backend != nullptr && panelIndex >= 0) g_backend->onPanelCapture(panelIndex);
}

void panelPaint(int panelIndex) {
    if (g_backend != nullptr && panelIndex >= 0) g_backend->onPanelPaint(panelIndex);
}

Box viewBounds() {
    if (g_surface.width > 0.0f && g_surface.height > 0.0f) {
        return Box(0.0f, 0.0f, g_surface.width, g_surface.height);
    }
    return Box(0.0f, 0.0f, 1.0f, 1.0f);
}

TextSize textMetrics(const char* content, float size) {
    if (g_measurer == nullptr || content == nullptr) return TextSize();
    return g_measurer(content, size);
}

void fillRound(const Box& box, float radius, unsigned int color) {
    g_canvas.fillRect(box, radius, color);
}

void strokeRound(const Box& box, float radius, float thickness, unsigned int color) {
    g_canvas.strokeRect(box, radius, thickness, color);
}

void strokeSegment(const Point& from, const Point& to, unsigned int color, float thickness) {
    g_canvas.line(from, to, color, thickness);
}

void drawTextAt(const Point& position, float size, unsigned int color, const char* content,
                float wrapWidth) {
    g_canvas.text(position, size, color, content, wrapWidth);
}

void drawTextCentered(const Point& center, float size, unsigned int color,
                      const char* content) {
    if (content == nullptr) return;
    const TextSize span = textMetrics(content, size);
    g_canvas.text(Point(center.x - span.width * 0.5f, center.y - span.height * 0.5f),
                  size, color, content, 0.0f);
}

void drawTexture(unsigned int texture, const Box& box, unsigned int color) {
    g_canvas.image(texture, box, Point(0.0f, 0.0f), Point(1.0f, 1.0f), color);
}

void pushClipBox(const Box& box) {
    g_canvas.pushClip(box);
}

void popClipBox() {
    g_canvas.popClip();
}

void paintIcon(IconPainter painter, const Point& center, float size, unsigned int color,
               void* tag) {
    g_canvas.icon(painter, center, size, color, tag);
}

float smoothStep01(float x) {
    const float t = saturate(x);
    return t * t * (3.0f - 2.0f * t);
}

float spineInner(float size, float base) {
    return lerp(base, 1.0f,
                easeOutCubic(saturate((size - config::Motion::SHELL_VEIL_AT)
                                      / config::Motion::SHELL_INNER_SPAN)));
}

Point zoomAbout(const Point& center, const Point& p, float s) {
    return Point(center.x + (p.x - center.x) * s, center.y + (p.y - center.y) * s);
}

void upperAscii(const char* src, char* dst, int capacity) {
    if (dst == nullptr || capacity <= 0) return;
    int cursor = 0;
    if (src != nullptr) {
        while (src[cursor] != 0 && cursor < capacity - 1) {
            const char c = src[cursor];
            dst[cursor] = (c >= 'a' && c <= 'z') ? (char)(c - 32) : c;
            ++cursor;
        }
    }
    dst[cursor] = 0;
}

}

float saturate(float value) {
    if (!(value > 0.0f)) return 0.0f;
    return value > 1.0f ? 1.0f : value;
}

float clampRange(float value, float low, float high) {
    if (!isfinite(value) || !isfinite(low) || !isfinite(high)) return low;
    if (high < low) {
        const float swap = low;
        low = high;
        high = swap;
    }
    if (value < low) return low;
    return value > high ? high : value;
}

float lerp(float from, float to, float t) { return from + (to - from) * t; }

namespace detail {

float curveAt(float t) {
    const float x = clampRange(t, 0.0f, 1.0f);
    if (x <= 0.0f || x >= 1.0f) return x;
    const float x1 = config::Motion::CURVE_X1;
    const float y1 = config::Motion::CURVE_Y1;
    const float x2 = config::Motion::CURVE_X2;
    const float y2 = config::Motion::CURVE_Y2;
    float u = x;
    for (int i = 0; i < config::Motion::CURVE_ITERATIONS; ++i) {
        const float inv = 1.0f - u;
        const float bx = 3.0f * inv * inv * u * x1 + 3.0f * inv * u * u * x2 + u * u * u;
        const float dx = 3.0f * inv * inv * x1 + 6.0f * inv * u * (x2 - x1)
                       + 3.0f * u * u * (1.0f - x2);
        const float error = bx - x;
        if (fabsf(error) < config::Motion::CURVE_TOLERANCE) break;
        if (fabsf(dx) < config::Motion::CURVE_SLOPE_FLOOR) break;
        u = clampRange(u - error / dx, 0.0f, 1.0f);
    }
    const float inv = 1.0f - u;
    return 3.0f * inv * inv * u * y1 + 3.0f * inv * u * u * y2 + u * u * u;
}

float curveHold(float t) {
    return curveAt(saturate(t) / config::Motion::CURVE_HOLD);
}

}

float easeOutCubic(float x) {
    const float target = clampRange(x, 0.0f, 1.0f);
    if (target == 0.0f || target == 1.0f) return target;
    const float control = config::Motion::EASE_OUT_CONTROL;
    float t = target;
    bool converged = false;
    for (int i = 0; i < config::Motion::EASE_SOLVE_ITERATIONS && !converged; ++i) {
        const float inverse = 1.0f - t;
        const float curve = 3.0f * control * inverse * t * t + t * t * t;
        const float slope = 6.0f * control * inverse * t - 3.0f * control * t * t
                          + 3.0f * t * t;
        const float error = curve - target;
        converged = fabsf(error) < config::Motion::EASE_SOLVE_TOLERANCE
                 || fabsf(slope) < config::Motion::EASE_SOLVE_TOLERANCE;
        if (!converged) t = clampRange(t - error / slope, 0.0f, 1.0f);
    }
    const float inverse = 1.0f - t;
    return 3.0f * inverse * t * t + t * t * t;
}

unsigned int rgba(float alpha, unsigned int rgb) {
    const float clamped = saturate(alpha);
    const unsigned int a = (unsigned int)(config::COLOR_CHANNEL * clamped + 0.5f);
    const unsigned int r = (rgb >> 16) & 0xFFu;
    const unsigned int g = (rgb >> 8) & 0xFFu;
    const unsigned int b = rgb & 0xFFu;
    return (a << 24) | (r << 16) | (g << 8) | b;
}

Rgba rgbaOf(unsigned int argb) {
    return Rgba((float)((argb >> 16) & 0xFFu) / config::COLOR_CHANNEL,
                (float)((argb >> 8) & 0xFFu) / config::COLOR_CHANNEL,
                (float)(argb & 0xFFu) / config::COLOR_CHANNEL,
                (float)((argb >> 24) & 0xFFu) / config::COLOR_CHANNEL);
}

unsigned int mixRgb(unsigned int from, unsigned int to, float t) {
    const float clamped = saturate(t);
    const unsigned int start[4] = {(from >> 24) & 0xFFu, (from >> 16) & 0xFFu,
                                   (from >> 8) & 0xFFu, from & 0xFFu};
    const unsigned int stop[4] = {(to >> 24) & 0xFFu, (to >> 16) & 0xFFu,
                                  (to >> 8) & 0xFFu, to & 0xFFu};
    unsigned int blended[4];
    for (int i = 0; i < 4; ++i) {
        const float a = (float)start[i];
        const float b = (float)stop[i];
        blended[i] = (unsigned int)(a + (b - a) * clamped + 0.5f);
    }
    return (blended[0] << 24) | (blended[1] << 16) | (blended[2] << 8) | blended[3];
}

float px(float value) { return value * detail::densityScale(); }

void setScale(float value) {
    detail::setDensity(clampRange(value, config::DENSITY_MIN, config::DENSITY_MAX));
}

void setBackdropEnabled(bool on) { detail::setBackdropFlag(on); }

Rgba frostedFallbackColor() { return detail::tintFallback(); }

void setGlassColors(const GlassColors& colors, bool animated) {
    detail::applyThemeColors(colors, animated);
}

bool glassIsLight() { return detail::lightFlag(); }

float colorProgress() { return detail::tintBlend(); }

void setPointer(const PointerState& state) { detail::setPointerState(state); }

void setSurface(const Surface& value) { detail::setSurfaceSize(value); }

void setHostBounds(const Box& box) { detail::setHostBox(box); }

void setTextMeasurer(TextMeasurer measurer) { detail::setTextMeasurerHook(measurer); }

void attachCanvasBackend(CanvasBackend* backend) { detail::setCanvasBackend(backend); }

void bindBackdropTexture(unsigned int texture, int width, int height) {
    detail::bindBackdrop(texture, width, height);
}

void releaseBackdropTexture() { detail::releaseBackdrop(); }

void drawPanel(const PanelStyle& style, const Box& box) {
    if (detail::canvasBackendRef() == nullptr) return;
    const int panel = submitPanel(style, box);
    if (panel < 0) return;
    detail::panelCapture(panel);
    detail::panelPaint(panel);
}

void blockPointer(const Box& box) { detail::blockPointer(box); }

void clearPointerBlock() { detail::clearPointerBlock(); }

void Canvas::fillRect(const Box& box, float radius, unsigned int color) {
    if (surface_ != nullptr) surface_->onFillRect(box, radius, color);
}

void Canvas::strokeRect(const Box& box, float radius, float thickness, unsigned int color) {
    if (surface_ != nullptr) surface_->onStrokeRect(box, radius, thickness, color);
}

void Canvas::fillPoly(const Point* points, int count, unsigned int color) {
    if (surface_ != nullptr && points != nullptr && count >= 3) {
        surface_->onFillPoly(points, count, color);
    }
}

void Canvas::fillConcave(const Point* points, int count, unsigned int color) {
    if (surface_ != nullptr && points != nullptr && count >= 3) {
        surface_->onFillConcave(points, count, color);
    }
}

void Canvas::strokePoly(const Point* points, int count, float thickness, unsigned int color) {
    if (surface_ != nullptr && points != nullptr && count >= 2) {
        surface_->onStrokePoly(points, count, thickness, color);
    }
}

void Canvas::line(const Point& from, const Point& to, unsigned int color, float thickness) {
    if (surface_ != nullptr) surface_->onLine(from, to, color, thickness);
}

void Canvas::fillCircle(const Point& center, float radius, unsigned int color, int segments) {
    if (surface_ != nullptr) surface_->onFillCircle(center, radius, color, segments);
}

void Canvas::fillTriangle(const Point& a, const Point& b, const Point& c,
                          unsigned int color) {
    if (surface_ != nullptr) surface_->onFillTriangle(a, b, c, color);
}

void Canvas::fillArc(const Point& center, float radius, float fromRadians, float toRadians,
                     unsigned int color, int segments) {
    if (surface_ != nullptr) {
        surface_->onFillArc(center, radius, fromRadians, toRadians, color, segments);
    }
}

void Canvas::text(const Point& position, float size, unsigned int color, const char* content,
                  float wrapWidth) {
    if (surface_ != nullptr && content != nullptr) {
        surface_->onText(position, size, color, content, wrapWidth);
    }
}

void Canvas::image(unsigned int texture, const Box& box, const Point& uvMin,
                   const Point& uvMax, unsigned int color) {
    if (surface_ != nullptr) surface_->onImage(texture, box, uvMin, uvMax, color);
}

void Canvas::pushClip(const Box& box) {
    if (surface_ != nullptr) surface_->onPushClip(box);
}

void Canvas::popClip() {
    if (surface_ != nullptr) surface_->onPopClip();
}

void Canvas::icon(IconPainter painter, const Point& center, float size, unsigned int color,
                  void* tag) {
    if (painter != nullptr) painter(*this, center, size, color, tag);
}

bool startup() {
    if (!detail::glStartup()) return false;
    detail::beginStamp();
    return detail::glReady();
}

void shutdown() {
    detail::glShutdown();
    detail::releaseBackdrop();
    detail::resetJobs();
}

void beginFrame(float deltaSeconds) {
    detail::advanceStamp(deltaSeconds);
    detail::advanceTint(deltaSeconds);
    detail::clearPointerBlock();
    detail::resetJobs();
    detail::glBeginFrame();
}

EdgeLight EdgeLight::plain(unsigned int tint) {
    return EdgeLight(EdgeMode::Plain, tint, config::Render::EDGE_ANGLE_DEFAULT,
                     config::Render::EDGE_FALLOFF_DEFAULT,
                     config::Render::EDGE_PLAIN_WIDTH,
                     config::Render::EDGE_PLAIN_SOFTNESS,
                     config::Render::EDGE_PLAIN_ALPHA);
}

EdgeLight EdgeLight::ambient(float strength) {
    return EdgeLight(EdgeMode::Ambient,
                     detail::tintEdgeArgb(config::Render::EDGE_TINT_AMBIENT),
                     config::Render::EDGE_ANGLE_DEFAULT,
                     config::Render::EDGE_FALLOFF_DEFAULT,
                     config::Render::EDGE_AMBIENT_WIDTH,
                     config::Render::EDGE_AMBIENT_SOFTNESS,
                     clampRange(strength, 0.0f, 1.0f));
}

EdgeLight EdgeLight::contour(float angleDegrees, float falloff, unsigned int tint) {
    return EdgeLight(EdgeMode::Contour, tint, angleDegrees, falloff,
                     config::Render::EDGE_CONTOUR_WIDTH,
                     config::Render::EDGE_CONTOUR_SOFTNESS,
                     config::Render::EDGE_CONTOUR_ALPHA);
}

}
