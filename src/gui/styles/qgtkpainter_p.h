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

#ifndef QGTKPAINTER_P_H
#define QGTKPAINTER_P_H

#include <qglobal.h>

#if ! defined(QT_NO_STYLE_GTK)

#include <qsize.h>
#include <qrect.h>
#include <qpoint.h>
#include <qpixmap.h>
#include <qpainter.h>

#include <gtk/gtk.h>

class QGtkPainter
{

 public:
   QGtkPainter();
   virtual ~QGtkPainter();

   void reset(QPainter *painter = nullptr);

   void setAlphaSupport(bool value) {
      m_alpha = value;
   }

   void setClipRect(const QRect &rect) {
      m_cliprect = rect;
   }

   void setFlipHorizontal(bool value) {
      m_hflipped = value;
   }

   void setFlipVertical(bool value) {
      m_vflipped = value;
   }

   void setUsePixmapCache(bool value) {
      m_usePixmapCache = value;
   }

   virtual void paintBoxGap(GtkWidget *gtkWidget, const QString &part, const QRect &rect, GtkStateType state, GtkShadowType shadow,
      GtkPositionType gap_side, gint x, gint width, GtkStyle *style) = 0;

   virtual void paintBox(GtkWidget *gtkWidget, const QString &part, const QRect &rect, GtkStateType state, GtkShadowType shadow,
      GtkStyle *style, const QString &pmKey = QString()) = 0;

   virtual void paintHline(GtkWidget *gtkWidget, const QString &part, const QRect &rect, GtkStateType state, GtkStyle *style,
      int x1, int x2, int y, const QString &pmKey = QString()) = 0;

   virtual void paintVline(GtkWidget *gtkWidget, const QString &part, const QRect &rect, GtkStateType state, GtkStyle *style,
      int y1, int y2, int x, const QString &pmKey = QString()) = 0;

   virtual void paintExpander(GtkWidget *gtkWidget, const QString &part, const QRect &rect, GtkStateType state,
      GtkExpanderStyle expander_state, GtkStyle *style, const QString &pmKey = QString()) = 0;

   virtual void paintFocus(GtkWidget *gtkWidget, const QString &part, const QRect &rect, GtkStateType state, GtkStyle *style,
      const QString &pmKey = QString()) = 0;

   virtual void paintResizeGrip(GtkWidget *gtkWidget, const QString &part, const QRect &rect, GtkStateType state,
      GtkShadowType shadow, GdkWindowEdge edge, GtkStyle *style, const QString &pmKey = QString()) = 0;

   virtual void paintArrow(GtkWidget *gtkWidget, const QString &part, const QRect &arrowrect, GtkArrowType arrow_type,
      GtkStateType state, GtkShadowType shadow, gboolean fill, GtkStyle *style, const QString &pmKey = QString()) = 0;

   virtual void paintHandle(GtkWidget *gtkWidget, const QString &part, const QRect &rect, GtkStateType state, GtkShadowType shadow,
      GtkOrientation orientation, GtkStyle *style) = 0;

   virtual void paintSlider(GtkWidget *gtkWidget, const QString &part, const QRect &rect, GtkStateType state, GtkShadowType shadow,
      GtkStyle *style, GtkOrientation orientation, const QString &pmKey = QString()) = 0;

   virtual void paintShadow(GtkWidget *gtkWidget, const QString &part, const QRect &rect, GtkStateType state, GtkShadowType shadow,
      GtkStyle *style, const QString &pmKey = QString()) = 0;

   virtual void paintFlatBox(GtkWidget *gtkWidget, const QString &part, const QRect &rect, GtkStateType state, GtkShadowType shadow,
      GtkStyle *style, const QString & = QString()) = 0;

   virtual void paintExtention(GtkWidget *gtkWidget, const QString &part, const QRect &rect, GtkStateType state,
      GtkShadowType shadow, GtkPositionType gap_pos, GtkStyle *style) = 0;

   virtual void paintOption(GtkWidget *gtkWidget, const QRect &rect, GtkStateType state,
               GtkShadowType shadow, GtkStyle *style, const QString &detail) = 0;

   virtual void paintCheckbox(GtkWidget *gtkWidget, const QRect &rect, GtkStateType state,
               GtkShadowType shadow, GtkStyle *style, const QString &detail) = 0;

 protected:
   static QString uniqueName(const QString &key, GtkStateType state, GtkShadowType shadow, const QSize &size,
               GtkWidget *widget = nullptr);

   QPainter *m_painter;
   bool m_alpha;
   bool m_hflipped;
   bool m_vflipped;
   bool m_usePixmapCache;
   QRect m_cliprect;

};

#endif //!defined(QT_NO_STYLE_QGTK)

#endif // QGTKPAINTER_H
