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

// internal
QInputMethod::QInputMethod()
   : d_ptr(new QInputMethodPrivate)
{
}

// internal
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

/*!
    \property QInputMethod::keyboardRectangle
    \brief Virtual keyboard's geometry in window coordinates.

    This might be an empty rectangle if it is not possible to know the geometry
    of the keyboard. This is the case for a floating keyboard on android.
*/
QRectF QInputMethod::keyboardRectangle() const
{
   Q_D(const QInputMethod);
   QPlatformInputContext *ic = d->platformInputContext();
   if (ic) {
      return ic->keyboardRect();
   }
   return QRectF();
}

/*!
    Requests virtual keyboard to open. If the platform
    doesn't provide virtual keyboard the visibility
    remains false.

    Normally applications should not need to call this
    function, keyboard should automatically open when
    the text editor gains focus.
*/
void QInputMethod::show()
{
   Q_D(QInputMethod);
   QPlatformInputContext *ic = d->platformInputContext();
   if (ic) {
      ic->showInputPanel();
   }
}

/*!
    Requests virtual keyboard to close.

    Normally applications should not need to call this function,
    keyboard should automatically close when the text editor loses
    focus, for example when the parent view is closed.
*/
void QInputMethod::hide()
{
   Q_D(QInputMethod);
   QPlatformInputContext *ic = d->platformInputContext();
   if (ic) {
      ic->hideInputPanel();
   }
}

/*!
    \property QInputMethod::visible
    \brief Virtual keyboard's visibility on the screen

    Input method visibility remains false for devices
    with no virtual keyboards.

    \sa show(), hide()
*/
bool QInputMethod::isVisible() const
{
   Q_D(const QInputMethod);
   QPlatformInputContext *ic = d->platformInputContext();
   if (ic) {
      return ic->isInputPanelVisible();
   }
   return false;
}

/*!
    Controls the keyboard visibility. Equivalent
    to calling show() (if \a visible is \c true)
    or hide() (if \a visible is \c false).

    \sa show(), hide()
*/
void QInputMethod::setVisible(bool visible)
{
   visible ? show() : hide();
}

/*!
    \property QInputMethod::animating
    \brief True when the virtual keyboard is being opened or closed.

    Animating is false when keyboard is fully open or closed.
    When \c animating is \c true and \c visibility is \c true keyboard
    is being opened. When \c animating is \c true and \c visibility is
    false keyboard is being closed.
*/

bool QInputMethod::isAnimating() const
{
   Q_D(const QInputMethod);
   QPlatformInputContext *ic = d->platformInputContext();
   if (ic) {
      return ic->isAnimating();
   }
   return false;
}

/*!
    \property QInputMethod::locale
    \brief Current input locale.
*/
QLocale QInputMethod::locale() const
{
   Q_D(const QInputMethod);
   QPlatformInputContext *ic = d->platformInputContext();
   if (ic) {
      return ic->locale();
   }
   return QLocale::c();
}

/*!
    \property QInputMethod::inputDirection
    \brief Current input direction.
*/
Qt::LayoutDirection QInputMethod::inputDirection() const
{
   Q_D(const QInputMethod);
   QPlatformInputContext *ic = d->platformInputContext();
   if (ic) {
      return ic->inputDirection();
   }
   return Qt::LeftToRight;
}

/*!
    Called by the input item to inform the platform input methods when there has been
    state changes in editor's input method query attributes. When calling the function
    \a queries parameter has to be used to tell what has changes, which input method
    can use to make queries for attributes it's interested with QInputMethodQueryEvent.

    In particular calling update whenever the cursor position changes is important as
    that often causes other query attributes like surrounding text and text selection
    to change as well. The attributes that often change together with cursor position
    have been grouped in Qt::ImQueryInput value for convenience.
*/
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

/*!
    Resets the input method state. For example, a text editor normally calls
    this method before inserting a text to make widget ready to accept a text.

    Input method resets automatically when the focused editor changes.
*/
void QInputMethod::reset()
{
   Q_D(QInputMethod);
   QPlatformInputContext *ic = d->platformInputContext();
   if (ic) {
      ic->reset();
   }
}

/*!
    Commits the word user is currently composing to the editor. The function is
    mostly needed by the input methods with text prediction features and by the
    methods where the script used for typing characters is different from the
    script that actually gets appended to the editor. Any kind of action that
    interrupts the text composing needs to flush the composing state by calling the
    commit() function, for example when the cursor is moved elsewhere.
*/
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

