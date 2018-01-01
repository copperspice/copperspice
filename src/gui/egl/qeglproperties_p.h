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

#ifndef QEGLPROPERTIES_P_H
#define QEGLPROPERTIES_P_H

#include <QtCore/qvarlengtharray.h>
#include <QtGui/qimage.h>
#include <qegl_p.h>

QT_BEGIN_NAMESPACE

class QX11Info;
class QPaintDevice;

class Q_GUI_EXPORT QEglProperties
{
 public:
   QEglProperties();
   QEglProperties(EGLConfig);
   QEglProperties(const QEglProperties &other) : props(other.props) {}
   ~QEglProperties() {}

   int value(int name) const;
   void setValue(int name, int value);
   bool removeValue(int name);

   bool isEmpty() const {
      return props[0] == EGL_NONE;
   }

   const EGLint *properties() const {
      return props.constData();
   }

   void setPixelFormat(QImage::Format pixelFormat);

#ifdef Q_WS_X11
   void setVisualFormat(const QX11Info *xinfo);
#endif

   void setDeviceType(int devType);
   void setPaintDeviceFormat(QPaintDevice *dev);
   void setRenderableType(QEgl::API api);

   bool reduceConfiguration();

   QString toString() const;

 private:
   QVarLengthArray<EGLint> props;
};

QT_END_NAMESPACE

#endif // QEGLPROPERTIES_P_H
