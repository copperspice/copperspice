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

#ifndef QSTYLEHINTS_H
#define QSTYLEHINTS_H

#include <qobject.h>
#include <qscopedpointer.h>

class QPlatformIntegration;
class QStyleHintsPrivate;

class Q_GUI_EXPORT QStyleHints : public QObject
{
   GUI_CS_OBJECT(QStyleHints)
   Q_DECLARE_PRIVATE(QStyleHints)

   GUI_CS_PROPERTY_READ(cursorFlashTime, cursorFlashTime)
   GUI_CS_PROPERTY_NOTIFY(cursorFlashTime, cursorFlashTimeChanged)
   GUI_CS_PROPERTY_FINAL(cursorFlashTime)
   GUI_CS_PROPERTY_READ(fontSmoothingGamma, fontSmoothingGamma)
   GUI_CS_PROPERTY_STORED(fontSmoothingGamma, false)
   GUI_CS_PROPERTY_CONSTANT(fontSmoothingGamma)
   GUI_CS_PROPERTY_FINAL(fontSmoothingGamma)
   GUI_CS_PROPERTY_READ(keyboardAutoRepeatRate, keyboardAutoRepeatRate)
   GUI_CS_PROPERTY_STORED(keyboardAutoRepeatRate, false)
   GUI_CS_PROPERTY_CONSTANT(keyboardAutoRepeatRate)
   GUI_CS_PROPERTY_FINAL(keyboardAutoRepeatRate)
   GUI_CS_PROPERTY_READ(keyboardInputInterval, keyboardInputInterval)
   GUI_CS_PROPERTY_NOTIFY(keyboardInputInterval, keyboardInputIntervalChanged)
   GUI_CS_PROPERTY_FINAL(keyboardInputInterval)
   GUI_CS_PROPERTY_READ(mouseDoubleClickInterval, mouseDoubleClickInterval)
   GUI_CS_PROPERTY_NOTIFY(mouseDoubleClickInterval, mouseDoubleClickIntervalChanged)
   GUI_CS_PROPERTY_FINAL(mouseDoubleClickInterval)
   GUI_CS_PROPERTY_READ(mousePressAndHoldInterval, mousePressAndHoldInterval)
   GUI_CS_PROPERTY_STORED(mousePressAndHoldInterval, false)
   GUI_CS_PROPERTY_CONSTANT(mousePressAndHoldInterval)
   GUI_CS_PROPERTY_FINAL(mousePressAndHoldInterval)
   GUI_CS_PROPERTY_READ(passwordMaskCharacter, passwordMaskCharacter)
   GUI_CS_PROPERTY_STORED(passwordMaskCharacter, false)
   GUI_CS_PROPERTY_CONSTANT(passwordMaskCharacter)
   GUI_CS_PROPERTY_FINAL(passwordMaskCharacter)
   GUI_CS_PROPERTY_READ(passwordMaskDelay, passwordMaskDelay)
   GUI_CS_PROPERTY_STORED(passwordMaskDelay, false)
   GUI_CS_PROPERTY_CONSTANT(passwordMaskDelay)
   GUI_CS_PROPERTY_FINAL(passwordMaskDelay)
   GUI_CS_PROPERTY_READ(setFocusOnTouchRelease, setFocusOnTouchRelease)
   GUI_CS_PROPERTY_STORED(setFocusOnTouchRelease, false)
   GUI_CS_PROPERTY_CONSTANT(setFocusOnTouchRelease)
   GUI_CS_PROPERTY_FINAL(setFocusOnTouchRelease)
   GUI_CS_PROPERTY_READ(showIsFullScreen, showIsFullScreen)
   GUI_CS_PROPERTY_STORED(showIsFullScreen, false)
   GUI_CS_PROPERTY_CONSTANT(showIsFullScreen)
   GUI_CS_PROPERTY_FINAL(showIsFullScreen)
   GUI_CS_PROPERTY_READ(showIsMaximized, showIsMaximized)
   GUI_CS_PROPERTY_STORED(showIsMaximized, false)
   GUI_CS_PROPERTY_CONSTANT(showIsMaximized)
   GUI_CS_PROPERTY_FINAL(showIsMaximized)
   GUI_CS_PROPERTY_READ(startDragDistance, startDragDistance)
   GUI_CS_PROPERTY_NOTIFY(startDragDistance, startDragDistanceChanged)
   GUI_CS_PROPERTY_FINAL(startDragDistance)
   GUI_CS_PROPERTY_READ(startDragTime, startDragTime)
   GUI_CS_PROPERTY_NOTIFY(startDragTime, startDragTimeChanged)
   GUI_CS_PROPERTY_FINAL(startDragTime)
   GUI_CS_PROPERTY_READ(startDragVelocity, startDragVelocity)
   GUI_CS_PROPERTY_STORED(startDragVelocity, false)
   GUI_CS_PROPERTY_CONSTANT(startDragVelocity)
   GUI_CS_PROPERTY_FINAL(startDragVelocity)
   GUI_CS_PROPERTY_READ(useRtlExtensions, useRtlExtensions)
   GUI_CS_PROPERTY_STORED(useRtlExtensions, false)
   GUI_CS_PROPERTY_CONSTANT(useRtlExtensions)
   GUI_CS_PROPERTY_FINAL(useRtlExtensions)
   GUI_CS_PROPERTY_READ(tabFocusBehavior, tabFocusBehavior)
   GUI_CS_PROPERTY_STORED(tabFocusBehavior, false)
   GUI_CS_PROPERTY_CONSTANT(tabFocusBehavior)
   GUI_CS_PROPERTY_FINAL(tabFocusBehavior)
   GUI_CS_PROPERTY_READ(singleClickActivation, singleClickActivation)
   GUI_CS_PROPERTY_STORED(singleClickActivation, false)
   GUI_CS_PROPERTY_CONSTANT(singleClickActivation)
   GUI_CS_PROPERTY_FINAL(singleClickActivation)

 public:
   ~QStyleHints();

   void setMouseDoubleClickInterval(int mouseDoubleClickInterval);
   int mouseDoubleClickInterval() const;
   int mousePressAndHoldInterval() const;
   void setStartDragDistance(int startDragDistance);
   int startDragDistance() const;
   void setStartDragTime(int startDragTime);
   int startDragTime() const;
   int startDragVelocity() const;
   void setKeyboardInputInterval(int keyboardInputInterval);
   int keyboardInputInterval() const;
   int keyboardAutoRepeatRate() const;
   void setCursorFlashTime(int cursorFlashTime);
   int cursorFlashTime() const;
   bool showIsFullScreen() const;
   bool showIsMaximized() const;
   int passwordMaskDelay() const;
   QChar passwordMaskCharacter() const;
   qreal fontSmoothingGamma() const;
   bool useRtlExtensions() const;
   bool setFocusOnTouchRelease() const;
   Qt::TabFocusBehavior tabFocusBehavior() const;
   bool singleClickActivation() const;

   GUI_CS_SIGNAL_1(Public, void cursorFlashTimeChanged(int cursorFlashTime))
   GUI_CS_SIGNAL_2(cursorFlashTimeChanged, cursorFlashTime)

   GUI_CS_SIGNAL_1(Public, void keyboardInputIntervalChanged(int keyboardInputInterval))
   GUI_CS_SIGNAL_2(keyboardInputIntervalChanged, keyboardInputInterval)

   GUI_CS_SIGNAL_1(Public, void mouseDoubleClickIntervalChanged(int mouseDoubleClickInterval))
   GUI_CS_SIGNAL_2(mouseDoubleClickIntervalChanged, mouseDoubleClickInterval)

   GUI_CS_SIGNAL_1(Public, void startDragDistanceChanged(int startDragDistance))
   GUI_CS_SIGNAL_2(startDragDistanceChanged, startDragDistance)

   GUI_CS_SIGNAL_1(Public, void startDragTimeChanged(int startDragTime))
   GUI_CS_SIGNAL_2(startDragTimeChanged, startDragTime)

 protected:
   QScopedPointer<QStyleHintsPrivate> d_ptr;

 private:
   friend class QApplication;
   QStyleHints();
};

#endif
