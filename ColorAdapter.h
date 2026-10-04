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

namespace lgx {
namespace detail {

void resetTint();
void applyThemeColors(const GlassColors& colors, bool animated);
void advanceTint(float dt);
float tintBlend();
Rgba tintPrimary();
Rgba tintSecondary();
Rgba tintSheet();
float tintSheetAlpha();
Rgba tintButtonSheet();
Rgba tintScrim();
float tintScrimAlpha();
Rgba tintEdge();
unsigned int tintEdgeArgb(unsigned int base);
Rgba tintKnob();
Rgba tintTrackNeutral();
Rgba tintFallback();
Rgba tintRefract();
float tintBrightness();
float tintSaturation();
float tintContrast();
unsigned int tintTextRgb();
unsigned int tintScrimRgb();
unsigned int tintSpectrumRgb();
unsigned int tintTrackArgb();
unsigned int tintMixRgb(unsigned int dark, unsigned int light);

bool lightFlag();
Rgba tintBase();

}
}
