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

#ifndef QOPENGLWINDOW_H
#define QOPENGLWINDOW_H

#include <qglobal.h>

#ifndef QT_NO_OPENGL

#include <qpaintdevicewindow.h>
#include <qopenglcontext.h>
#include <qimage.h>

class QOpenGLWindowPrivate;

class Q_GUI_EXPORT QOpenGLWindow : public QPaintDeviceWindow
{
   GUI_CS_OBJECT(QOpenGLWindow)

 public:
   enum UpdateBehavior {
      NoPartialUpdate,
      PartialUpdateBlit,
      PartialUpdateBlend
   };

   explicit QOpenGLWindow(UpdateBehavior updateBehavior = NoPartialUpdate, QWindow *parent = nullptr);
   explicit QOpenGLWindow(QOpenGLContext *shareContext, UpdateBehavior updateBehavior = NoPartialUpdate,
      QWindow *parent = nullptr);

   QOpenGLWindow(const QOpenGLWindow &) = delete;
   QOpenGLWindow &operator=(const QOpenGLWindow &) = delete;

   ~QOpenGLWindow();

   UpdateBehavior updateBehavior() const;
   bool isValid() const;

   void makeCurrent();
   void doneCurrent();

   QOpenGLContext *context() const;
   QOpenGLContext *shareContext() const;

   GLuint defaultFramebufferObject() const;

   QImage grabFramebuffer();

   GUI_CS_SIGNAL_1(Public, void frameSwapped())
   GUI_CS_SIGNAL_2(frameSwapped)

 protected:
   virtual void initializeGL();
   virtual void resizeGL(int w, int h);
   virtual void paintGL();
   virtual void paintUnderGL();
   virtual void paintOverGL();

   void paintEvent(QPaintEvent *event) override;
   void resizeEvent(QResizeEvent *event) override;
   int metric(PaintDeviceMetric metric) const override;
   QPaintDevice *redirected(QPoint *) const override;

 private:
   Q_DECLARE_PRIVATE(QOpenGLWindow)
};

#endif // QT_NO_OPENGL

#endif
