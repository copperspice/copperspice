/***********************************************************************
*
* Copyright (c) 2012-2020 Barbara Geller
* Copyright (c) 2012-2020 Ansel Sermersheim
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

#ifndef QWINDOWSDIALOGHELPER_H
#define QWINDOWSDIALOGHELPER_H

#include "qtwindows_additional.h"
#include <qplatform_dialoghelper.h>
#include <qplatform_theme.h>
#include <QStringList>
#include <QSharedPointer>

class QFileDialog;
class QDialog;
class QThread;
class QWindowsNativeDialogBase;

namespace QWindowsDialogs {

void eatMouseMove();

bool useHelper(QPlatformTheme::DialogType type);
QPlatformDialogHelper *createHelper(QPlatformTheme::DialogType type);
} // namespace

template <class BaseClass>
class QWindowsDialogHelperBase : public BaseClass
{
   Q_DISABLE_COPY(QWindowsDialogHelperBase)

 public:
   typedef QSharedPointer<QWindowsNativeDialogBase> QWindowsNativeDialogBasePtr;
   ~QWindowsDialogHelperBase() {
      cleanupThread();
   }

   void exec() override;
   bool show(Qt::WindowFlags windowFlags, Qt::WindowModality windowModality, QWindow *parent) override;
   void hide() override;

   virtual bool supportsNonModalDialog(const QWindow *parent = nullptr) const {
      return true;
   }

 protected:
   QWindowsDialogHelperBase();
   QWindowsNativeDialogBase *nativeDialog() const;
   inline bool hasNativeDialog() const {
      return m_nativeDialog;
   }
   void timerEvent(QTimerEvent *) override;

 private:
   virtual QWindowsNativeDialogBase *createNativeDialog() = 0;
   inline QWindowsNativeDialogBase *ensureNativeDialog();
   inline void startDialogThread();
   inline void stopTimer();
   void cleanupThread();

   QWindowsNativeDialogBasePtr m_nativeDialog;
   HWND m_ownerWindow;
   int m_timerId;
   QThread *m_thread;
};

#endif
