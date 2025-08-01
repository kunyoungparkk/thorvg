/*
 * Copyright (c) 2023 - 2025 the ThorVG project. All rights reserved.

 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef _TVG_TEXT_H
#define _TVG_TEXT_H

#include "tvgStr.h"
#include "tvgMath.h"
#include "tvgShape.h"
#include "tvgFill.h"
#include "tvgLoader.h"

#define TEXT(A) static_cast<TextImpl*>(A)
#define CONST_TEXT(A) static_cast<const TextImpl*>(A)

struct TextImpl : Text
{
    Paint::Impl impl;
    Shape* shape;   //text shape
    FontLoader* loader = nullptr;
    FontMetrics metrics;
    char* utf8 = nullptr;
    float fontSize;
    bool italic = false;

    TextImpl() : impl(Paint::Impl(this)), shape(Shape::gen())
    {
        PAINT(shape)->parent = this;
    }

    ~TextImpl()
    {
        tvg::free(utf8);
        LoaderMgr::retrieve(loader);
        delete(shape);
    }

    Result text(const char* utf8)
    {
        tvg::free(this->utf8);
        if (utf8) this->utf8 = tvg::duplicate(utf8);
        else this->utf8 = nullptr;

        impl.mark(RenderUpdateFlag::Path);

        return Result::Success;
    }

    Result font(const char* name, float size, const char* style)
    {
        auto loader = name ? LoaderMgr::font(name) : LoaderMgr::anyfont();
        if (!loader) return Result::InsufficientCondition;

        if (style && strstr(style, "italic")) italic = true;
        else italic = false;

        fontSize = size;

        //Same resource has been loaded.
        if (this->loader == loader) {
            this->loader->sharing--;  //make it sure the reference counting.
            return Result::Success;
        } else if (this->loader) {
            LoaderMgr::retrieve(this->loader);
        }
        this->loader = static_cast<FontLoader*>(loader);

        impl.mark(RenderUpdateFlag::Path);

        return Result::Success;
    }

    RenderRegion bounds(RenderMethod* renderer) const
    {
        return SHAPE(shape)->bounds(renderer);
    }

    bool render(RenderMethod* renderer)
    {
        if (!loader) return true;
        renderer->blend(impl.blendMethod);
        return PAINT(shape)->render(renderer);
    }

    float load()
    {
        if (!loader) return 0.0f;

        //reload
        if (impl.marked(RenderUpdateFlag::Path)) loader->read(shape, utf8, metrics);

        return loader->transform(shape, metrics, fontSize, italic);
    }

    bool skip(RenderUpdateFlag flag)
    {
        if (flag == RenderUpdateFlag::None) return true;
        return false;
    }

    bool update(RenderMethod* renderer, const Matrix& transform, Array<RenderData>& clips, uint8_t opacity, RenderUpdateFlag flag, TVG_UNUSED bool clipper)
    {
        auto scale = 1.0f / load();
        if (tvg::zero(scale)) return false;

        //transform the gradient coordinates based on the final scaled font.
        auto fill = SHAPE(shape)->rs.fill;
        if (fill && SHAPE(shape)->impl.renderFlag & RenderUpdateFlag::Gradient) {
            if (fill->type() == Type::LinearGradient) {
                LINEAR(fill)->x1 *= scale;
                LINEAR(fill)->y1 *= scale;
                LINEAR(fill)->x2 *= scale;
                LINEAR(fill)->y2 *= scale;
            } else {
                RADIAL(fill)->cx *= scale;
                RADIAL(fill)->cy *= scale;
                RADIAL(fill)->r *= scale;
                RADIAL(fill)->fx *= scale;
                RADIAL(fill)->fy *= scale;
                RADIAL(fill)->fr *= scale;
            }
        }
        PAINT(shape)->update(renderer, transform, clips, opacity, flag, false);
        return true;
    }

    bool intersects(const RenderRegion& region)
    {
        if (load() == 0.0f) return false;
        return SHAPE(shape)->intersects(region);
    }


    Result bounds(Point* pt4, Matrix& m, bool obb, TVG_UNUSED bool stroking)
    {
        if (load() == 0.0f) return Result::InsufficientCondition;
        return PAINT(shape)->bounds(pt4, &m, obb, true);
    }

    Paint* duplicate(Paint* ret)
    {
        if (ret) TVGERR("RENDERER", "TODO: duplicate()");

        load();

        auto text = Text::gen();
        auto dup = TEXT(text);

        SHAPE(shape)->duplicate(dup->shape);

        if (loader) {
            dup->loader = loader;
            ++dup->loader->sharing;
        }

        dup->utf8 = tvg::duplicate(utf8);
        dup->italic = italic;
        dup->fontSize = fontSize;

        return text;
    }

    Iterator* iterator()
    {
        return nullptr;
    }
};

#endif //_TVG_TEXT_H
