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

#ifndef QWINDOWSSTYLE_P_H
#define QWINDOWSSTYLE_P_H

#include <qwindowsstyle.h>
#include <qcommonstyle_p.h>

#ifndef QT_NO_STYLE_WINDOWS
#include <qlist.h>
#include <qhash.h>
#include <qelapsedtimer.h>

QT_BEGIN_NAMESPACE

class QTime;
class QProgressBar;

class QWindowsStylePrivate : public QCommonStylePrivate
{
   Q_DECLARE_PUBLIC(QWindowsStyle)

 public:
   QWindowsStylePrivate();
   void startAnimation(QObject *o, QProgressBar *bar);
   void stopAnimation(QObject *o, QProgressBar *bar);
   bool hasSeenAlt(const QWidget *widget) const;
   bool altDown() const {
      return alt_down;
   }
   bool alt_down;
   QList<const QWidget *> seenAlt;
   int menuBarTimer;

   QList<QProgressBar *> animatedProgressBars;
   int animationFps;
   int animateTimer;
   QElapsedTimer startTime;
   int animateStep;
   QColor inactiveCaptionText;
   QColor activeCaptionColor;
   QColor activeGradientCaptionColor;
   QColor inactiveCaptionColor;
   QColor inactiveGradientCaptionColor;

   enum {
      windowsItemFrame        =  2, // menu item frame width
      windowsSepHeight        =  9, // separator item height
      windowsItemHMargin      =  3, // menu item hor text margin
      windowsItemVMargin      =  2, // menu item ver text margin
      windowsArrowHMargin     =  6, // arrow horizontal margin
      windowsRightBorder      = 15, // right border on windows
      windowsCheckMarkWidth   = 12  // checkmarks width on windows
   };
};

QT_END_NAMESPACE

#endif // QT_NO_STYLE_WINDOWS

#endif //QWINDOWSSTYLE_P_H
