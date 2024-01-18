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

#include <qgtkpainter_p.h>

#if ! defined(QT_NO_STYLE_GTK)

#include <qhexstring_p.h>

QGtkPainter::QGtkPainter()
{
   reset(0);
}

QGtkPainter::~QGtkPainter()
{
}

void QGtkPainter::reset(QPainter *painter)
{
   m_painter  = painter;
   m_alpha    = true;
   m_hflipped = false;
   m_vflipped = false;
   m_usePixmapCache = true;
   m_cliprect = QRect();
}

QString QGtkPainter::uniqueName(const QString &key, GtkStateType state, GtkShadowType shadow,
   const QSize &size, GtkWidget *widget)
{
   // Note the widget arg should ideally use the widget path, though would compromise performance
   QString tmp = key
      + HexString<uint>(state)
      + HexString<uint>(shadow)
      + HexString<uint>(size.width())
      + HexString<uint>(size.height())
      + HexString<quint64>(quint64(widget));
   return tmp;
}

#endif //!defined(QT_NO_STYLE_GTK)
