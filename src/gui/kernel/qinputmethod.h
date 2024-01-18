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

#ifndef QINPUTMETHOD_H
#define QINPUTMETHOD_H

#include <qobject.h>
#include <qrectf.h>

class QInputMethodPrivate;
class QWindow;
class QTransform;
class QInputMethodQueryEvent;

class Q_GUI_EXPORT QInputMethod : public QObject
{
   GUI_CS_OBJECT(QInputMethod)

   GUI_CS_ENUM(Action)

   GUI_CS_PROPERTY_READ(cursorRectangle, cursorRectangle)
   GUI_CS_PROPERTY_NOTIFY(cursorRectangle, cursorRectangleChanged)

   GUI_CS_PROPERTY_READ(keyboardRectangle, keyboardRectangle)
   GUI_CS_PROPERTY_NOTIFY(keyboardRectangle, keyboardRectangleChanged)

   GUI_CS_PROPERTY_READ(visible, isVisible)
   GUI_CS_PROPERTY_NOTIFY(visible, visibleChanged)

   GUI_CS_PROPERTY_READ(animating, isAnimating)
   GUI_CS_PROPERTY_NOTIFY(animating, animatingChanged)

   GUI_CS_PROPERTY_READ(locale, locale)
   GUI_CS_PROPERTY_NOTIFY(locale, localeChanged)

   GUI_CS_PROPERTY_READ(inputDirection, inputDirection)
   GUI_CS_PROPERTY_NOTIFY(inputDirection, inputDirectionChanged)

 public:
   enum Action {
      Click,
      ContextMenu
   };

   QTransform inputItemTransform() const;
   void setInputItemTransform(const QTransform &transform);

   QRectF inputItemRectangle() const;
   void setInputItemRectangle(const QRectF &rect);

   // in window coordinates
   QRectF cursorRectangle() const; // ### what if we have rotations for the item?

   // keyboard geometry in window coords
   QRectF keyboardRectangle() const;

   bool isVisible() const;
   void setVisible(bool visible);

   bool isAnimating() const;

   QLocale locale() const;
   Qt::LayoutDirection inputDirection() const;

   static QVariant queryFocusObject(Qt::InputMethodQuery query, QVariant argument); // TODO: QVariant by const-ref

   GUI_CS_SLOT_1(Public, void show())
   GUI_CS_SLOT_2(show)
   GUI_CS_SLOT_1(Public, void hide())
   GUI_CS_SLOT_2(hide)

   GUI_CS_SLOT_1(Public, void update(Qt::InputMethodQueries queries))
   GUI_CS_SLOT_2(update)
   GUI_CS_SLOT_1(Public, void reset())
   GUI_CS_SLOT_2(reset)
   GUI_CS_SLOT_1(Public, void commit())
   GUI_CS_SLOT_2(commit)

   GUI_CS_SLOT_1(Public, void invokeAction(Action a, int cursorPosition))
   GUI_CS_SLOT_2(invokeAction)

   GUI_CS_SIGNAL_1(Public, void cursorRectangleChanged())
   GUI_CS_SIGNAL_2(cursorRectangleChanged)
   GUI_CS_SIGNAL_1(Public, void keyboardRectangleChanged())
   GUI_CS_SIGNAL_2(keyboardRectangleChanged)
   GUI_CS_SIGNAL_1(Public, void visibleChanged())
   GUI_CS_SIGNAL_2(visibleChanged)
   GUI_CS_SIGNAL_1(Public, void animatingChanged())
   GUI_CS_SIGNAL_2(animatingChanged)
   GUI_CS_SIGNAL_1(Public, void localeChanged())
   GUI_CS_SIGNAL_2(localeChanged)
   GUI_CS_SIGNAL_1(Public, void inputDirectionChanged(Qt::LayoutDirection newDirection))
   GUI_CS_SIGNAL_2(inputDirectionChanged, newDirection)

 private:
   Q_DECLARE_PRIVATE(QInputMethod)

   QInputMethod();
   ~QInputMethod();

   QScopedPointer<QInputMethodPrivate> d_ptr;

   friend class QApplication;
   friend class QApplicationPrivate;
   friend class QPlatformInputContext;
};

#endif
