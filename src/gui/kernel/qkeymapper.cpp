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

#include <qapplication.h>
#include <qkeymapper_p.h>
#include <qwidget.h>

QT_BEGIN_NAMESPACE

QKeyMapper::QKeyMapper()
   : QObject(0), d_ptr(new QKeyMapperPrivate)
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

extern bool qt_sendSpontaneousEvent(QObject *receiver, QEvent *event); // in qapplication_*.cpp
void QKeyMapper::changeKeyboard()
{
   instance()->d_func()->clearMappings();

   // inform all toplevel widgets of the change
   QEvent e(QEvent::KeyboardLayoutChange);
   QWidgetList list = QApplication::topLevelWidgets();
   for (int i = 0; i < list.size(); ++i) {
      QWidget *w = list.at(i);
      qt_sendSpontaneousEvent(w, &e);
   }
}

Q_GLOBAL_STATIC(QKeyMapper, keymapper)
/*!
    Returns the pointer to the single instance of QKeyMapper in the application.
    If none yet exists, the function ensures that one is created.
*/
QKeyMapper *QKeyMapper::instance()
{
   return keymapper();
}

QKeyMapperPrivate *qt_keymapper_private()
{
   return QKeyMapper::instance()->d_func();
}

QT_END_NAMESPACE
