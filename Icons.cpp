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
namespace {

constexpr int RING_SAMPLE_LIMIT = config::Layout::ICON_RING_LIMIT;

void strokeCircle(Canvas& canvas, const Point& center, float radius, unsigned int color,
                  float thickness, int segments) {
    int samples = segments;
    if (samples < config::Layout::ICON_MIN_SEGMENTS) samples = config::Layout::ICON_MIN_SEGMENTS;
    if (samples > RING_SAMPLE_LIMIT) samples = RING_SAMPLE_LIMIT;
    Point ring[RING_SAMPLE_LIMIT + 1];
    const int count = arcOutline(center, radius, 0.0f, config::TWO_PI, samples, ring,
                                 RING_SAMPLE_LIMIT + 1);
    if (count <= 0) return;
    ring[count] = ring[0];
    canvas.strokePoly(ring, count + 1, thickness, color);
}

float strokeWidth(float size, float ratio) {
    const float scaled = size * ratio;
    return scaled > config::Layout::ICON_MIN_STROKE_PX ? scaled
                                                       : config::Layout::ICON_MIN_STROKE_PX;
}

}
}

void iconHome(Canvas& canvas, const Point& center, float size, unsigned int color, void*) {
    const float unit = size * config::HALF;
    const float apex = unit * config::Layout::ICON_HOME_APEX;
    const float shoulder = unit * config::Layout::ICON_HOME_SHOULDER;
    const float eave = unit * config::Layout::ICON_HOME_EAVE;
    const float wall = unit * config::Layout::ICON_HOME_WALL;
    const float door = unit * config::Layout::ICON_HOME_DOOR;
    const float lintel = unit * config::Layout::ICON_HOME_LINTEL;
    const Point outline[9] = {
        Point(center.x, center.y - apex),
        Point(center.x + apex, center.y - eave),
        Point(center.x + shoulder, center.y + wall),
        Point(center.x + door, center.y + wall),
        Point(center.x + door, center.y + lintel),
        Point(center.x - door, center.y + lintel),
        Point(center.x - door, center.y + wall),
        Point(center.x - shoulder, center.y + wall),
        Point(center.x - apex, center.y - eave)
    };
    canvas.fillConcave(outline, 9, color);
}

void iconSearch(Canvas& canvas, const Point& center, float size, unsigned int color, void*) {
    const float thickness = size * config::Layout::ICON_SEARCH_STROKE;
    const float radius = size * config::Layout::ICON_SEARCH_RING;
    const float hub = size * config::Layout::ICON_SEARCH_HUB;
    const float touch = radius * config::Layout::ICON_SEARCH_TOUCH;
    const float handle = size * config::Layout::ICON_SEARCH_HANDLE;
    const Point centerHub(center.x - hub, center.y - hub);
    detail::strokeCircle(canvas, centerHub, radius, color, thickness,
                 config::Layout::ICON_SEARCH_SEGMENTS);
    const Point handleStart(centerHub.x + touch, centerHub.y + touch);
    const Point handleEnd(center.x + handle, center.y + handle);
    canvas.line(handleStart, handleEnd, color, thickness);
    canvas.fillCircle(handleStart, thickness * config::HALF, color,
                      config::Layout::ICON_DOT_SEGMENTS);
    canvas.fillCircle(handleEnd, thickness * config::HALF, color,
                      config::Layout::ICON_DOT_SEGMENTS);
}

void iconMessage(Canvas& canvas, const Point& center, float size, unsigned int color, void*) {
    const float unit = size * config::HALF;
    const float top = center.y - unit * config::Layout::ICON_MESSAGE_TOP;
    const float bottom = center.y + unit * config::Layout::ICON_MESSAGE_BOTTOM;
    const float body = unit * config::Layout::ICON_MESSAGE_BODY;
    canvas.fillRect(Box(center.x - body, top, center.x + body, bottom),
                    unit * config::Layout::ICON_MESSAGE_ROUND, color);
    const float tailNear = unit * config::Layout::ICON_MESSAGE_TAIL_NEAR;
    const Point tail[3] = {
        Point(center.x - unit * config::Layout::ICON_MESSAGE_TAIL_FAR, bottom - tailNear),
        Point(center.x - unit * config::Layout::ICON_MESSAGE_TAIL_MID,
              center.y + unit * config::Layout::ICON_MESSAGE_TAIL),
        Point(center.x - unit * config::Layout::ICON_MESSAGE_TAIL_NEAR, bottom - tailNear)
    };
    canvas.fillPoly(tail, 3, color);
}

void iconProfile(Canvas& canvas, const Point& center, float size, unsigned int color, void*) {
    const float unit = size * config::HALF;
    canvas.fillCircle(Point(center.x, center.y - unit * config::Layout::ICON_PROFILE_HEAD_Y),
                      unit * config::Layout::ICON_PROFILE_HEAD, color,
                      config::Layout::ICON_PROFILE_SEGMENTS);
    canvas.fillArc(Point(center.x,
                         center.y + unit * config::Layout::ICON_PROFILE_SHOULDER_Y),
                   unit * config::Layout::ICON_PROFILE_SHOULDER, config::PI, config::TWO_PI,
                   color, config::Layout::ICON_PROFILE_SEGMENTS);
}

void iconHeart(Canvas& canvas, const Point& center, float size, unsigned int color, void*) {
    const float radius = size * config::Layout::ICON_HEART_LOBE;
    const float spread = radius * config::Layout::ICON_HEART_SPREAD;
    const float top = size * config::Layout::ICON_HEART_TOP;
    const float brim = radius * config::Layout::ICON_HEART_BRIM;
    const float base = size * config::Layout::ICON_HEART_BASE;
    canvas.fillCircle(Point(center.x - spread, center.y - top), radius, color,
                      config::Layout::ICON_HEART_SEGMENTS);
    canvas.fillCircle(Point(center.x + spread, center.y - top), radius, color,
                      config::Layout::ICON_HEART_SEGMENTS);
    canvas.fillTriangle(Point(center.x - brim, center.y - base),
                        Point(center.x + brim, center.y - base),
                        Point(center.x, center.y + size * config::Layout::ICON_HEART_TIP),
                        color);
}

void iconStar(Canvas& canvas, const Point& center, float size, unsigned int color, void*) {
    const float outer = size * config::Layout::ICON_STAR_OUTER;
    const float inner = outer * config::Layout::STAR_INNER_RATIO;
    const int arms = config::Layout::STAR_ARMS;
    Point tips[config::Layout::STAR_ARMS * 2];
    const float step = config::TWO_PI / (float)arms;
    const float half = step * config::HALF;
    for (int i = 0; i < arms; ++i) {
        const float outerAngle = -config::HALF_PI + (float)i * step;
        const float innerAngle = outerAngle + half;
        tips[i * 2] = Point(center.x + cosf(outerAngle) * outer,
                            center.y + sinf(outerAngle) * outer);
        tips[i * 2 + 1] = Point(center.x + cosf(innerAngle) * inner,
                                center.y + sinf(innerAngle) * inner);
    }
    for (int i = 0; i < arms; ++i) {
        const int next = (i + 1) % arms;
        canvas.fillTriangle(center, tips[i * 2], tips[i * 2 + 1], color);
        canvas.fillTriangle(center, tips[i * 2 + 1], tips[next * 2], color);
    }
}

void iconBell(Canvas& canvas, const Point& center, float size, unsigned int color, void*) {
    const float unit = size * config::HALF;
    canvas.fillArc(Point(center.x, center.y + unit * config::Layout::ICON_BELL_DOME_Y),
                   unit * config::Layout::ICON_BELL_DOME, config::PI, config::TWO_PI, color,
                   config::Layout::ICON_BELL_SEGMENTS);
    canvas.fillRect(Box(center.x - unit * config::Layout::ICON_BELL_LIP,
                        center.y + unit * config::Layout::ICON_BELL_LIP_TOP,
                        center.x + unit * config::Layout::ICON_BELL_LIP,
                        center.y + unit * config::Layout::ICON_BELL_LIP_BOTTOM),
                    unit * config::Layout::ICON_BELL_LIP_ROUND, color);
    canvas.fillCircle(Point(center.x, center.y + unit * config::Layout::ICON_BELL_CLAPPER_Y),
                      unit * config::Layout::ICON_BELL_CLAPPER, color,
                      config::Layout::ICON_SMALL_DOT_SEGMENTS);
    canvas.fillCircle(Point(center.x, center.y - unit * config::Layout::ICON_BELL_CROWN_Y),
                      unit * config::Layout::ICON_BELL_CROWN, color,
                      config::Layout::ICON_DOT_SEGMENTS);
}

void iconGear(Canvas& canvas, const Point& center, float size, unsigned int color, void*) {
    const float unit = size * config::HALF;
    const int teeth = config::Layout::ICON_GEAR_TEETH;
    const float step = config::TWO_PI / (float)teeth;
    const float inner = unit * config::Layout::ICON_GEAR_INNER;
    const float outer = unit * config::Layout::ICON_GEAR_OUTER;
    for (int i = 0; i < teeth; ++i) {
        const float angle = (float)i * step;
        const float cosine = cosf(angle);
        const float sine = sinf(angle);
        canvas.line(Point(center.x + cosine * inner, center.y + sine * inner),
                    Point(center.x + cosine * outer, center.y + sine * outer),
                    color, unit * config::Layout::ICON_GEAR_TOOTH_W);
    }
    detail::strokeCircle(canvas, center, unit * config::Layout::ICON_GEAR_HUB, color,
                 unit * config::Layout::ICON_GEAR_HUB_W,
                 config::Layout::ICON_GEAR_SEGMENTS);
}

void iconClose(Canvas& canvas, const Point& center, float size, unsigned int color, void*) {
    const float arm = size * config::Layout::ICON_CLOSE_ARM;
    const float thickness = detail::strokeWidth(size, config::Layout::ICON_CLOSE_WIDTH);
    canvas.line(Point(center.x - arm, center.y - arm),
                Point(center.x + arm, center.y + arm), color, thickness);
    canvas.line(Point(center.x - arm, center.y + arm),
                Point(center.x + arm, center.y - arm), color, thickness);
}

void iconCheck(Canvas& canvas, const Point& center, float size, unsigned int color, void*) {
    const float arm = size * config::Layout::ICON_CHECK_ARM;
    const float thickness = detail::strokeWidth(size, config::Layout::ICON_CHECK_WIDTH);
    const Point elbow(center.x - arm * config::Layout::ICON_CHECK_ELBOW,
                      center.y + arm * config::Layout::ICON_CHECK_VALLEY);
    canvas.line(Point(center.x - arm, center.y + arm * config::Layout::ICON_CHECK_START),
                elbow, color, thickness);
    canvas.line(elbow,
                Point(center.x + arm, center.y - arm * config::Layout::ICON_CHECK_END),
                color, thickness);
}

void iconChevron(Canvas& canvas, const Point& center, float size, unsigned int color, void*) {
    const float arm = size * config::Layout::ICON_CHEVRON_ARM;
    const float thickness = detail::strokeWidth(size, config::Layout::ICON_CHEVRON_WIDTH);
    const Point tip(center.x, center.y + arm * config::Layout::ICON_CHEVRON_TIP);
    canvas.line(Point(center.x - arm, center.y - arm * config::Layout::ICON_CHEVRON_TOP),
                tip, color, thickness);
    canvas.line(tip,
                Point(center.x + arm, center.y - arm * config::Layout::ICON_CHEVRON_TOP),
                color, thickness);
}

}
