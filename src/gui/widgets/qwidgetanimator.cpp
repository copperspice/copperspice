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

#include <QtCore/qpropertyanimation.h>
#include <QtGui/qwidget.h>
#include <qmainwindowlayout_p.h>

#include <qwidgetanimator_p.h>

QT_BEGIN_NAMESPACE

QWidgetAnimator::QWidgetAnimator(QMainWindowLayout *layout) : m_mainWindowLayout(layout)
{
}

void QWidgetAnimator::abort(QWidget *w)
{
#ifndef QT_NO_ANIMATION
   AnimationMap::iterator it = m_animation_map.find(w);

   if (it == m_animation_map.end()) {
      return;
   }

   QPropertyAnimation *anim = *it;
   m_animation_map.erase(it);

   if (anim) {
      anim->stop();
   }

#ifndef QT_NO_MAINWINDOW
   m_mainWindowLayout->animationFinished(w);
#endif

#else
   Q_UNUSED(w); //there is no animation to abort

#endif //QT_NO_ANIMATION
}

#ifndef QT_NO_ANIMATION
void QWidgetAnimator::animationFinished()
{
   QPropertyAnimation *anim = qobject_cast<QPropertyAnimation *>(sender());
   abort(static_cast<QWidget *>(anim->targetObject()));
}
#endif

void QWidgetAnimator::animate(QWidget *widget, const QRect & end_geometry, bool animate)
{
   QRect r = widget->geometry();
   if (r.right() < 0 || r.bottom() < 0) {
      r = QRect();
   }

   animate = animate && ! r.isNull() && ! end_geometry.isNull();

   // might make the wigdet go away by sending it to negative space
   QRect final_geometry = end_geometry; 

   if (! (end_geometry.isValid() || widget->isWindow())  ) {
      final_geometry = QRect(QPoint(-500 - widget->width(), -500 - widget->height()), widget->size());
   }

#ifndef QT_NO_ANIMATION

   AnimationMap::const_iterator it = m_animation_map.constFind(widget);

   if (it != m_animation_map.constEnd() && (*it)->endValue().toRect() == final_geometry) {
      return;
   }

   QPropertyAnimation *anim = new QPropertyAnimation(widget, "geometry", widget);
   anim->setDuration(animate ? 200 : 0);
   anim->setEasingCurve(QEasingCurve::InOutQuad);
   anim->setEndValue(final_geometry);
   m_animation_map[widget] = anim;

   connect(anim, SIGNAL(finished()), this, SLOT(animationFinished()));

   anim->start(QPropertyAnimation::DeleteWhenStopped);

#else
   //we do it in one shot
   widget->setGeometry(final_geometry);

#ifndef QT_NO_MAINWINDOW
   m_mainWindowLayout->animationFinished(widget);
#endif

#endif

}

bool QWidgetAnimator::animating() const
{
   return ! m_animation_map.isEmpty();
}

QT_END_NAMESPACE
