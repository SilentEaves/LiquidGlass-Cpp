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

#include <GLES3/gl3.h>
#include <math.h>

namespace lgx {
namespace detail {
namespace {

constexpr int TAP_COUNT = config::Render::BLUR_TAPS;

enum PanelSlot {
    P_FRAME_RECT = 0,
    P_VIEW_SPAN,
    P_SCENE,
    P_GATHERED,
    P_GATHERED_ON,
    P_GATHER_PITCH,
    P_GATHER_ROWS,
    P_GATHER_TEXEL,
    P_GATHER_LIMIT,
    P_PANEL_SPAN,
    P_SAMPLE_SHIFT,
    P_SAMPLE_SPAN,
    P_SAMPLE_TEXEL,
    P_CORNER_RADII,
    P_DROP_RECT,
    P_DROP_ROUND,
    P_FEED_RECT,
    P_FEED_ROUND,
    P_FEED_MERGE,
    P_STRAND_RECT,
    P_STRAND_ROUND,
    P_STRAND_MERGE,
    P_LENS_HEIGHT,
    P_LENS_AMOUNT,
    P_LENS_DEPTH,
P_DARK_GUARD,
    P_SPECTRAL,
    P_PRISM_DIST,
    P_PRISM_EVEN_RED,
    P_PRISM_ODD_RED,
    P_PRISM_EVEN_GREEN,
    P_PRISM_ODD_GREEN,
    P_PRISM_EVEN_BLUE,
    P_PRISM_ODD_BLUE,
    P_PRISM_CENTER_GREEN,
    P_PANEL_OPACITY,
    P_TONE_CTRL,
    P_TONE_BOOST,
    P_REFRACT_TINT,
    P_SURFACE_TINT,
    P_SURFACE_MODE,
    P_EDGE_MODE,
    P_EDGE_TINT,
    P_EDGE_TINT_ALPHA,
    P_EDGE_ANGLE,
    P_EDGE_FALLOFF,
    P_EDGE_ALPHA,
    P_EDGE_WIDTH,
    P_EDGE_BLUR,
    P_INSET_TINT,
    P_INSET_SHIFT,
    P_INSET_REACH,
    P_INSET_ALPHA,
    P_GLOW_ON,
    P_GLOW_AT,
    P_GLOW_REACH,
    P_GLOW_LEVEL,
    P_SCENE_ON,
    P_SOLID_SCENE,
    P_STRETCH_SCALE,
    P_STRETCH_MIX,
    P_ABSENT_TINT,
    P_TRACK_ON,
    P_TRACK_RECT,
    P_TRACK_ROUND,
    P_TRACK_TINT,
    P_SHADE_MODE,
    P_SHADE_TINT,
    P_SHADE_SHIFT,
    P_SHADE_REACH,
    P_SHADE_BODY,
    P_SLOT_COUNT
};

enum GatherSlot {
    G_FRAME_RECT = 0,
    G_VIEW_SPAN,
    G_SOURCE,
    G_SOURCE_TEXEL,
    G_REGION_SPAN,
    G_AXIS_DIR,
    G_AXIS_STEP,
    G_TAP_MASS,
    G_FBO_PITCH,
    G_BOX_HALF,
    G_SLOT_COUNT
};

template <int Capacity>
class UniformTable {
public:
    void declare(int slot, GLuint program, const char* name) {
        if (slot < 0 || slot >= Capacity) return;
        location_[slot] = glGetUniformLocation(program, name);
        known_[slot] = false;
    }

    GLint location(int slot) const {
        return (slot >= 0 && slot < Capacity) ? location_[slot] : -1;
    }

    void set(int slot, const float* values, int count) {
        if (slot < 0 || slot >= Capacity) return;
        const GLint handle = location_[slot];
        if (handle < 0) return;
        float* stored = value_[slot];
        if (known_[slot]) {
            bool unchanged = true;
            for (int i = 0; i < count; ++i) {
                if (stored[i] != values[i]) { unchanged = false; break; }
            }
            if (unchanged) return;
        }
        for (int i = 0; i < count; ++i) stored[i] = values[i];
        known_[slot] = true;
        switch (count) {
            case 1: glUniform1f(handle, values[0]); break;
            case 2: glUniform2f(handle, values[0], values[1]); break;
            case 3: glUniform3f(handle, values[0], values[1], values[2]); break;
            default: glUniform4f(handle, values[0], values[1], values[2], values[3]); break;
        }
    }

    void set1(int slot, float a) {
        const float v[1] = {a};
        set(slot, v, 1);
    }

    void set2(int slot, float a, float b) {
        const float v[2] = {a, b};
        set(slot, v, 2);
    }

    void set3(int slot, float a, float b, float c) {
        const float v[3] = {a, b, c};
        set(slot, v, 3);
    }

    void set4(int slot, float a, float b, float c, float d) {
        const float v[4] = {a, b, c, d};
        set(slot, v, 4);
    }

    void setColor(int slot, const Rgba& color) {
        set4(slot, color.r, color.g, color.b, color.a);
    }

    void setArray(int slot, const float* values, int count) {
        if (slot < 0 || slot >= Capacity) return;
        const GLint handle = location_[slot];
        if (handle < 0) return;
        glUniform1fv(handle, count, values);
    }

private:
    GLint location_[Capacity] = {};
    float value_[Capacity][4] = {};
    bool known_[Capacity] = {};
};

GLuint g_panelProgram = 0;
GLuint g_gatherProgram = 0;
UniformTable<P_SLOT_COUNT> g_panelUniforms;
UniformTable<G_SLOT_COUNT> g_gatherUniforms;

GLuint g_captureTexture = 0;
int g_captureMaxStamp = -1;
int g_captureMaxW = 0;
int g_captureMaxH = 0;
int g_captureWidth = 0;
int g_captureHeight = 0;
int g_captureShrinkStamp = -1;

GLuint g_gatherFbo[2] = {0, 0};
GLuint g_gatherTexture[2] = {0, 0};
int g_gatherAllocW[2] = {0, 0};
int g_gatherAllocH[2] = {0, 0};

GatherInfo g_gatherInfo;

int g_frameStamp = 0;
bool g_ready = false;

float g_tapMass[TAP_COUNT] = {};

GLuint compileStage(GLenum type, const char* source) {
    const GLuint shader = glCreateShader(type);
    if (shader == 0) return 0;
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);
    GLint status = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (status != GL_TRUE) {
        glDeleteShader(shader);
        return GLuint(0);
    }
    return shader;
}

GLuint linkPair(const char* vertexSource, const char* fragmentSource) {
    const GLuint vertex = compileStage(GL_VERTEX_SHADER, vertexSource);
    const GLuint fragment = compileStage(GL_FRAGMENT_SHADER, fragmentSource);
    if (vertex == 0 || fragment == 0) {
        glDeleteShader(vertex);
        glDeleteShader(fragment);
        return GLuint(0);
    }
    const GLuint program = glCreateProgram();
    glAttachShader(program, vertex);
    glAttachShader(program, fragment);
    glLinkProgram(program);
    glDeleteShader(vertex);
    glDeleteShader(fragment);
    GLint status = GL_FALSE;
    glGetProgramiv(program, GL_LINK_STATUS, &status);
    if (status != GL_TRUE) {
        glDeleteProgram(program);
        return GLuint(0);
    }
    return program;
}

void declarePanelUniforms(GLuint program) {
    g_panelUniforms.declare(P_FRAME_RECT, program, "uFrameRect");
    g_panelUniforms.declare(P_VIEW_SPAN, program, "uViewSpan");
    g_panelUniforms.declare(P_SCENE, program, "uScene");
    g_panelUniforms.declare(P_GATHERED, program, "uGathered");
    g_panelUniforms.declare(P_GATHERED_ON, program, "uGatheredOn");
    g_panelUniforms.declare(P_GATHER_PITCH, program, "uGatherPitch");
    g_panelUniforms.declare(P_GATHER_ROWS, program, "uGatherRows");
    g_panelUniforms.declare(P_GATHER_TEXEL, program, "uGatherTexel");
    g_panelUniforms.declare(P_GATHER_LIMIT, program, "uGatherLimit");
    g_panelUniforms.declare(P_PANEL_SPAN, program, "uPanelSpan");
    g_panelUniforms.declare(P_SAMPLE_SHIFT, program, "uSampleShift");
    g_panelUniforms.declare(P_SAMPLE_SPAN, program, "uSampleSpan");
    g_panelUniforms.declare(P_SAMPLE_TEXEL, program, "uSampleTexel");
    g_panelUniforms.declare(P_CORNER_RADII, program, "uCornerRadii");
    g_panelUniforms.declare(P_DROP_RECT, program, "uDropRect");
    g_panelUniforms.declare(P_DROP_ROUND, program, "uDropRound");
    g_panelUniforms.declare(P_FEED_RECT, program, "uFeedRect");
    g_panelUniforms.declare(P_FEED_ROUND, program, "uFeedRound");
    g_panelUniforms.declare(P_FEED_MERGE, program, "uFeedMerge");
    g_panelUniforms.declare(P_STRAND_RECT, program, "uStrandRect");
    g_panelUniforms.declare(P_STRAND_ROUND, program, "uStrandRound");
    g_panelUniforms.declare(P_STRAND_MERGE, program, "uStrandMerge");
    g_panelUniforms.declare(P_LENS_HEIGHT, program, "uLensHeight");
    g_panelUniforms.declare(P_LENS_AMOUNT, program, "uLensAmount");
    g_panelUniforms.declare(P_LENS_DEPTH, program, "uLensDepth");
    g_panelUniforms.declare(P_DARK_GUARD, program, "uDarkGuard");
    g_panelUniforms.declare(P_SPECTRAL, program, "uSpectral");
    g_panelUniforms.declare(P_PRISM_DIST, program, "uPrismDist");
    g_panelUniforms.declare(P_PRISM_EVEN_RED, program, "uPrismEvenRed");
    g_panelUniforms.declare(P_PRISM_ODD_RED, program, "uPrismOddRed");
    g_panelUniforms.declare(P_PRISM_EVEN_GREEN, program, "uPrismEvenGreen");
    g_panelUniforms.declare(P_PRISM_ODD_GREEN, program, "uPrismOddGreen");
    g_panelUniforms.declare(P_PRISM_EVEN_BLUE, program, "uPrismEvenBlue");
    g_panelUniforms.declare(P_PRISM_ODD_BLUE, program, "uPrismOddBlue");
    g_panelUniforms.declare(P_PRISM_CENTER_GREEN, program, "uPrismCenterGreen");
    g_panelUniforms.declare(P_PANEL_OPACITY, program, "uPanelOpacity");
    g_panelUniforms.declare(P_TONE_CTRL, program, "uToneCtrl");
    g_panelUniforms.declare(P_TONE_BOOST, program, "uToneBoost");
    g_panelUniforms.declare(P_REFRACT_TINT, program, "uRefractTint");
    g_panelUniforms.declare(P_SURFACE_TINT, program, "uSurfaceTint");
    g_panelUniforms.declare(P_SURFACE_MODE, program, "uSurfaceMode");
    g_panelUniforms.declare(P_EDGE_MODE, program, "uEdgeMode");
    g_panelUniforms.declare(P_EDGE_TINT, program, "uEdgeTint");
    g_panelUniforms.declare(P_EDGE_TINT_ALPHA, program, "uEdgeTintAlpha");
    g_panelUniforms.declare(P_EDGE_ANGLE, program, "uEdgeAngle");
    g_panelUniforms.declare(P_EDGE_FALLOFF, program, "uEdgeFalloff");
    g_panelUniforms.declare(P_EDGE_ALPHA, program, "uEdgeAlpha");
    g_panelUniforms.declare(P_EDGE_WIDTH, program, "uEdgeWidth");
    g_panelUniforms.declare(P_EDGE_BLUR, program, "uEdgeBlur");
    g_panelUniforms.declare(P_INSET_TINT, program, "uInsetTint");
    g_panelUniforms.declare(P_INSET_SHIFT, program, "uInsetShift");
    g_panelUniforms.declare(P_INSET_REACH, program, "uInsetReach");
    g_panelUniforms.declare(P_INSET_ALPHA, program, "uInsetAlpha");
    g_panelUniforms.declare(P_GLOW_ON, program, "uGlowOn");
    g_panelUniforms.declare(P_GLOW_AT, program, "uGlowAt");
    g_panelUniforms.declare(P_GLOW_REACH, program, "uGlowReach");
    g_panelUniforms.declare(P_GLOW_LEVEL, program, "uGlowLevel");
    g_panelUniforms.declare(P_SCENE_ON, program, "uSceneOn");
    g_panelUniforms.declare(P_SOLID_SCENE, program, "uSolidScene");
    g_panelUniforms.declare(P_STRETCH_SCALE, program, "uStretchScale");
    g_panelUniforms.declare(P_STRETCH_MIX, program, "uStretchMix");
    g_panelUniforms.declare(P_ABSENT_TINT, program, "uAbsentTint");
    g_panelUniforms.declare(P_TRACK_ON, program, "uTrackOn");
    g_panelUniforms.declare(P_TRACK_RECT, program, "uTrackRect");
    g_panelUniforms.declare(P_TRACK_ROUND, program, "uTrackRound");
    g_panelUniforms.declare(P_TRACK_TINT, program, "uTrackTint");
    g_panelUniforms.declare(P_SHADE_MODE, program, "uShadeMode");
    g_panelUniforms.declare(P_SHADE_TINT, program, "uShadeTint");
    g_panelUniforms.declare(P_SHADE_SHIFT, program, "uShadeShift");
    g_panelUniforms.declare(P_SHADE_REACH, program, "uShadeReach");
    g_panelUniforms.declare(P_SHADE_BODY, program, "uShadeBody");
}

void declareGatherUniforms(GLuint program) {
    g_gatherUniforms.declare(G_FRAME_RECT, program, "uFrameRect");
    g_gatherUniforms.declare(G_VIEW_SPAN, program, "uViewSpan");
    g_gatherUniforms.declare(G_SOURCE, program, "uGatherSource");
    g_gatherUniforms.declare(G_SOURCE_TEXEL, program, "uSourceTexel");
    g_gatherUniforms.declare(G_REGION_SPAN, program, "uRegionSpan");
    g_gatherUniforms.declare(G_AXIS_DIR, program, "uAxisDir");
    g_gatherUniforms.declare(G_AXIS_STEP, program, "uAxisStep");
    g_gatherUniforms.declare(G_TAP_MASS, program, "uTapMass");
    g_gatherUniforms.declare(G_FBO_PITCH, program, "uFboPitch");
    g_gatherUniforms.declare(G_BOX_HALF, program, "uBoxHalf");
}

void setLinearClamp() {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

struct GlTarget {
    GLint fbo = 0;
    GLint viewport[4] = {0, 0, 0, 0};
};

GlTarget probeTarget() {
    GlTarget target;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &target.fbo);
    glGetIntegerv(GL_VIEWPORT, target.viewport);
    return target;
}

void restoreTarget(const GlTarget& target) {
    glBindFramebuffer(GL_FRAMEBUFFER, (GLuint)target.fbo);
    glViewport(target.viewport[0], target.viewport[1], target.viewport[2],
               target.viewport[3]);
}

GlTarget g_frameTarget;
bool g_frameTargetKnown = false;

void drawUnitQuad(const Box& frame, float viewW, float viewH) {
    g_panelUniforms.set4(P_FRAME_RECT, frame.left, frame.top, frame.width(),
                         frame.height());
    g_panelUniforms.set2(P_VIEW_SPAN, viewW, viewH);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void drawGatherQuad(const Box& frame, float viewW, float viewH) {
    g_gatherUniforms.set4(G_FRAME_RECT, frame.left, frame.top, frame.width(),
                          frame.height());
    g_gatherUniforms.set2(G_VIEW_SPAN, viewW, viewH);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void uploadTapMass() {
    const GLint location = g_gatherUniforms.location(G_TAP_MASS);
    if (location >= 0) glUniform1fv(location, TAP_COUNT, g_tapMass);
}

void clearTextureOnce(GLuint texture, int w, int h) {
    static unsigned char zeros[config::Render::CAPTURE_CLEAR_BYTES];
    if (texture == 0u || w <= 0 || h <= 0) return;
    const int rowBytes = w * 4;
    const int chunkRows = rowBytes > 0 ? (int)(sizeof(zeros) / (size_t)rowBytes) : 0;
    if (chunkRows <= 0) return;
    glBindTexture(GL_TEXTURE_2D, texture);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    for (int row = 0; row < h; row += chunkRows) {
        const int span = (row + chunkRows > h) ? (h - row) : chunkRows;
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, row, w, span, GL_RGBA, GL_UNSIGNED_BYTE,
                        zeros);
    }
}

int quantizeSpan(int value, int quantum) {
    if (quantum <= 1 || value <= 0) return value;
    return ((value + quantum - 1) / quantum) * quantum;
}

bool ensureCaptureTexture(int w, int h) {
    if (w <= 0 || h <= 0) return false;
    if (g_captureMaxStamp != g_frameStamp) {
        g_captureMaxStamp = g_frameStamp;
        g_captureMaxW = 0;
        g_captureMaxH = 0;
    }
    if (w > g_captureMaxW) g_captureMaxW = w;
    if (h > g_captureMaxH) g_captureMaxH = h;

    if (g_captureTexture == 0) {
        const int initW = quantizeSpan(w, config::Render::CAPTURE_ALLOC_QUANTUM);
        const int initH = quantizeSpan(h, config::Render::CAPTURE_ALLOC_QUANTUM);
        glGenTextures(1, &g_captureTexture);
        if (g_captureTexture == 0) return false;
        glBindTexture(GL_TEXTURE_2D, g_captureTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, initW, initH, 0, GL_RGBA,
                     GL_UNSIGNED_BYTE, nullptr);
        setLinearClamp();
        g_captureWidth = initW;
        g_captureHeight = initH;
        g_captureShrinkStamp = -1;
        return g_captureTexture != 0;
    }

    glBindTexture(GL_TEXTURE_2D, g_captureTexture);
    if (w > g_captureWidth || h > g_captureHeight) {
        const int wantW = w > g_captureWidth ? w : g_captureWidth;
        const int wantH = h > g_captureHeight ? h : g_captureHeight;
        const int slackW = wantW + wantW / config::Render::ALLOC_SLACK_DIVISOR;
        const int slackH = wantH + wantH / config::Render::ALLOC_SLACK_DIVISOR;
        const int growW = quantizeSpan(slackW > g_captureWidth ? slackW : g_captureWidth,
                                       config::Render::CAPTURE_ALLOC_QUANTUM);
        const int growH = quantizeSpan(slackH > g_captureHeight ? slackH : g_captureHeight,
                                       config::Render::CAPTURE_ALLOC_QUANTUM);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, growW, growH, 0, GL_RGBA,
                     GL_UNSIGNED_BYTE, nullptr);
        setLinearClamp();
        g_captureWidth = growW;
        g_captureHeight = growH;
        g_captureShrinkStamp = -1;
        return g_captureTexture != 0;
    }

    const int peakW = g_captureMaxW > 0 ? g_captureMaxW : w;
    const int peakH = g_captureMaxH > 0 ? g_captureMaxH : h;
    const float keepW = (float)g_captureWidth * config::Render::BLUR_CAPTURE_SHRINK_RATIO;
    const float keepH = (float)g_captureHeight * config::Render::BLUR_CAPTURE_SHRINK_RATIO;
    if ((float)peakW < keepW && (float)peakH < keepH) {
        if (g_captureShrinkStamp < 0) {
            g_captureShrinkStamp = g_frameStamp;
        }
        else if (g_frameStamp - g_captureShrinkStamp
                 > config::Render::BLUR_CAPTURE_SHRINK_FRAMES) {
            const int shrinkW = quantizeSpan(peakW, config::Render::CAPTURE_ALLOC_QUANTUM);
            const int shrinkH = quantizeSpan(peakH, config::Render::CAPTURE_ALLOC_QUANTUM);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, shrinkW, shrinkH, 0, GL_RGBA,
                         GL_UNSIGNED_BYTE, nullptr);
            setLinearClamp();
            g_captureWidth = shrinkW;
            g_captureHeight = shrinkH;
            g_captureShrinkStamp = -1;
        }
    }
    else {
        g_captureShrinkStamp = -1;
    }
    return g_captureTexture != 0;
}

bool ensureGatherTarget(int slot, int& w, int& h, GLint restoreFbo) {
    if (slot < 0 || slot > 1 || w <= 0 || h <= 0) return false;
    if (g_gatherFbo[slot] != 0 && g_gatherTexture[slot] != 0
            && w <= g_gatherAllocW[slot] && h <= g_gatherAllocH[slot]) {
        w = g_gatherAllocW[slot];
        h = g_gatherAllocH[slot];
        return g_gatherTexture[slot] != 0;
    }

    const int wantW = w > g_gatherAllocW[slot] ? w : g_gatherAllocW[slot];
    const int wantH = h > g_gatherAllocH[slot] ? h : g_gatherAllocH[slot];
    const int slackW = wantW + wantW / config::Render::ALLOC_SLACK_DIVISOR;
    const int slackH = wantH + wantH / config::Render::ALLOC_SLACK_DIVISOR;
    const int allocW = quantizeSpan(slackW > wantW ? slackW : wantW,
                                    config::Render::BLUR_TARGET_ALIGN);
    const int allocH = quantizeSpan(slackH > wantH ? slackH : wantH,
                                    config::Render::BLUR_TARGET_ALIGN);

    while (glGetError() != GL_NO_ERROR) {}
    GLuint freshTexture = 0;
    glGenTextures(1, &freshTexture);
    if (freshTexture != 0) {
        glBindTexture(GL_TEXTURE_2D, freshTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, allocW, allocH, 0, GL_RGBA,
                     GL_UNSIGNED_BYTE, nullptr);
        setLinearClamp();
        clearTextureOnce(freshTexture, allocW, allocH);
    }
    GLuint freshFbo = 0;
    if (freshTexture != 0) {
        glGenFramebuffers(1, &freshFbo);
    }
    if (freshFbo != 0) {
        glBindFramebuffer(GL_FRAMEBUFFER, freshFbo);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                               freshTexture, 0);
    }
    const bool built = freshFbo != 0;
    const GLenum status = built ? glCheckFramebufferStatus(GL_FRAMEBUFFER) : (GLenum)0;
    glBindFramebuffer(GL_FRAMEBUFFER, (GLuint)restoreFbo);
    if (!built || status != GL_FRAMEBUFFER_COMPLETE) {
        if (freshFbo != 0) glDeleteFramebuffers(1, &freshFbo);
        if (freshTexture != 0) glDeleteTextures(1, &freshTexture);
        w = g_gatherAllocW[slot];
        h = g_gatherAllocH[slot];
        return g_gatherTexture[slot] != 0;
    }

    if (g_gatherTexture[slot] != 0) glDeleteTextures(1, &g_gatherTexture[slot]);
    if (g_gatherFbo[slot] != 0) glDeleteFramebuffers(1, &g_gatherFbo[slot]);
    g_gatherTexture[slot] = freshTexture;
    g_gatherFbo[slot] = freshFbo;
    g_gatherAllocW[slot] = allocW;
    g_gatherAllocH[slot] = allocH;
    w = allocW;
    h = allocH;
    return g_gatherFbo[slot] != 0;
}

void buildTapMass(float sigma, float step) {
    const float safeSigma = sigma > config::Motion::SPRING_EPSILON
        ? sigma : config::Motion::SPRING_EPSILON;
    const float decay = expf(-0.5f * (step * step) / (safeSigma * safeSigma));
    const float decaySquared = decay * decay;
    float mass = decay;
    float massRatio = decay * decaySquared;
    for (int i = 0; i < TAP_COUNT; ++i) {
        g_tapMass[i] = mass;
        mass *= massRatio;
        massRatio *= decaySquared;
    }
}

void releaseGatherTargets() {
    for (int i = 0; i < 2; ++i) {
        if (g_gatherFbo[i] != 0) {
            glDeleteFramebuffers(1, &g_gatherFbo[i]);
            g_gatherFbo[i] = 0;
        }
        if (g_gatherTexture[i] != 0) {
            glDeleteTextures(1, &g_gatherTexture[i]);
            g_gatherTexture[i] = 0;
        }
        g_gatherAllocW[i] = 0;
        g_gatherAllocH[i] = 0;
    }
}

void applyClipScissor(const Box& clip, float viewW, float viewH) {
    const float scaleX = pixelScaleXOf();
    const float scaleY = pixelScaleYOf();
    int left = (int)floorf(clip.left * scaleX);
    int top = (int)floorf(clip.top * scaleY);
    int right = (int)ceilf(clip.right * scaleX);
    int bottom = (int)ceilf(clip.bottom * scaleY);
    const int limitW = viewW > 0.0f ? (int)viewW : 0;
    const int limitH = viewH > 0.0f ? (int)viewH : 0;
    if (limitW <= 0 || limitH <= 0) return;
    if (left < 0) left = 0;
    if (top < 0) top = 0;
    if (right > limitW) right = limitW;
    if (bottom > limitH) bottom = limitH;
    if (right < left) right = left;
    if (bottom < top) bottom = top;
    glScissor(left, limitH - bottom, right - left, bottom - top);
    glEnable(GL_SCISSOR_TEST);
}

}

bool glStartup() {
    if (g_ready) return true;
    g_panelProgram = linkPair(quadVertexSource(), panelFragmentSource());
    if (g_panelProgram == 0) return false;
    g_gatherProgram = linkPair(quadVertexSource(), gatherFragmentSource());
    if (g_gatherProgram == 0) {
        glDeleteProgram(g_panelProgram);
        g_panelProgram = 0;
        return g_gatherProgram != 0;
    }
    declarePanelUniforms(g_panelProgram);
    declareGatherUniforms(g_gatherProgram);
    glUseProgram(g_panelProgram);
    glUniform1i(g_panelUniforms.location(P_SCENE), 0);
    glUniform1i(g_panelUniforms.location(P_GATHERED), 1);
    g_panelUniforms.setArray(P_PRISM_DIST, config::Render::PRISM_DIST,
                             config::Render::PRISM_PAIRS);
    g_panelUniforms.setArray(P_PRISM_EVEN_RED, config::Render::PRISM_EVEN_RED,
                             config::Render::PRISM_PAIRS);
    g_panelUniforms.setArray(P_PRISM_ODD_RED, config::Render::PRISM_ODD_RED,
                             config::Render::PRISM_PAIRS);
    g_panelUniforms.setArray(P_PRISM_EVEN_GREEN, config::Render::PRISM_EVEN_GREEN,
                             config::Render::PRISM_PAIRS);
    g_panelUniforms.setArray(P_PRISM_ODD_GREEN, config::Render::PRISM_ODD_GREEN,
                             config::Render::PRISM_PAIRS);
    g_panelUniforms.setArray(P_PRISM_EVEN_BLUE, config::Render::PRISM_EVEN_BLUE,
                             config::Render::PRISM_PAIRS);
    g_panelUniforms.setArray(P_PRISM_ODD_BLUE, config::Render::PRISM_ODD_BLUE,
                             config::Render::PRISM_PAIRS);
    g_panelUniforms.set1(P_PRISM_CENTER_GREEN, config::Render::PRISM_CENTER_GREEN);
    glUseProgram(g_gatherProgram);
    glUniform1i(g_gatherUniforms.location(G_SOURCE), 0);
    glUseProgram(0);
    g_ready = true;
    return g_ready;
}

void glShutdown() {
    glReleaseCapture();
    releaseGatherTargets();
    if (g_gatherProgram != 0) {
        glDeleteProgram(g_gatherProgram);
        g_gatherProgram = 0;
    }
    if (g_panelProgram != 0) {
        glDeleteProgram(g_panelProgram);
        g_panelProgram = 0;
    }
    g_ready = false;
}

bool glReady() { return g_ready; }

bool glGatherReady() { return g_ready && g_gatherProgram != 0; }

void glBeginFrame() {
    ++g_frameStamp;
    g_frameTargetKnown = false;
}

bool glCaptureRegion(int x, int y, int w, int h, int viewW, int viewH,
                     float& originX, float& originY, float& sizeX, float& sizeY,
                     float& strideX, float& strideY) {
    if (!g_ready || w <= 0 || h <= 0 || viewW <= 0 || viewH <= 0) return false;
    if (!ensureCaptureTexture(w, h)) return false;
    if (w > g_captureWidth || h > g_captureHeight) return false;
    glBindTexture(GL_TEXTURE_2D, g_captureTexture);
    const int glY = viewH - (y + h);
    glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, x, glY, w, h);
    originX = (float)x;
    originY = (float)y;
    sizeX = (float)w;
    sizeY = (float)h;
    strideX = (float)g_captureWidth;
    strideY = (float)g_captureHeight;
    return sizeX > 0.0f;
}

void glReleaseCapture() {
    if (g_captureTexture != 0) {
        glDeleteTextures(1, &g_captureTexture);
        g_captureTexture = 0;
    }
    g_captureWidth = 0;
    g_captureHeight = 0;
    g_captureShrinkStamp = -1;
}

unsigned int glCaptureTexture() { return g_captureTexture; }
int glCaptureWidth() { return g_captureWidth; }
int glCaptureHeight() { return g_captureHeight; }

bool glRunGather(int regionW, int regionH, unsigned int sourceTexture, int sourceW,
                 int sourceH, float blurRadius) {
    g_gatherInfo.ready = false;
    if (!glGatherReady() || sourceTexture == 0) return false;
    if (blurRadius < config::Render::BLUR_MIN_SIGMA) return false;
    if (regionW <= 1 || regionH <= 1 || sourceW <= 0 || sourceH <= 0) return false;

    const float sigma = blurRadius * config::Render::BLUR_SIGMA_FROM_RADIUS;
    int down = 1;
    if (sigma > config::Render::BLUR_FBO_SIGMA_CAP) {
        down = (int)ceilf(sigma / config::Render::BLUR_FBO_SIGMA_CAP);
        if (down < 2) down = 2;
        if (down > config::Render::BLUR_DECIMATE_LIMIT) {
            down = config::Render::BLUR_DECIMATE_LIMIT;
        }
    }

    int fboW = (regionW + down - 1) / down;
    int fboH = (regionH + down - 1) / down;
    if (fboW < config::Render::BLUR_TARGET_MIN_SPAN) {
        fboW = config::Render::BLUR_TARGET_MIN_SPAN;
    }
    if (fboH < config::Render::BLUR_TARGET_MIN_SPAN) {
        fboH = config::Render::BLUR_TARGET_MIN_SPAN;
    }
    const int align = config::Render::BLUR_TARGET_ALIGN;
    const int allocW = ((fboW + align - 1) / align) * align;
    const int allocH = ((fboH + align - 1) / align) * align;

    if (!g_frameTargetKnown) {
        g_frameTarget = probeTarget();
        g_frameTargetKnown = true;
    }
    const GlTarget& target = g_frameTarget;
    int stageW = allocW;
    int stageH = allocH;
    if (!ensureGatherTarget(0, stageW, stageH, target.fbo)) return false;
    int outputW = allocW;
    int outputH = allocH;
    if (!ensureGatherTarget(1, outputW, outputH, target.fbo)) return false;
    if (fboW > stageW) fboW = stageW;
    if (fboH > stageH) fboH = stageH;

    const GLboolean hadScissor = glIsEnabled(GL_SCISSOR_TEST);
    const GLboolean hadBlend = glIsEnabled(GL_BLEND);

    const float sigmaFbo = sigma / (float)down;
    const float stepFbo = fmaxf(config::Render::BLUR_MIN_STEP,
                                sigmaFbo * config::Render::BLUR_STEP_FROM_SIGMA);
    const float stepSrc = stepFbo * (float)down;

    const float sourceBoxHalf = (down > 1)
        ? stepSrc * config::Render::BLUR_BOX_HALF_RATIO : 0.0f;
    const float targetBoxHalf = (stepFbo > config::Render::BLUR_STEP_BOX_AT)
        ? stepFbo * config::Render::BLUR_BOX_HALF_RATIO : 0.0f;
    buildTapMass(sigmaFbo, stepFbo);

    glDisable(GL_SCISSOR_TEST);
    glDisable(GL_BLEND);

    glUseProgram(g_gatherProgram);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, sourceTexture);
    g_gatherUniforms.set2(G_SOURCE_TEXEL, (float)sourceW, (float)sourceH);
    g_gatherUniforms.set2(G_VIEW_SPAN, (float)fboW, (float)fboH);
    g_gatherUniforms.set2(G_FBO_PITCH, (float)regionW / (float)fboW,
                          (float)regionH / (float)fboH);
    uploadTapMass();

    const Box stage(0.0f, 0.0f, (float)fboW, (float)fboH);

    glBindFramebuffer(GL_FRAMEBUFFER, g_gatherFbo[0]);
    glViewport(0, 0, fboW, fboH);
    g_gatherUniforms.set2(G_REGION_SPAN, (float)regionW, (float)regionH);
    g_gatherUniforms.set2(G_AXIS_DIR, 1.0f, 0.0f);
    g_gatherUniforms.set1(G_AXIS_STEP, stepSrc);
    g_gatherUniforms.set1(G_BOX_HALF, sourceBoxHalf);
    drawGatherQuad(stage, (float)fboW, (float)fboH);

    glBindFramebuffer(GL_FRAMEBUFFER, g_gatherFbo[1]);
    glViewport(0, 0, fboW, fboH);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, g_gatherTexture[0]);
    g_gatherUniforms.set2(G_SOURCE_TEXEL, (float)stageW, (float)stageH);
    g_gatherUniforms.set2(G_REGION_SPAN, (float)fboW, (float)fboH);
    g_gatherUniforms.set2(G_FBO_PITCH, 1.0f, 1.0f);
    g_gatherUniforms.set2(G_AXIS_DIR, 0.0f, 1.0f);
    g_gatherUniforms.set1(G_AXIS_STEP, stepFbo);
    g_gatherUniforms.set1(G_BOX_HALF, targetBoxHalf);
    drawGatherQuad(stage, (float)fboW, (float)fboH);

    restoreTarget(g_frameTarget);
    if (hadScissor) glEnable(GL_SCISSOR_TEST); else glDisable(GL_SCISSOR_TEST);
    if (hadBlend) glEnable(GL_BLEND); else glDisable(GL_BLEND);

    g_gatherInfo.ready = true;
    g_gatherInfo.pitchX = (float)fboW / (float)regionW;
    g_gatherInfo.pitchY = (float)fboH / (float)regionH;
    g_gatherInfo.rows = (float)fboH;
    g_gatherInfo.texelX = (float)outputW;
    g_gatherInfo.texelY = (float)outputH;
    g_gatherInfo.limitX = ((float)fboW - 0.5f) / (float)outputW;
    g_gatherInfo.limitY = ((float)fboH - 0.5f) / (float)outputH;
    return g_gatherInfo.ready;
}

GatherInfo glGatherInfo() { return g_gatherInfo; }

void glPaintPanel(const PanelJob& job, const Box& clip, unsigned int sceneTexture,
                  const GatherInfo& gather) {
    if (!g_ready) return;
    if (job.frame.width() <= 0.0f || job.frame.height() <= 0.0f) return;

    const bool useGather = gather.ready && sceneTexture == glCaptureTexture()
                        && glCaptureTexture() != 0;

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, sceneTexture);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, useGather ? g_gatherTexture[1] : 0);
    glActiveTexture(GL_TEXTURE0);

    glUseProgram(g_panelProgram);
    UniformTable<P_SLOT_COUNT>& u = g_panelUniforms;

    u.set4(P_FRAME_RECT, job.frame.left, job.frame.top, job.frame.width(),
           job.frame.height());
    u.set2(P_VIEW_SPAN, job.viewW, job.viewH);
    u.set1(P_GATHERED_ON, useGather ? 1.0f : 0.0f);
    u.set2(P_GATHER_PITCH, gather.pitchX, gather.pitchY);
    u.set1(P_GATHER_ROWS, gather.rows);
    u.set2(P_GATHER_TEXEL, gather.texelX, gather.texelY);
    u.set2(P_GATHER_LIMIT, gather.limitX, gather.limitY);

    u.set2(P_PANEL_SPAN, job.frame.width(), job.frame.height());
    u.set2(P_SAMPLE_SHIFT, job.frame.left - job.sampleOriginX,
           job.frame.top - job.sampleOriginY);
    u.set2(P_SAMPLE_SPAN, job.sampleSizeX, job.sampleSizeY);
    u.set2(P_SAMPLE_TEXEL, job.sampleStrideX, job.sampleStrideY);
    u.set4(P_CORNER_RADII, job.corners.topLeft, job.corners.topRight,
           job.corners.bottomRight, job.corners.bottomLeft);
    u.set4(P_DROP_RECT, job.dropRect[0], job.dropRect[1], job.dropRect[2],
           job.dropRect[3]);
    u.set1(P_DROP_ROUND, job.dropRound);
    u.set4(P_FEED_RECT, job.feedRect[0], job.feedRect[1], job.feedRect[2],
           job.feedRect[3]);
    u.set1(P_FEED_ROUND, job.feedRound);
    u.set1(P_FEED_MERGE, job.feedMerge);
    u.set4(P_STRAND_RECT, job.strandRect[0], job.strandRect[1], job.strandRect[2],
           job.strandRect[3]);
    u.set1(P_STRAND_ROUND, job.strandRound);
    u.set1(P_STRAND_MERGE, job.strandMerge);
    u.set1(P_LENS_HEIGHT, job.lensHeight);
    u.set1(P_LENS_AMOUNT, job.lensAmount);
    u.set1(P_LENS_DEPTH, job.lensDepth);
    u.set1(P_DARK_GUARD, job.darkGuard);
    u.set1(P_SPECTRAL, job.spectral);
    u.set1(P_PANEL_OPACITY, job.panelOpacity);
    u.set3(P_TONE_CTRL, job.toneCtrl[0], job.toneCtrl[1], job.toneCtrl[2]);
    u.set1(P_TONE_BOOST, job.toneBoost);
    u.setColor(P_REFRACT_TINT, job.refractTint);
    u.setColor(P_SURFACE_TINT, job.surface);
    u.set1(P_SURFACE_MODE, job.surfaceMode);
    u.set1(P_EDGE_MODE, job.edgeMode);
    u.setColor(P_EDGE_TINT, job.edgeTint);
    u.set1(P_EDGE_TINT_ALPHA, job.edgeTintAlpha);
    u.set1(P_EDGE_ANGLE, job.edgeAngle);
    u.set1(P_EDGE_FALLOFF, job.edgeFalloff);
    u.set1(P_EDGE_ALPHA, job.edgeAlpha);
    u.set1(P_EDGE_WIDTH, job.edgeWidth);
    u.set1(P_EDGE_BLUR, job.edgeBlur);
    u.setColor(P_INSET_TINT, job.insetTint);
    u.set2(P_INSET_SHIFT, job.insetShiftX, job.insetShiftY);
    u.set1(P_INSET_REACH, job.insetReach);
    u.set1(P_INSET_ALPHA, job.insetAlpha);
    u.set1(P_GLOW_ON, job.glowOn);
    u.set2(P_GLOW_AT, job.glowX, job.glowY);
    u.set1(P_GLOW_REACH, job.glowReach);
    u.set1(P_GLOW_LEVEL, job.glowLevel);
    u.set1(P_SCENE_ON, job.sceneOn);
    u.set1(P_SOLID_SCENE, job.solidBackdrop);
    u.set2(P_STRETCH_SCALE, job.stretchX, job.stretchY);
    u.set1(P_STRETCH_MIX, job.stretchMix);
    u.setColor(P_ABSENT_TINT, job.absentTint);
    u.set1(P_TRACK_ON, job.trackOn);
    u.set4(P_TRACK_RECT, job.trackRect[0], job.trackRect[1], job.trackRect[2],
           job.trackRect[3]);
    u.set1(P_TRACK_ROUND, job.trackRadius);
    u.setColor(P_TRACK_TINT, job.trackTint);
    u.set1(P_SHADE_MODE, job.shadeMode);
    u.setColor(P_SHADE_TINT, job.shadeTint);
    u.set2(P_SHADE_SHIFT, job.shadeShiftX, job.shadeShiftY);
    u.set1(P_SHADE_REACH, job.shadeReach);
    u.set2(P_SHADE_BODY, job.shadeBodyW, job.shadeBodyH);

    applyClipScissor(clip, job.viewW, job.viewH);
    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE,
                        GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);

    drawUnitQuad(job.frame, job.viewW, job.viewH);
    glUseProgram(0);
}

}
}
