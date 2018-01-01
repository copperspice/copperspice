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

#ifndef QColorOutput_P_H
#define QColorOutput_P_H

#include <QtCore/QtGlobal>
#include <QtCore/QHash>

QT_BEGIN_NAMESPACE

namespace QPatternist {
class ColorOutputPrivate;

class ColorOutput
{
   enum {
      ForegroundShift = 10,
      BackgroundShift = 20,
      SpecialShift    = 20,
      ForegroundMask  = ((static_cast<quint64>(1) << ForegroundShift) - 1) << ForegroundShift,
      BackgroundMask  = ((static_cast<quint64>(1) << BackgroundShift) - 1) << BackgroundShift
   };

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
      DefaultColor            = 1 << SpecialShift
   };

   typedef QFlags<ColorCodeComponent> ColorCode;
   typedef QHash<int, ColorCode> ColorMapping;

   ColorOutput();
   ~ColorOutput();

   void setColorMapping(const ColorMapping &cMapping);
   ColorMapping colorMapping() const;
   void insertMapping(int colorID, const ColorCode colorCode);

   void writeUncolored(const QString &message);
   void write(const QString &message, int color = -1);
   QString colorify(const QString &message, int color = -1) const;

 private:
   ColorOutputPrivate *d;
   Q_DISABLE_COPY(ColorOutput)
};
}

Q_DECLARE_OPERATORS_FOR_FLAGS(QPatternist::ColorOutput::ColorCode)

QT_END_NAMESPACE

#endif
