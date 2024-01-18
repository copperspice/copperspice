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

#ifndef AVFVIDEOWIDGET_H
#define AVFVIDEOWIDGET_H

#include <QWidget>

@class AVPlayerLayer;
#if defined(Q_OS_DARWIN)
@class NSView;
#else
@class UIView;
#endif

class AVFVideoWidget : public QWidget
{
 public:
   AVFVideoWidget(QWidget *parent);
   virtual ~AVFVideoWidget();

   QSize sizeHint() const;
   Qt::AspectRatioMode aspectRatioMode() const;
   void setAspectRatioMode(Qt::AspectRatioMode mode);
   void setPlayerLayer(AVPlayerLayer *layer);

 protected:
   void resizeEvent(QResizeEvent *);
   void paintEvent(QPaintEvent *);

 private:
   void updateAspectRatio();
   void updatePlayerLayerBounds(const QSize &size);

   QSize m_nativeSize;
   Qt::AspectRatioMode m_aspectRatioMode;
   AVPlayerLayer *m_playerLayer;

#if defined(Q_OS_DARWIN)
   NSView *m_nativeView;
#else
   UIView *m_nativeView;
#endif

};

#endif
