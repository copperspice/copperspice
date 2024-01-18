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

#include <qapplication.h>

#include <qplatform_integration.h>
#include <qwidget.h>

#include <qguiapplication_p.h>
#include <qkeymapper_p.h>

QKeyMapper::QKeyMapper()
   : QObject(nullptr), d_ptr(new QKeyMapperPrivate)
{
   d_ptr->q_ptr = this;
}

QKeyMapper::~QKeyMapper()
{
}

QList<int> QKeyMapper::possibleKeys(QKeyEvent *e)
{
   QList<int> result;

   if (!e->nativeScanCode()) {
      if (e->key() && (e->key() != Qt::Key_unknown)) {
         result << int(e->key() + e->modifiers());
      } else if (!e->text().isEmpty()) {
         result << int(e->text().at(0).unicode() + e->modifiers());
      }
      return result;
   }

   return instance()->d_func()->possibleKeys(e);
}

// in qapplication_*.cpp
extern bool qt_sendSpontaneousEvent(QObject *receiver, QEvent *event);

void QKeyMapper::changeKeyboard()
{
   instance()->d_func()->clearMappings();

   // emerald - support KeyboardLayoutChange
#if 0
   // inform all toplevel widgets of the change
   QEvent e(QEvent::KeyboardLayoutChange);
   QWidgetList list = QApplication::topLevelWidgets();

   for (int i = 0; i < list.size(); ++i) {
      QWidget *w = list.at(i);
      qt_sendSpontaneousEvent(w, &e);
   }
#endif

}

static QKeyMapper *keymapper()
{
   static QKeyMapper retval;
   return &retval;
}

QKeyMapper *QKeyMapper::instance()
{
   return keymapper();
}

QKeyMapperPrivate *qt_keymapper_private()
{
   return QKeyMapper::instance()->d_func();
}

QKeyMapperPrivate::QKeyMapperPrivate()
{
   keyboardInputLocale = QLocale::system();
   keyboardInputDirection = keyboardInputLocale.textDirection();
}

QKeyMapperPrivate::~QKeyMapperPrivate()
{
   // clearMappings();
}

void QKeyMapperPrivate::clearMappings()
{
}
QList<int> QKeyMapperPrivate::possibleKeys(QKeyEvent *e)
{
   QList<int> result = QGuiApplicationPrivate::platformIntegration()->possibleKeys(e);
   if (!result.isEmpty()) {
      return result;
   }
   if (e->key() && (e->key() != Qt::Key_unknown)) {
      result << int(e->key() + e->modifiers());
   } else if (!e->text().isEmpty()) {
      result << int(e->text().at(0).unicode() + e->modifiers());
   }
   return result;
}

