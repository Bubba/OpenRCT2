#pragma region Copyright (c) 2014-2016 OpenRCT2 Developers
/*****************************************************************************
 * OpenRCT2, an open source clone of Roller Coaster Tycoon 2.
 *
 * OpenRCT2 is the work of many authors, a full list can be found in contributors.md
 * For more information, visit https://github.com/OpenRCT2/OpenRCT2
 *
 * OpenRCT2 is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * A full copy of the GNU General Public License can be found in licence.txt
 *****************************************************************************/
#pragma endregion

#include <SDL_platform.h>

#ifdef __WINDOWS__
    #include <windows.h>
    #pragma comment(lib, "opengl32.lib")
#endif

#include <gl/gl.h>
#include <gl/glu.h>

#include "../../core/Math.hpp"
#include "../../core/Memory.hpp"
#include "../IDrawingContext.h"
#include "../IDrawingEngine.h"
#include "../Rain.h"

extern "C"
{
    #include "../../config.h"
    #include "../drawing.h"
    #include "../../interface/window.h"
}

class OpenGLDrawingEngine;

struct vec4f
{
    union { float x; float s; float r; };
    union { float y; float t; float g; };
    union { float z; float p; float b; };
    union { float w; float q; float a; };
};

class OpenGLDrawingContext : public IDrawingContext
{
private:
    OpenGLDrawingEngine *   _engine;
    rct_drawpixelinfo *     _dpi;

    sint32 _offsetX;
    sint32 _offsetY;
    sint32 _clipLeft;
    sint32 _clipTop;
    sint32 _clipRight;
    sint32 _clipBottom;

public:
    OpenGLDrawingContext(OpenGLDrawingEngine * engine);
    ~OpenGLDrawingContext() override;

    IDrawingEngine * GetEngine() override;

    void Clear(uint32 colour) override;
    void FillRect(uint32 colour, sint32 x, sint32 y, sint32 w, sint32 h) override;
    void DrawSprite(uint32 image, sint32 x, sint32 y, uint32 tertiaryColour) override;
    void DrawSpritePaletteSet(uint32 image, sint32 x, sint32 y, uint8 * palette, uint8 * unknown) override;
    void DrawSpriteRawMasked(sint32 x, sint32 y, uint32 maskImage, uint32 colourImage) override;

    void SetDPI(rct_drawpixelinfo * dpi);
};

class OpenGLDrawingEngine : public IDrawingEngine
{
private:
    SDL_Window *    _window         = nullptr;
    SDL_GLContext   _context;

    uint32  _width      = 0;
    uint32  _height     = 0;
    uint32  _pitch      = 0;
    size_t  _bitsSize   = 0;
    uint8 * _bits       = nullptr;

    rct_drawpixelinfo _bitsDPI  = { 0 };

    OpenGLDrawingContext *    _drawingContext;

public:
    vec4f GLPalette[256];

    OpenGLDrawingEngine()
    {
        _drawingContext = new OpenGLDrawingContext(this);
    }

    ~OpenGLDrawingEngine() override
    {
        delete _drawingContext;
        delete _bits;

        SDL_GL_DeleteContext(_context);
    }

    void Initialise(SDL_Window * window) override
    {
        _window = window;

        _context = SDL_GL_CreateContext(_window);
        SDL_GL_MakeCurrent(_window, _context);
    }

    void Resize(uint32 width, uint32 height) override
    {
        ConfigureBits(width, height, width);

        glViewport(0, 0, (GLsizei)width, (GLsizei)height);
    }

    void SetPalette(SDL_Color * palette) override
    {
        for (int i = 0; i < 256; i++)
        {
            SDL_Color colour = palette[i];
            GLPalette[i] = { colour.r / 255.0f,
                             colour.g / 255.0f,
                             colour.b / 255.0f,
                             colour.a / 255.0f };
        }
    }

    void Invalidate(sint32 left, sint32 top, sint32 right, sint32 bottom) override
    {
    }

    void Draw() override
    {
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glMatrixMode(GL_PROJECTION_MATRIX);
        glLoadIdentity();
        glOrtho(0, _width, _height, 0, -1.0, 1.0);

        glMatrixMode(GL_MODELVIEW_MATRIX);
        glLoadIdentity();
        glScalef(1, -1.0f, 0);
        glTranslatef(-1.0f, -1.0f, 0);
        glScalef(2.0f / _width, 2.0f / _height, 0);

        gfx_redraw_screen_rect(0, 0, _width - 1, _height - 1);
        window_update_all_viewports();
        gfx_redraw_screen_rect(0, 0, _width - 1, _height - 1);
        window_update_all();

        rct2_draw();
        Display();
    }

    IDrawingContext * GetDrawingContext(rct_drawpixelinfo * dpi) override
    {
        _drawingContext->SetDPI(dpi);
        return _drawingContext;
    }

    rct_drawpixelinfo * GetDPI()
    {
        return &_bitsDPI;
    }

private:

    void ConfigureBits(uint32 width, uint32 height, uint32 pitch)
    {
        size_t  newBitsSize = pitch * height;
        uint8 * newBits = new uint8[newBitsSize];
        if (_bits == nullptr)
        {
            Memory::Set(newBits, 0, newBitsSize);
        }
        else
        {
            if (_pitch == pitch)
            {
                Memory::Copy(newBits, _bits, Math::Min(_bitsSize, newBitsSize));
            }
            else
            {
                uint8 * src = _bits;
                uint8 * dst = newBits;

                uint32 minWidth = Math::Min(_width, width);
                uint32 minHeight = Math::Min(_height, height);
                for (uint32 y = 0; y < minHeight; y++)
                {
                    Memory::Copy(dst, src, minWidth);
                    if (pitch - minWidth > 0)
                    {
                        Memory::Set(dst + minWidth, 0, pitch - minWidth);
                    }
                    src += _pitch;
                    dst += pitch;
                }
            }
            delete _bits;
        }

        _bits = newBits;
        _bitsSize = newBitsSize;
        _width = width;
        _height = height;
        _pitch = pitch;

        rct_drawpixelinfo * dpi = &_bitsDPI;
        dpi->bits = _bits;
        dpi->x = 0;
        dpi->y = 0;
        dpi->width = width;
        dpi->height = height;
        dpi->pitch = _pitch - width;

        gScreenDPI = *dpi;
    }

    void Display()
    {
        SDL_GL_SwapWindow(_window);
    }
};

IDrawingEngine * DrawingEngineFactory::CreateOpenGL()
{
    return new OpenGLDrawingEngine();
}

OpenGLDrawingContext::OpenGLDrawingContext(OpenGLDrawingEngine * engine)
{
    _engine = engine;
}

OpenGLDrawingContext::~OpenGLDrawingContext()
{

}

IDrawingEngine * OpenGLDrawingContext::GetEngine()
{
    return _engine;
}

void OpenGLDrawingContext::Clear(uint32 colour)
{
}

void OpenGLDrawingContext::FillRect(uint32 colour, sint32 left, sint32 top, sint32 right, sint32 bottom)
{
    vec4f paletteColour = _engine->GLPalette[colour & 0xFF];

    if (left > right)
    {
        left ^= right;
        right ^= left;
        left ^= right;
    }
    if (top > bottom)
    {
        top ^= bottom;
        bottom ^= top;
        top ^= bottom;
    }

    left += _offsetX;
    top += _offsetY;
    right += _offsetX;
    bottom += _offsetY;

    left = Math::Max(left, _clipLeft);
    top = Math::Max(top, _clipTop);
    right = Math::Min(right, _clipRight);
    bottom = Math::Min(bottom, _clipBottom);

    if (right < left || bottom < top)
    {
        return;
    }

    glColor3f(paletteColour.r, paletteColour.g, paletteColour.b);
    glBegin(GL_QUADS);
        glVertex2i(left,  top);
        glVertex2i(left,  bottom);
        glVertex2i(right, bottom);
        glVertex2i(right, top);
    glEnd();
}

void OpenGLDrawingContext::DrawSprite(uint32 image, sint32 x, sint32 y, uint32 tertiaryColour)
{
    int g1Id = image & 0x7FFFF;
    rct_g1_element * g1Element = gfx_get_g1_element(g1Id);

    sint32 left = x + g1Element->x_offset;
    sint32 top = y + g1Element->y_offset;
    sint32 right = left + g1Element->width;
    sint32 bottom = top + g1Element->height;
    FillRect(g1Id & 0xFF, left, top, right, bottom);
}

void OpenGLDrawingContext::DrawSpritePaletteSet(uint32 image, sint32 x, sint32 y, uint8 * palette, uint8 * unknown)
{
}

void OpenGLDrawingContext::DrawSpriteRawMasked(sint32 x, sint32 y, uint32 maskImage, uint32 colourImage)
{
}

void OpenGLDrawingContext::SetDPI(rct_drawpixelinfo * dpi)
{
    rct_drawpixelinfo * screenDPI = _engine->GetDPI();
    size_t bitsSize = (size_t)screenDPI->height * (size_t)(screenDPI->width + screenDPI->pitch);
    size_t bitsOffset = (size_t)(dpi->bits - screenDPI->bits);

    assert(bitsOffset < bitsSize);

    _clipLeft = bitsOffset % (screenDPI->width + screenDPI->pitch);
    _clipTop = bitsOffset / (screenDPI->width + screenDPI->pitch);
    _clipRight = _clipLeft + dpi->width;
    _clipBottom = _clipTop + dpi->height;
    _offsetX = _clipLeft - dpi->x;
    _offsetY = _clipTop - dpi->y;

    _dpi = dpi;
}
