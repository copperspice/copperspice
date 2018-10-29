/***********************************************************************
*
* Copyright (c) 2012-2018 Barbara Geller
* Copyright (c) 2012-2018 Ansel Sermersheim
* Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
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
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#ifndef QPLATFORMWINDOW_QPA_H
#define QPLATFORMWINDOW_QPA_H

#include <QtCore/qscopedpointer.h>
#include <QtCore/qrect.h>
#include <QtCore/qstring.h>
#include <QtGui/qwindowdefs.h>

QT_BEGIN_NAMESPACE

class QPlatformWindowPrivate;
class QWidget;
class QPlatformGLContext;

class Q_GUI_EXPORT QPlatformWindow
{
   Q_DECLARE_PRIVATE(QPlatformWindow)

 public:
   QPlatformWindow(QWidget *tlw);
   virtual ~QPlatformWindow();

   QWidget *widget() const;
   virtual void setGeometry(const QRect &rect);
   virtual QRect geometry() const;

   virtual void setVisible(bool visible);
   virtual Qt::WindowFlags setWindowFlags(Qt::WindowFlags flags);
   virtual Qt::WindowFlags windowFlags() const;
   virtual WId winId() const;
   virtual void setParent(const QPlatformWindow *window);

   virtual void setWindowTitle(const QString &title);
   virtual void raise();
   virtual void lower();

   virtual void setOpacity(qreal level);
   virtual void requestActivateWindow();

   virtual QPlatformGLContext *glContext() const;

 protected:
   QScopedPointer<QPlatformWindowPrivate> d_ptr;

 private:
   Q_DISABLE_COPY(QPlatformWindow)
};

QT_END_NAMESPACE

#endif //QPLATFORMWINDOW_H
