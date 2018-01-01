/***********************************************************************
*
* Copyright (c) 2012-2018 Barbara Geller
* Copyright (c) 2012-2018 Ansel Sermersheim
* Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
*
* This file is part of CopperSpice.
*
* CopperSpice is free software. You can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 as published by the Free Software Foundation.
*
* CopperSpice is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

static const QFontEngineQPF::TagType tagTypes[QFontEngineQPF::NumTags] = {
   QFontEngineQPF::StringType, // FontName
   QFontEngineQPF::StringType, // FileName
   QFontEngineQPF::UInt32Type, // FileIndex
   QFontEngineQPF::UInt32Type, // FontRevision
   QFontEngineQPF::StringType, // FreeText
   QFontEngineQPF::FixedType,  // Ascent
   QFontEngineQPF::FixedType,  // Descent
   QFontEngineQPF::FixedType,  // Leading
   QFontEngineQPF::FixedType,  // XHeight
   QFontEngineQPF::FixedType,  // AverageCharWidth
   QFontEngineQPF::FixedType,  // MaxCharWidth
   QFontEngineQPF::FixedType,  // LineThickness
   QFontEngineQPF::FixedType,  // MinLeftBearing
   QFontEngineQPF::FixedType,  // MinRightBearing
   QFontEngineQPF::FixedType,  // UnderlinePosition
   QFontEngineQPF::UInt8Type,  // GlyphFormat
   QFontEngineQPF::UInt8Type,  // PixelSize
   QFontEngineQPF::UInt8Type,  // Weight
   QFontEngineQPF::UInt8Type,  // Style
   QFontEngineQPF::StringType, // EndOfHeader
   QFontEngineQPF::BitFieldType// WritingSystems
};


