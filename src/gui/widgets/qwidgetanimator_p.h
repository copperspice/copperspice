/***********************************************************************
*
* Copyright (c) 2012-2014 Barbara Geller
* Copyright (c) 2012-2014 Ansel Sermersheim
* Copyright (c) 2012-2014 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
*
* This file is part of CopperSpice.
*
* CopperSpice is free software: you can redistribute it and/or 
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 as published by the Free Software Foundation.
*
* CopperSpice is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with CopperSpice.  If not, see 
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#ifndef QWIDGETANIMATOR_P_H
#define QWIDGETANIMATOR_P_H

#include <qobject.h>
#include <qmap.h>
#include <qpointer.h>

QT_BEGIN_NAMESPACE

class QWidget;
class QMainWindowLayout;
class QPropertyAnimation;
class QRect;

class QWidgetAnimator : public QObject
{
    CS_OBJECT(QWidgetAnimator)

public:
    QWidgetAnimator(QMainWindowLayout *layout);
    void animate(QWidget *widget, const QRect &final_geometry, bool animate);
    bool animating() const;

    void abort(QWidget *widget);

#ifndef QT_NO_ANIMATION
private :
    GUI_CS_SLOT_1(Private, void animationFinished())
    GUI_CS_SLOT_2(animationFinished) 
#endif

private:    
    typedef QMap<QWidget*, QPointer<QPropertyAnimation> > AnimationMap;
  
    AnimationMap m_animation_map;
    QMainWindowLayout *m_mainWindowLayout;
};

QT_END_NAMESPACE

#endif // QWIDGET_ANIMATOR_P_H
