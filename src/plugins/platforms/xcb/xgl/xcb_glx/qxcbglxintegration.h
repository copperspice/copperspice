/***********************************************************************
*
* Copyright (c) 2012-2019 Barbara Geller
* Copyright (c) 2012-2019 Ansel Sermersheim
*
* Copyright (C) 2015 The Qt Company Ltd.
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

#ifndef QXCBGLXINTEGRATION_H
#define QXCBGLXINTEGRATION_H

#include "qxcbglintegration.h"

class QXcbNativeInterfaceHandler;

class QXcbGlxIntegration : public QXcbGlIntegration
{
 public:
   QXcbGlxIntegration();
   ~QXcbGlxIntegration();

   bool initialize(QXcbConnection *connection) override;
   bool handleXcbEvent(xcb_generic_event_t *event, uint responseType) override;

   QXcbWindow *createWindow(QWindow *window) const override;
   QPlatformOpenGLContext *createPlatformOpenGLContext(QOpenGLContext *context) const override;
   QPlatformOffscreenSurface *createPlatformOffscreenSurface(QOffscreenSurface *surface) const override;

   virtual bool supportsThreadedOpenGL() const override;
   virtual bool supportsSwitchableWidgetComposition() const override;

 private:
   QXcbConnection *m_connection;
   uint32_t m_glx_first_event;

   QScopedPointer<QXcbNativeInterfaceHandler> m_native_interface_handler;
};

#endif //QXCBGLXINTEGRATION_H
