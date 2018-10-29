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

#include <qplatformwindow_qpa.h>

#include <QtGui/qwindowsysteminterface_qpa.h>
#include <QtGui/qwidget.h>

class QPlatformWindowPrivate
{
   QWidget *tlw;
   QRect rect;
   Qt::WindowFlags flags;
   friend class QPlatformWindow;
};

QPlatformWindow::QPlatformWindow(QWidget *tlw)
   : d_ptr(new QPlatformWindowPrivate)
{
   Q_D(QPlatformWindow);
   d->tlw = tlw;
   tlw->setPlatformWindow(this);
}

QPlatformWindow::~QPlatformWindow()
{
}

QWidget *QPlatformWindow::widget() const
{
   Q_D(const QPlatformWindow);
   return d->tlw;
}

void QPlatformWindow::setGeometry(const QRect &rect)
{
   Q_D(QPlatformWindow);
   d->rect = rect;
}

QRect QPlatformWindow::geometry() const
{
   Q_D(const QPlatformWindow);
   return d->rect;
}

void QPlatformWindow::setVisible(bool visible)
{
   Q_UNUSED(visible);
}

Qt::WindowFlags QPlatformWindow::setWindowFlags(Qt::WindowFlags flags)
{
   Q_D(QPlatformWindow);
   d->flags = flags;
   return flags;
}

Qt::WindowFlags QPlatformWindow::windowFlags() const
{
   Q_D(const QPlatformWindow);
   return d->flags;
}

WId QPlatformWindow::winId() const
{
   return WId(0);
}

void QPlatformWindow::setParent(const QPlatformWindow *parent)
{
   Q_UNUSED(parent);
   qWarning("This plugin does not support setParent!");
}

void QPlatformWindow::setWindowTitle(const QString &title)
{
   Q_UNUSED(title);
}

void QPlatformWindow::raise()
{
   qWarning("This plugin does not support raise()");
}

/*!
  Reimplement to be able to let Qt lower windows to the bottom of the desktop
*/
void QPlatformWindow::lower()
{
   qWarning("This plugin does not support lower()");
}

/*!
  Reimplement to be able to let Qt set the opacity level of a window
*/
void QPlatformWindow::setOpacity(qreal level)
{
   Q_UNUSED(level);
   qWarning("This plugin does not support setting window opacity");
}

/*!
  Reimplement to let Qt be able to request activation/focus for a window

  Some window systems will probably not have callbacks for this functionality,
  and then calling QWindowSystemInterface::handleWindowActivated(QWidget *w)
  would be sufficient.

  If the window system has some event handling/callbacks then call
  QWindowSystemInterface::handleWindowActivated(QWidget *w) when the window system
  gives the notification.

  Default implementation calls QWindowSystem::handleWindowActivated(QWidget *w)
*/
void QPlatformWindow::requestActivateWindow()
{
   QWindowSystemInterface::handleWindowActivated(widget());
}

/*!
  Reimplement to return the glContext associated with the window.
*/
QPlatformGLContext *QPlatformWindow::glContext() const
{
   return 0;
}

/*!
    \class QPlatformWindow
    \since 4.8
    \internal
    \preliminary
    \ingroup qpa

    \brief The QPlatformWindow class provides an abstraction for top-level windows.

    The QPlatformWindow abstraction is used by QWidget for all its top level widgets. It is being
    created by calling the createPlatformWindow function in the loaded QPlatformIntegration
    instance.

    QPlatformWindow is used to signal to the windowing system, how Qt persieves its frame.
    However, it is not concerned with how Qt renders into the window it represents.

    Top level QWidgets(tlw) will always have a QPlatformWindow. However, it is not necessary for
    all tlw to have a QWindowSurface. This is the case for QGLWidget. And could be the case for
    widgets where some  3.party renders into it.

    The platform specific window handle can be retrieved by the winId function.

    QPlatformWindow is also the way QPA defines how native child windows should be supported
    through the setParent function.

    The only way to retrieve a QPlatformGLContext in QPA is by calling the glContext() function
    on QPlatformWindow.

    \sa QWindowSurface, QWidget
*/
