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

#include <qinputcontrol_p.h>
#include <qevent.h>

QInputControl::QInputControl(Type type, QObject *parent)
   : QObject(parent), m_type(type)
{
}

bool QInputControl::isAcceptableInput(const QKeyEvent *event) const
{
   const QString text = event->text();
   if (text.isEmpty()) {
      return false;
   }

   const QChar c = text.at(0);

   // Formatting characters such as ZWNJ, ZWJ, RLM, etc. This needs to go before the
   // next test, since CTRL+SHIFT is sometimes used to input it on Windows.
   if (c.category() == QChar::Other_Format) {
      return true;
   }

   // QTBUG-35734: ignore Ctrl/Ctrl+Shift; accept only AltGr (Alt+Ctrl) on German keyboards
   if (event->modifiers() == Qt::ControlModifier
      || event->modifiers() == (Qt::ShiftModifier | Qt::ControlModifier)) {
      return false;
   }

   if (c.isPrint()) {
      return true;
   }

   if (c.category() == QChar::Other_PrivateUse) {
      return true;
   }

   if (m_type == TextEdit && c == QLatin1Char('\t')) {
      return true;
   }

   return false;
}


