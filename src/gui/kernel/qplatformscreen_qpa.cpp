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

#include <qplatformscreen_qpa.h>
#include <QtGui/qapplication.h>
#include <qapplication_p.h>
#include <QtGui/qdesktopwidget.h>
#include <QtGui/qplatformintegration_qpa.h>
#include <QtGui/qwidget.h>
#include <qwidget_p.h>

/*!
    Return the given top level widget for a given position.

    Default implementation retrieves a list of all top level widgets and finds the first widget
    which contains point \a pos
*/
QWidget *QPlatformScreen::topLevelAt(const QPoint &pos) const
{
   QWidgetList list = QApplication::topLevelWidgets();
   for (int i = list.size() - 1; i >= 0; --i) {
      QWidget *w = list[i];
      //### mask is ignored
      if (w != QApplication::desktop() && w->isVisible() && w->geometry().contains(pos)) {
         return w;
      }
   }

   return 0;
}

/*!
    Reimplement this function in subclass to return the physical size of the
    screen. This function is used by QFont to convert point sizes to pixel
    sizes.

    The default implementation takes the pixel size of the screen, considers a
    resolution of 100 dots per inch, and returns the calculated physical size.
    A device with a screen that has different resolutions will need to be
    supported by a suitable reimplementation of this function.
*/
QSize QPlatformScreen::physicalSize() const
{
   static const int dpi = 100;
   int width = geometry().width() / dpi * qreal(25.4) ;
   int height = geometry().height() / dpi * qreal(25.4) ;
   return QSize(width, height);
}

Q_GUI_EXPORT extern QWidgetPrivate *qt_widget_private(QWidget *widget);
QPlatformScreen *QPlatformScreen::platformScreenForWidget(const QWidget *widget)
{
   QWidget *window = widget->window();
   QWidgetPrivate *windowPrivate = qt_widget_private(window);
   QTLWExtra *topData = windowPrivate->topData();
   QPlatformIntegration *integration =
      QApplicationPrivate::platformIntegration();
   return integration->screens()[topData->screenIndex];
}

/*!
    \class QPlatformScreen
    \since 4.8
    \internal
    \preliminary
    \ingroup qpa

    \brief The QPlatformScreen class provides an abstraction for visual displays.

    Many window systems has support for retrieving information on the attached displays. To be able
    to query the display QPA uses QPlatformScreen. Qt its self is most dependent on the
    physicalSize() function, since this is the function it uses to calculate the dpi to use when
    converting point sizes to pixels sizes. However, this is unfortunate on some systems, as the
    native system fakes its dpi size.

    QPlatformScreen is also used by the public api QDesktopWidget for information about the desktop.
 */

/*! \fn QRect QPlatformScreen::geometry() const = 0
    Reimplement in subclass to return the pixel geometry of the screen
*/

/*! \fn QRect QPlatformScreen::availableGeometry() const
    Reimplement in subclass to return the pixel geometry of the available space
    This normally is the desktop screen minus the task manager, global menubar etc.
*/

/*! \fn int QPlatformScreen::depth() const = 0
    Reimplement in subclass to return current depth of the screen
*/

/*! \fn QImage::Format QPlatformScreen::format() const = 0
    Reimplement in subclass to return the image format which corresponds to the screen format
*/

