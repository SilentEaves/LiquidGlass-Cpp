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

float tangentScale(float span) {
    return config::Layout::CORNER_ARC_TANGENT * (span / config::HALF_PI);
}

}

float cubicPoint(float a, float b, float c, float d, float t) {
    const float inverse = 1.0f - t;
    const float inverse2 = inverse * inverse;
    const float t2 = t * t;
    return a * inverse2 * inverse + 3.0f * b * inverse2 * t + 3.0f * c * inverse * t2
         + d * t2 * t;
}

int arcOutline(const Point& center, float radius, float fromRadians, float toRadians,
               int samples, Point* out, int capacity) {
    if (out == nullptr || capacity < samples) return 0;
    const int pieces = config::Layout::ARC_BEZIER_PIECES;
    if (samples < pieces * 2) return 0;

    const int perPiece = samples / pieces;
    const float span = (toRadians - fromRadians) / (float)pieces;
    const float control = tangentScale(span);

    int cursor = 0;
    for (int piece = 0; piece < pieces; ++piece) {
        const float fromAngle = fromRadians + span * (float)piece;
        const float toAngle = fromAngle + span;
        const float fromCos = cosf(fromAngle);
        const float fromSin = sinf(fromAngle);
        const float toCos = cosf(toAngle);
        const float toSin = sinf(toAngle);

        const float startX = center.x + fromCos * radius;
        const float startY = center.y + fromSin * radius;
        const float endX = center.x + toCos * radius;
        const float endY = center.y + toSin * radius;
        const float leadX = startX - fromSin * radius * control;
        const float leadY = startY + fromCos * radius * control;
        const float trailX = endX + toSin * radius * control;
        const float trailY = endY - toCos * radius * control;

        for (int step = 0; step < perPiece; ++step) {
            const float t = (float)step / (float)perPiece;
            out[cursor].x = cubicPoint(startX, leadX, trailX, endX, t);
            out[cursor].y = cubicPoint(startY, leadY, trailY, endY, t);
            ++cursor;
        }
    }
    return cursor;
}

}
}
