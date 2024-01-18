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

#ifndef QWIDGETANIMATOR_P_H
#define QWIDGETANIMATOR_P_H

#include <qobject.h>
#include <qhash.h>
#include <qpointer.h>

class QWidget;
class QMainWindowLayout;
class QPropertyAnimation;
class QRect;

class QWidgetAnimator : public QObject
{
   GUI_CS_OBJECT(QWidgetAnimator)

 public:
   QWidgetAnimator(QMainWindowLayout *layout);
   void animate(QWidget *widget, const QRect &final_geometry, bool animate);
   bool animating() const;

   void abort(QWidget *widget);

 private:
   typedef QHash<QWidget *, QPointer<QPropertyAnimation>> AnimationMap;

   AnimationMap m_animation_map;
   QMainWindowLayout *m_mainWindowLayout;

#ifndef QT_NO_ANIMATION
   GUI_CS_SLOT_1(Private, void animationFinished())
   GUI_CS_SLOT_2(animationFinished)
#endif

};

#endif // QWIDGET_ANIMATOR_P_H
