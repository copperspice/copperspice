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

#include <qaccessibleobject.h>

#ifndef QT_NO_ACCESSIBILITY

#include <qapplication.h>
#include <qwindow.h>
#include <qpointer.h>
#include <qmetaobject.h>

class QAccessibleObjectPrivate
{
 public:
   QPointer<QObject> object;
};

QAccessibleObject::QAccessibleObject(QObject *object)
{
   d = new QAccessibleObjectPrivate;
   d->object = object;
}

QAccessibleObject::~QAccessibleObject()
{
   delete d;
}

QObject *QAccessibleObject::object() const
{
   return d->object;
}

bool QAccessibleObject::isValid() const
{
   return ! d->object.isNull();
}

QRect QAccessibleObject::rect() const
{
   return QRect();
}

void QAccessibleObject::setText(QAccessible::Text text, const QString &str)
{
   (void) text;
   (void) str;
}

QAccessibleInterface *QAccessibleObject::childAt(int x, int y) const
{
   for (int i = 0; i < childCount(); ++i) {
      QAccessibleInterface *childIface = child(i);

      Q_ASSERT(childIface);
      if (childIface->rect().contains(x, y)) {
         return childIface;
      }
   }

   return nullptr;
}

QAccessibleApplication::QAccessibleApplication()
   : QAccessibleObject(qApp)
{
}

QWindow *QAccessibleApplication::window() const
{
   return nullptr;
}

// all toplevel widgets except popups and the desktop
static QObjectList topLevelObjects()
{
   QObjectList list;
   const QWindowList tlw(QGuiApplication::topLevelWindows());

   for (int i = 0; i < tlw.count(); ++i) {
      QWindow *w = tlw.at(i);

      if (w->type() != Qt::Popup && w->type() != Qt::Desktop) {
         if (QAccessibleInterface *root = w->accessibleRoot()) {
            if (root->object()) {
               list.append(root->object());
            }
         }
      }
   }

   return list;
}

int QAccessibleApplication::childCount() const
{
   return topLevelObjects().count();
}

int QAccessibleApplication::indexOfChild(const QAccessibleInterface *child) const
{
   if (! child) {
      return -1;
   }

   const QObjectList tlw(topLevelObjects());
   return tlw.indexOf(child->object());
}

QAccessibleInterface *QAccessibleApplication::parent() const
{

   return nullptr;
}

QAccessibleInterface *QAccessibleApplication::child(int index) const
{
   const QObjectList tlo(topLevelObjects());
   if (index >= 0 && index < tlo.count()) {
      return QAccessible::queryAccessibleInterface(tlo.at(index));
   }
   return nullptr;
}


QAccessibleInterface *QAccessibleApplication::focusChild() const
{
   if (QWindow *window = QGuiApplication::focusWindow()) {
      return window->accessibleRoot();
   }
   return nullptr;
}

QString QAccessibleApplication::text(QAccessible::Text t) const
{
   switch (t) {
      case QAccessible::Name:
         return QGuiApplication::applicationName();
      case QAccessible::Description:
         return QGuiApplication::applicationFilePath();
      default:
         break;
   }
   return QString();
}

QAccessible::Role QAccessibleApplication::role() const
{
   return QAccessible::Application;
}



QAccessible::State QAccessibleApplication::state() const
{
   return QAccessible::State();
}


#endif
