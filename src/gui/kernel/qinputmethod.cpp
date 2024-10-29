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

#include <qinputmethod.h>
#include <qinputmethod_p.h>
#include <qguiapplication.h>
#include <qtimer.h>
#include <qplatform_inputcontext_p.h>
#include <QDebug>

QInputMethod::QInputMethod()
   : d_ptr(new QInputMethodPrivate)
{
}

QInputMethod::~QInputMethod()
{
}

QTransform QInputMethod::inputItemTransform() const
{
   Q_D(const QInputMethod);
   return d->inputItemTransform;
}

void QInputMethod::setInputItemTransform(const QTransform &transform)
{
   Q_D(QInputMethod);
   if (d->inputItemTransform == transform) {
      return;
   }

   d->inputItemTransform = transform;
   emit cursorRectangleChanged();
}

QRectF QInputMethod::inputItemRectangle() const
{
   Q_D(const QInputMethod);
   return d->inputRectangle;
}

void QInputMethod::setInputItemRectangle(const QRectF &rect)
{
   Q_D(QInputMethod);
   d->inputRectangle = rect;
}

QRectF QInputMethod::cursorRectangle() const
{
   Q_D(const QInputMethod);

   QObject *focusObject = qGuiApp->focusObject();
   if (!focusObject) {
      return QRectF();
   }

   QInputMethodQueryEvent query(Qt::ImCursorRectangle);
   QGuiApplication::sendEvent(focusObject, &query);
   QRectF r = query.value(Qt::ImCursorRectangle).toRectF();

   if (!r.isValid()) {
      return QRectF();
   }

   return d->inputItemTransform.mapRect(r);
}

QRectF QInputMethod::keyboardRectangle() const
{
   Q_D(const QInputMethod);
   QPlatformInputContext *ic = d->platformInputContext();
   if (ic) {
      return ic->keyboardRect();
   }
   return QRectF();
}

void QInputMethod::show()
{
   Q_D(QInputMethod);
   QPlatformInputContext *ic = d->platformInputContext();
   if (ic) {
      ic->showInputPanel();
   }
}

void QInputMethod::hide()
{
   Q_D(QInputMethod);
   QPlatformInputContext *ic = d->platformInputContext();
   if (ic) {
      ic->hideInputPanel();
   }
}

bool QInputMethod::isVisible() const
{
   Q_D(const QInputMethod);
   QPlatformInputContext *ic = d->platformInputContext();
   if (ic) {
      return ic->isInputPanelVisible();
   }
   return false;
}

void QInputMethod::setVisible(bool visible)
{
   visible ? show() : hide();
}

bool QInputMethod::isAnimating() const
{
   Q_D(const QInputMethod);
   QPlatformInputContext *ic = d->platformInputContext();
   if (ic) {
      return ic->isAnimating();
   }
   return false;
}

QLocale QInputMethod::locale() const
{
   Q_D(const QInputMethod);
   QPlatformInputContext *ic = d->platformInputContext();
   if (ic) {
      return ic->locale();
   }
   return QLocale::c();
}

Qt::LayoutDirection QInputMethod::inputDirection() const
{
   Q_D(const QInputMethod);
   QPlatformInputContext *ic = d->platformInputContext();
   if (ic) {
      return ic->inputDirection();
   }
   return Qt::LeftToRight;
}

void QInputMethod::update(Qt::InputMethodQueries queries)
{
   Q_D(QInputMethod);

   if (queries & Qt::ImEnabled) {
      QObject *focus = qApp->focusObject();
      bool enabled = d->objectAcceptsInputMethod(focus);
      QPlatformInputContextPrivate::setInputMethodAccepted(enabled);
   }

   QPlatformInputContext *ic = d->platformInputContext();
   if (ic) {
      ic->update(queries);
   }

   if (queries & Qt::ImCursorRectangle) {
      emit cursorRectangleChanged();
   }
}

void QInputMethod::reset()
{
   Q_D(QInputMethod);
   QPlatformInputContext *ic = d->platformInputContext();
   if (ic) {
      ic->reset();
   }
}

void QInputMethod::commit()
{
   Q_D(QInputMethod);
   QPlatformInputContext *ic = d->platformInputContext();
   if (ic) {
      ic->commit();
   }
}

void QInputMethod::invokeAction(Action a, int cursorPosition)
{
   Q_D(QInputMethod);
   QPlatformInputContext *ic = d->platformInputContext();
   if (ic) {
      ic->invokeAction(a, cursorPosition);
   }
}

bool QInputMethodPrivate::objectAcceptsInputMethod(QObject *object)
{
   bool enabled = false;
   if (object) {
      QInputMethodQueryEvent query(Qt::ImEnabled);
      QGuiApplication::sendEvent(object, &query);
      enabled = query.value(Qt::ImEnabled).toBool();
   }

   return enabled;
}

QVariant QInputMethod::queryFocusObject(Qt::InputMethodQuery query, QVariant argument)
{
   QVariant retval;
   QObject *focusObject = qGuiApp->focusObject();

   if (! focusObject) {
      return retval;
   }

   bool newMethodWorks = QMetaObject::invokeMethod(focusObject, "inputMethodQuery", Qt::DirectConnection,
                  Q_RETURN_ARG(QVariant, retval), Q_ARG(Qt::InputMethodQuery, query), Q_ARG(QVariant, argument));

   if (newMethodWorks) {
      return retval;
   }

   QInputMethodQueryEvent queryEvent(query);
   QCoreApplication::sendEvent(focusObject, &queryEvent);

   return queryEvent.value(query);
}
