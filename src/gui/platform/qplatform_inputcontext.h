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

#ifndef QPLATFORM_INPUTCONTEXT_H
#define QPLATFORM_INPUTCONTEXT_H

#include <qinputmethod.h>

class QPlatformInputContextPrivate;

class Q_GUI_EXPORT QPlatformInputContext : public QObject
{
   GUI_CS_OBJECT(QPlatformInputContext)
   Q_DECLARE_PRIVATE(QPlatformInputContext)

 public:
   enum Capability {
      HiddenTextCapability = 0x1
   };

   QPlatformInputContext();
   virtual ~QPlatformInputContext();

   virtual bool isValid() const;
   virtual bool hasCapability(Capability capability) const;

   virtual void reset();
   virtual void commit();
   virtual void update(Qt::InputMethodQueries);
   virtual void invokeAction(QInputMethod::Action action, int cursorPosition);
   virtual bool filterEvent(const QEvent *event);
   virtual QRectF keyboardRect() const;
   void emitKeyboardRectChanged();

   virtual bool isAnimating() const;
   void emitAnimatingChanged();

   virtual void showInputPanel();
   virtual void hideInputPanel();
   virtual bool isInputPanelVisible() const;
   void emitInputPanelVisibleChanged();

   virtual QLocale locale() const;
   void emitLocaleChanged();
   virtual Qt::LayoutDirection inputDirection() const;
   void emitInputDirectionChanged(Qt::LayoutDirection newDirection);

   virtual void setFocusObject(QObject *object);
   bool inputMethodAccepted() const;

 protected:
   QScopedPointer<QPlatformInputContextPrivate> d_ptr;

 private:
   friend class QApplication;            // QGuiApplication
   friend class QApplicationPrivate;     // QGuiApplicationPrivate
   friend class QInputMethod;
};

#endif
