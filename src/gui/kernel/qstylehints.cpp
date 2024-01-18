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

#include <qstylehints.h>
#include <qdebug.h>

#include <qplatform_integration.h>
#include <qplatform_theme.h>

#include <qapplication_p.h>

static inline QVariant hint(QPlatformIntegration::StyleHint h)
{
   return QApplicationPrivate::platformIntegration()->styleHint(h);
}

static inline QVariant themeableHint(QPlatformTheme::ThemeHint th, QPlatformIntegration::StyleHint ih)
{
   if (! QCoreApplication::instance()) {
      qWarning("themeableHint() QApplication must be started before accessing a platform theme hint");
      return QVariant();
   }

   if (const QPlatformTheme *theme = QApplicationPrivate::platformTheme()) {
      const QVariant themeHint = theme->themeHint(th);

      if (themeHint.isValid()) {
         return themeHint;
      }
   }

   return QApplicationPrivate::platformIntegration()->styleHint(ih);
}

class QStyleHintsPrivate
{
   Q_DECLARE_PUBLIC(QStyleHints)

 public:
   inline QStyleHintsPrivate()
      : m_mouseDoubleClickInterval(-1)
      , m_startDragDistance(-1)
      , m_startDragTime(-1)
      , m_keyboardInputInterval(-1)
      , m_cursorFlashTime(-1)
   {}

   int m_mouseDoubleClickInterval;
   int m_startDragDistance;
   int m_startDragTime;
   int m_keyboardInputInterval;
   int m_cursorFlashTime;

 protected:
   QStyleHints *q_ptr;
};

QStyleHints::QStyleHints()
   : d_ptr(new QStyleHintsPrivate)
{
   d_ptr->q_ptr = this;
}

QStyleHints::~QStyleHints()
{
}

void QStyleHints::setMouseDoubleClickInterval(int mouseDoubleClickInterval)
{
   Q_D(QStyleHints);
   if (d->m_mouseDoubleClickInterval == mouseDoubleClickInterval) {
      return;
   }

   d->m_mouseDoubleClickInterval = mouseDoubleClickInterval;
   emit mouseDoubleClickIntervalChanged(mouseDoubleClickInterval);
}


int QStyleHints::mouseDoubleClickInterval() const
{
   Q_D(const QStyleHints);

   return d->m_mouseDoubleClickInterval >= 0 ?
      d->m_mouseDoubleClickInterval :
      themeableHint(QPlatformTheme::MouseDoubleClickInterval, QPlatformIntegration::MouseDoubleClickInterval).toInt();
}

int QStyleHints::mousePressAndHoldInterval() const
{
   return themeableHint(QPlatformTheme::MousePressAndHoldInterval, QPlatformIntegration::MousePressAndHoldInterval).toInt();

}
void QStyleHints::setStartDragDistance(int startDragDistance)
{
   Q_D(QStyleHints);

   if (d->m_startDragDistance == startDragDistance) {
      return;
   }

   d->m_startDragDistance = startDragDistance;

   emit startDragDistanceChanged(startDragDistance);
}

int QStyleHints::startDragDistance() const
{
   Q_D(const QStyleHints);

   return d->m_startDragDistance >= 0 ?
      d->m_startDragDistance :
      themeableHint(QPlatformTheme::StartDragDistance, QPlatformIntegration::StartDragDistance).toInt();
}

void QStyleHints::setStartDragTime(int startDragTime)
{
   Q_D(QStyleHints);
   if (d->m_startDragTime == startDragTime) {
      return;
   }
   d->m_startDragTime = startDragTime;
   emit startDragTimeChanged(startDragTime);
}

int QStyleHints::startDragTime() const
{
   Q_D(const QStyleHints);

   return d->m_startDragTime >= 0 ?
      d->m_startDragTime :
      themeableHint(QPlatformTheme::StartDragTime, QPlatformIntegration::StartDragTime).toInt();
}

/*!
    \property QStyleHints::startDragVelocity
    \brief the limit for the velocity, in pixels per second, that the mouse may
    be moved, with a button held down, for a drag and drop operation to begin.
    A value of 0 means there is no such limit.

    \sa startDragDistance, {Drag and Drop}
*/
int QStyleHints::startDragVelocity() const
{
   return themeableHint(QPlatformTheme::StartDragVelocity, QPlatformIntegration::StartDragVelocity).toInt();
}

void QStyleHints::setKeyboardInputInterval(int keyboardInputInterval)
{
   Q_D(QStyleHints);
   if (d->m_keyboardInputInterval == keyboardInputInterval) {
      return;
   }
   d->m_keyboardInputInterval = keyboardInputInterval;
   emit keyboardInputIntervalChanged(keyboardInputInterval);
}

/*!
    \property QStyleHints::keyboardInputInterval
    \brief the time limit, in milliseconds, that distinguishes a key press
    from two consecutive key presses.
*/
int QStyleHints::keyboardInputInterval() const
{
   Q_D(const QStyleHints);

   return d->m_keyboardInputInterval >= 0 ?
      d->m_keyboardInputInterval :
      themeableHint(QPlatformTheme::KeyboardInputInterval, QPlatformIntegration::KeyboardInputInterval).toInt();
}

/*!
    \property QStyleHints::keyboardAutoRepeatRate
    \brief the rate, in events per second,  in which additional repeated key
    presses will automatically be generated if a key is being held down.
*/
int QStyleHints::keyboardAutoRepeatRate() const
{
   return themeableHint(QPlatformTheme::KeyboardAutoRepeatRate, QPlatformIntegration::KeyboardAutoRepeatRate).toInt();
}

void QStyleHints::setCursorFlashTime(int cursorFlashTime)
{
   Q_D(QStyleHints);
   if (d->m_cursorFlashTime == cursorFlashTime) {
      return;
   }
   d->m_cursorFlashTime = cursorFlashTime;
   emit cursorFlashTimeChanged(cursorFlashTime);
}

/*!
    \property QStyleHints::cursorFlashTime
    \brief the text cursor's flash (blink) time in milliseconds.

    The flash time is the time used to display, invert and restore the
    caret display. Usually the text cursor is displayed for half the cursor
    flash time, then hidden for the same amount of time.
*/
int QStyleHints::cursorFlashTime() const
{
   Q_D(const QStyleHints);

   return d->m_cursorFlashTime >= 0 ?
      d->m_cursorFlashTime :
      themeableHint(QPlatformTheme::CursorFlashTime, QPlatformIntegration::CursorFlashTime).toInt();
}
bool QStyleHints::showIsFullScreen() const
{
   return hint(QPlatformIntegration::ShowIsFullScreen).toBool();
}


bool QStyleHints::showIsMaximized() const
{
   return hint(QPlatformIntegration::ShowIsMaximized).toBool();
}

int QStyleHints::passwordMaskDelay() const
{
   return themeableHint(QPlatformTheme::PasswordMaskDelay, QPlatformIntegration::PasswordMaskDelay).toInt();
}

QChar QStyleHints::passwordMaskCharacter() const
{
   return themeableHint(QPlatformTheme::PasswordMaskCharacter, QPlatformIntegration::PasswordMaskCharacter).toChar();
}


qreal QStyleHints::fontSmoothingGamma() const
{
   return hint(QPlatformIntegration::FontSmoothingGamma).toReal();
}

bool QStyleHints::useRtlExtensions() const
{
   return hint(QPlatformIntegration::UseRtlExtensions).toBool();
}

bool QStyleHints::setFocusOnTouchRelease() const
{
   return hint(QPlatformIntegration::SetFocusOnTouchRelease).toBool();
}

Qt::TabFocusBehavior QStyleHints::tabFocusBehavior() const
{
   return Qt::TabFocusBehavior(themeableHint(QPlatformTheme::TabFocusBehavior, QPlatformIntegration::TabFocusBehavior).toInt());
}

bool QStyleHints::singleClickActivation() const
{
   return themeableHint(QPlatformTheme::ItemViewActivateItemOnSingleClick,
         QPlatformIntegration::ItemViewActivateItemOnSingleClick).toBool();

}

