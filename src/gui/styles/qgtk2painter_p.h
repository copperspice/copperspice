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

#ifndef QGTK2PAINTER_P_H
#define QGTK2PAINTER_P_H


#include <qglobal.h>

#if !defined(QT_NO_STYLE_GTK)

#include <qgtkpainter_p.h>

class QGtk2Painter : public QGtkPainter
{
 public:
   QGtk2Painter();

   void paintBoxGap(GtkWidget *gtkWidget, const QString &part, const QRect &rect,
      GtkStateType state, GtkShadowType shadow, GtkPositionType gap_side, gint x,
      gint width, GtkStyle *style) override;

   void paintBox(GtkWidget *gtkWidget, const QString &part,
      const QRect &rect, GtkStateType state, GtkShadowType shadow, GtkStyle *style,
      const QString &pmKey = QString()) override;

   void paintHline(GtkWidget *gtkWidget, const QString &part, const QRect &rect, GtkStateType state, GtkStyle *style,
      int x1, int x2, int y, const QString &pmKey = QString()) override;

   void paintVline(GtkWidget *gtkWidget, const QString &part, const QRect &rect, GtkStateType state, GtkStyle *style,
      int y1, int y2, int x, const QString &pmKey = QString()) override;

   void paintExpander(GtkWidget *gtkWidget, const QString &part, const QRect &rect, GtkStateType state,
      GtkExpanderStyle expander_state, GtkStyle *style, const QString &pmKey = QString()) override;

   void paintFocus(GtkWidget *gtkWidget, const QString &part, const QRect &rect, GtkStateType state, GtkStyle *style,
      const QString &pmKey = QString()) override;

   void paintResizeGrip(GtkWidget *gtkWidget, const QString &part, const QRect &rect, GtkStateType state, GtkShadowType shadow,
      GdkWindowEdge edge, GtkStyle *style, const QString &pmKey = QString()) override;

   void paintArrow(GtkWidget *gtkWidget, const QString &part, const QRect &arrowrect, GtkArrowType arrow_type, GtkStateType state,
      GtkShadowType shadow,
      gboolean fill, GtkStyle *style, const QString &pmKey = QString()) override;

   void paintHandle(GtkWidget *gtkWidget, const QString &part, const QRect &rect,
      GtkStateType state, GtkShadowType shadow, GtkOrientation orientation, GtkStyle *style) override;

   void paintSlider(GtkWidget *gtkWidget, const QString &part, const QRect &rect, GtkStateType state, GtkShadowType shadow,
      GtkStyle *style, GtkOrientation orientation, const QString &pmKey = QString()) override;

   void paintShadow(GtkWidget *gtkWidget, const QString &part, const QRect &rect, GtkStateType state, GtkShadowType shadow,
      GtkStyle *style, const QString &pmKey = QString()) override;

   void paintFlatBox(GtkWidget *gtkWidget, const QString &part, const QRect &rect, GtkStateType state, GtkShadowType shadow,
      GtkStyle *style, const QString & = QString()) override;

   void paintExtention(GtkWidget *gtkWidget, const QString &part, const QRect &rect, GtkStateType state, GtkShadowType shadow,
      GtkPositionType gap_pos, GtkStyle *style) override;

   void paintOption(GtkWidget *gtkWidget, const QRect &rect, GtkStateType state, GtkShadowType shadow, GtkStyle *style,
      const QString &detail) override;
   void paintCheckbox(GtkWidget *gtkWidget, const QRect &rect, GtkStateType state, GtkShadowType shadow, GtkStyle *style,
      const QString &detail) override;

 private:
   QPixmap renderTheme(uchar *bdata, uchar *wdata, const QRect &rect) const;

   GtkWidget *m_window;
};

#endif //!defined(QT_NO_STYLE_QGTK)

#endif // QGTK2PAINTER_P_H
