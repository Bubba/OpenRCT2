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

#pragma once

#include "../common.h"

interface IDrawingEngine;

interface IDrawingContext
{
    virtual ~IDrawingContext() { }

    virtual IDrawingEngine * GetEngine() abstract;

    virtual void Clear(uint32 colour)                                                                     abstract;
    virtual void FillRect(uint32 colour, sint32 left, sint32 top, sint32 right, sint32 bottom)            abstract;
    virtual void DrawSprite(uint32 image, sint32 x, sint32 y, uint32 tertiaryColour)                      abstract;
    virtual void DrawSpritePaletteSet(uint32 image, sint32 x, sint32 y, uint8 * palette, uint8 * unknown) abstract;
    virtual void DrawSpriteRawMasked(sint32 x, sint32 y, uint32 maskImage, uint32 colourImage)            abstract;
};