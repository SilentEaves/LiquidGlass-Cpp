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
#include "ColorAdapter.h"

#include "GlassInternal.h"

#include <math.h>

namespace lgx {
namespace detail {
namespace {

Rgba blendColor(const Rgba& from, const Rgba& to, float t) {
    return Rgba(lerp(from.r, to.r, t), lerp(from.g, to.g, t), lerp(from.b, to.b, t),
                lerp(from.a, to.a, t));
}

float luma(const Rgba& color) {
    return 0.213f * color.r + 0.715f * color.g + 0.072f * color.b;
}

Rgba scaled(const Rgba& color, float gain) {
    return Rgba(saturate(color.r * gain), saturate(color.g * gain),
                saturate(color.b * gain), color.a);
}

Rgba white() { return Rgba(1.0f, 1.0f, 1.0f, 1.0f); }

Rgba black() { return Rgba(0.0f, 0.0f, 0.0f, 1.0f); }

float lightnessOf(const Rgba& color) {
    return smoothStep01((luma(color) - config::Color::LUMA_BAND_LO)
                        / (config::Color::LUMA_BAND_HI - config::Color::LUMA_BAND_LO));
}

Rgba poleOf(const Rgba& base) {
    return luma(base) > config::Color::LUMA_FLIP ? black() : white();
}

Rgba contrastPair(const Rgba& base) {
    const float sharp = smoothStep01(lightnessOf(base));
    const float lift = lerp(config::Color::TEXT_LIFT_DARK, config::Color::TEXT_LIFT_LIGHT,
                            sharp);
    return Rgba(lift, lift, lift, 1.0f);
}

Rgba defaultPrimary() {
    return Rgba(config::Color::DARK_PRIMARY_R, config::Color::DARK_PRIMARY_G,
                config::Color::DARK_PRIMARY_B, 1.0f);
}

Rgba defaultSecondary() {
    return Rgba(config::Color::DARK_SECONDARY_R, config::Color::DARK_SECONDARY_G,
                config::Color::DARK_SECONDARY_B, 1.0f);
}

Rgba defaultLightPrimary() {
    return Rgba(config::Color::LIGHT_PRIMARY_R, config::Color::LIGHT_PRIMARY_G,
                config::Color::LIGHT_PRIMARY_B, 1.0f);
}

Rgba g_from = defaultPrimary();
Rgba g_to = defaultPrimary();
Rgba g_primary = defaultPrimary();
Rgba g_secondFrom = defaultSecondary();
Rgba g_secondTo = defaultSecondary();
Rgba g_secondary = defaultSecondary();
float g_progress = 1.0f;

}

void resetTint() {
    g_from = defaultPrimary();
    g_to = g_from;
    g_primary = g_from;
    g_secondFrom = defaultSecondary();
    g_secondTo = g_secondFrom;
    g_secondary = g_secondFrom;
    g_progress = 1.0f;
}

void applyThemeColors(const GlassColors& colors, bool animated) {
    g_from = g_primary;
    g_secondFrom = g_secondary;
    g_to = colors.primary;
    g_secondTo = colors.secondary;
    const bool same = g_from.r == g_to.r && g_from.g == g_to.g && g_from.b == g_to.b
                   && g_secondFrom.r == g_secondTo.r && g_secondFrom.g == g_secondTo.g
                   && g_secondFrom.b == g_secondTo.b;
    if (!animated || same) {
        g_primary = g_to;
        g_secondary = g_secondTo;
        g_progress = 1.0f;
    }
    else {
        g_progress = 0.0f;
    }
}

void advanceTint(float dt) {
    if (g_progress >= 1.0f) return;
    const float step = (dt > 0.0f ? dt : config::Motion::FRAME_FALLBACK)
                     / config::Color::SWITCH_SECONDS;
    g_progress = g_progress + step > 1.0f ? 1.0f : g_progress + step;
    const float eased = curveAt(g_progress);
    g_primary = blendColor(g_from, g_to, eased);
    g_secondary = blendColor(g_secondFrom, g_secondTo, eased);
}

float tintBlend() { return smoothStep01(lightnessOf(g_primary)); }

bool lightFlag() { return tintBlend() >= config::Color::LUMA_FLIP; }

float tintDeviation() {
    const Rgba reference = blendColor(defaultPrimary(), defaultLightPrimary(), tintBlend());
    float deviation = fabsf(g_primary.r - reference.r);
    const float devG = fabsf(g_primary.g - reference.g);
    const float devB = fabsf(g_primary.b - reference.b);
    if (devG > deviation) deviation = devG;
    if (devB > deviation) deviation = devB;
    return saturate(deviation);
}

Rgba tintBase() {
    const float bridge = config::Color::HUE_BRIDGE
        * saturate(tintDeviation() / config::Color::HUE_BRIDGE_REF);
    return blendColor(g_secondary, g_primary, bridge);
}

Rgba tintPrimary() { return g_primary; }

Rgba tintSecondary() { return g_secondary; }

Rgba tintSheet() {
    const Rgba base = scaled(tintBase(), config::Color::SHEET_GAIN);
    const float light = tintBlend();
    const float mix = lerp(config::Color::SHEET_MIX_DARK, config::Color::SHEET_MIX_LIGHT,
                           light);
    const Rgba sheet = blendColor(base, white(), mix);
    const float pull = lerp(config::Color::SHEET_PULL_DARK, config::Color::SHEET_PULL_LIGHT,
                            light);
    return blendColor(sheet, g_primary, pull);
}

float tintSheetAlpha() {
    return lerp(config::Color::SHEET_ALPHA_DARK, config::Color::SHEET_ALPHA_LIGHT,
                tintBlend());
}

Rgba tintButtonSheet() {
    const Rgba base = scaled(tintBase(), config::Color::BUTTON_SHEET_GAIN);
    const float light = tintBlend();
    const float mix = lerp(config::Color::BUTTON_SHEET_MIX_DARK,
                           config::Color::BUTTON_SHEET_MIX_LIGHT, light);
    Rgba sheet = blendColor(base, white(), mix);
    sheet = blendColor(sheet, g_primary, config::Color::BUTTON_SHEET_PULL);
    sheet = blendColor(sheet, white(),
                       config::Color::BUTTON_SHEET_LIFT_DARK * (1.0f - light));
    sheet.a = lerp(config::Color::BUTTON_SHEET_ALPHA_DARK,
                   config::Color::BUTTON_SHEET_ALPHA_LIGHT, light);
    return sheet;
}

Rgba tintScrim() { return scaled(g_primary, config::Color::SCRIM_GAIN); }

float tintScrimAlpha() {
    return lerp(config::Color::SCRIM_ALPHA_DARK, config::Color::SCRIM_ALPHA_LIGHT,
                tintBlend());
}

Rgba tintEdge() {
    const Rgba sheen = scaled(tintBase(), config::Color::EDGE_GAIN);
    const float light = tintBlend();
    const float mix = lerp(config::Color::EDGE_MIX_DARK, config::Color::EDGE_MIX_LIGHT,
                           light);
    return blendColor(sheen, poleOf(g_primary), mix);
}

unsigned int tintEdgeArgb(unsigned int base) {
    const Rgba tint = tintEdge();
    const unsigned int a = (base >> 24) & 0xFFu;
    const unsigned int r = (unsigned int)(saturate(tint.r) * config::COLOR_CHANNEL + 0.5f);
    const unsigned int g = (unsigned int)(saturate(tint.g) * config::COLOR_CHANNEL + 0.5f);
    const unsigned int b = (unsigned int)(saturate(tint.b) * config::COLOR_CHANNEL + 0.5f);
    return (a << 24) | (r << 16) | (g << 8) | b;
}

Rgba tintKnob() {
    const float mix = lerp(config::Color::KNOB_MIX_DARK, config::Color::KNOB_MIX_LIGHT,
                           tintBlend());
    return blendColor(white(), g_primary, mix);
}

Rgba tintTrackNeutral() {
    const Rgba target = blendColor(poleOf(g_primary), black(), tintBlend());
    return blendColor(tintBase(), target, config::Color::TRACK_MIX);
}

Rgba tintFallback() { return Rgba(g_primary.r, g_primary.g, g_primary.b, 1.0f); }

Rgba tintRefract() {
    return Rgba(g_primary.r, g_primary.g, g_primary.b,
                tintDeviation() * config::Color::REFRACT_GAIN);
}

float tintBrightness() {
    return lerp(config::Color::BRIGHTNESS_DARK, config::Color::BRIGHTNESS_LIGHT,
                tintBlend());
}

float tintSaturation() {
    return lerp(config::Color::SATURATION_DARK, config::Color::SATURATION_LIGHT,
                tintBlend());
}

float tintContrast() {
    return lerp(config::Color::CONTRAST_DARK, config::Color::CONTRAST_LIGHT,
                tintBlend());
}

unsigned int tintMixRgb(unsigned int dark, unsigned int light) {
    return mixRgb(dark, light, tintBlend());
}

unsigned int tintTextRgb() {
    const Rgba value = contrastPair(g_primary);
    const unsigned int r = (unsigned int)(saturate(value.r) * config::COLOR_CHANNEL + 0.5f);
    const unsigned int g = (unsigned int)(saturate(value.g) * config::COLOR_CHANNEL + 0.5f);
    const unsigned int b = (unsigned int)(saturate(value.b) * config::COLOR_CHANNEL + 0.5f);
    return (r << 16) | (g << 8) | b;
}

unsigned int tintScrimRgb() {
    const Rgba value = tintScrim();
    const unsigned int r = (unsigned int)(saturate(value.r) * config::COLOR_CHANNEL + 0.5f);
    const unsigned int g = (unsigned int)(saturate(value.g) * config::COLOR_CHANNEL + 0.5f);
    const unsigned int b = (unsigned int)(saturate(value.b) * config::COLOR_CHANNEL + 0.5f);
    return (r << 16) | (g << 8) | b;
}

unsigned int tintSpectrumRgb() {
    const Rgba sheen = scaled(tintBase(), config::Color::SPECTRUM_GAIN);
    const Rgba target = blendColor(white(), black(), tintBlend());
    const Rgba value = blendColor(sheen, target, config::Color::SPECTRUM_MIX);
    const unsigned int r = (unsigned int)(saturate(value.r) * config::COLOR_CHANNEL + 0.5f);
    const unsigned int g = (unsigned int)(saturate(value.g) * config::COLOR_CHANNEL + 0.5f);
    const unsigned int b = (unsigned int)(saturate(value.b) * config::COLOR_CHANNEL + 0.5f);
    return (r << 16) | (g << 8) | b;
}

unsigned int tintTrackArgb() {
    const Rgba value = tintTrackNeutral();
    const unsigned int a = (unsigned int)(config::Render::TRACK_NEUTRAL_ALPHA
                                          * config::COLOR_CHANNEL + 0.5f);
    const unsigned int r = (unsigned int)(saturate(value.r) * config::COLOR_CHANNEL + 0.5f);
    const unsigned int g = (unsigned int)(saturate(value.g) * config::COLOR_CHANNEL + 0.5f);
    const unsigned int b = (unsigned int)(saturate(value.b) * config::COLOR_CHANNEL + 0.5f);
    return (a << 24) | (r << 16) | (g << 8) | b;
}

}
}
