/***********************************************************************
*
* Copyright (c) 2012-2024 Barbara Geller
* Copyright (c) 2012-2024 Ansel Sermersheim
*
* Copyright (c) 2015 The Qt Company Ltd.
* Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
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
* https://www.gnu.org/licenses/
*
***********************************************************************/

#ifndef QColorOutput_P_H
#define QColorOutput_P_H

#include <qglobal.h>
#include <qhash.h>

namespace QPatternist {
class ColorOutputPrivate;

class ColorOutput
{
static constexpr uint32_t ForegroundShift = 0;
static constexpr uint32_t BackgroundShift = 10;

static constexpr uint32_t ForegroundMask  = 0x000003FF;
static constexpr uint32_t BackgroundMask  = 0xFFFFFC00;

 public:
   enum ColorCodeComponent {
      BlackForeground         = 1 << ForegroundShift,
      BlueForeground          = 2 << ForegroundShift,
      GreenForeground         = 3 << ForegroundShift,
      CyanForeground          = 4 << ForegroundShift,
      RedForeground           = 5 << ForegroundShift,
      PurpleForeground        = 6 << ForegroundShift,
      BrownForeground         = 7 << ForegroundShift,
      LightGrayForeground     = 8 << ForegroundShift,
      DarkGrayForeground      = 9 << ForegroundShift,
      LightBlueForeground     = 10 << ForegroundShift,
      LightGreenForeground    = 11 << ForegroundShift,
      LightCyanForeground     = 12 << ForegroundShift,
      LightRedForeground      = 13 << ForegroundShift,
      LightPurpleForeground   = 14 << ForegroundShift,
      YellowForeground        = 15 << ForegroundShift,
      WhiteForeground         = 16 << ForegroundShift,

      BlackBackground         = 1 << BackgroundShift,
      BlueBackground          = 2 << BackgroundShift,
      GreenBackground         = 3 << BackgroundShift,
      CyanBackground          = 4 << BackgroundShift,
      RedBackground           = 5 << BackgroundShift,
      PurpleBackground        = 6 << BackgroundShift,
      BrownBackground         = 7 << BackgroundShift,

      DefaultColor            = BlackBackground,
   };

   using ColorCode    = QFlags<ColorCodeComponent>;
   using ColorMapping = QHash<int, ColorCode>;

   ColorOutput();
   ~ColorOutput();

   void setColorMapping(const ColorMapping &cMapping);
   ColorMapping colorMapping() const;
   void insertMapping(int colorID, const ColorCode colorCode);

   void writeUncolored(const QString &message);
   void write(const QString &message, int color = -1);
   QString colorify(const QString &message, int color = -1) const;

 private:
   ColorOutput(const ColorOutput &) = delete;
   ColorOutput &operator=(const ColorOutput &) = delete;

   ColorOutputPrivate *d;
};
}

Q_DECLARE_OPERATORS_FOR_FLAGS(QPatternist::ColorOutput::ColorCode)

#endif
