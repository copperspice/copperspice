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

#ifndef QSCRIPTDEBUGGERSTACKWIDGETINTERFACE_P_H
#define QSCRIPTDEBUGGERSTACKWIDGETINTERFACE_P_H

#include <QtGui/qwidget.h>

QT_BEGIN_NAMESPACE

class QAbstractItemModel;
class QScriptDebuggerStackWidgetInterfacePrivate;

class QScriptDebuggerStackWidgetInterface: public QWidget
{
   SCRIPT_T_CS_OBJECT(QScriptDebuggerStackWidgetInterface)

 public:
   ~QScriptDebuggerStackWidgetInterface();

   virtual QAbstractItemModel *stackModel() const = 0;
   virtual void setStackModel(QAbstractItemModel *model) = 0;

   virtual int currentFrameIndex() const = 0;
   virtual void setCurrentFrameIndex(int frameIndex) = 0;

   CS_SIGNAL_1(Public, void currentFrameChanged(int newFrameIndex))
   CS_SIGNAL_2(currentFrameChanged, newFrameIndex)

 protected:
   QScriptDebuggerStackWidgetInterface(
      QScriptDebuggerStackWidgetInterfacePrivate &dd,
      QWidget *parent, Qt::WindowFlags flags);

 private:
   Q_DECLARE_PRIVATE(QScriptDebuggerStackWidgetInterface)
   Q_DISABLE_COPY(QScriptDebuggerStackWidgetInterface)
};

QT_END_NAMESPACE

#endif
