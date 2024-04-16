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

#ifndef DIRECTSHOWVIDEORENDERERCONTROL_H
#define DIRECTSHOWVIDEORENDERERCONTROL_H

#include <dshow.h>

#include <qvideorenderercontrol.h>

class DirectShowEventLoop;

#ifdef HAVE_EVR
class EVRCustomPresenter;
#endif

class DirectShowVideoRendererControl : public QVideoRendererControl
{
   CS_OBJECT(DirectShowVideoRendererControl)

 public:
   DirectShowVideoRendererControl(DirectShowEventLoop *loop, QObject *parent = nullptr);
   ~DirectShowVideoRendererControl();

   QAbstractVideoSurface *surface() const override;
   void setSurface(QAbstractVideoSurface *surface) override;

   IBaseFilter *filter();

   CS_SIGNAL_1(Public, void filterChanged())
   CS_SIGNAL_2(filterChanged)

 private:
   DirectShowEventLoop *m_loop;
   QAbstractVideoSurface *m_surface;
   IBaseFilter *m_filter;

#ifdef HAVE_EVR
   EVRCustomPresenter *m_evrPresenter;
#endif
};

#endif
