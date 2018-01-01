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

#ifndef QGTKPAINTER_P_H
#define QGTKPAINTER_P_H

#include <QtCore/qglobal.h>

#if !defined(QT_NO_STYLE_GTK)

#include <QtGui/QCleanlooksStyle>
#include <QtGui/QPainter>
#include <QtGui/QPalette>
#include <QtGui/QFont>
#include <qgtkstyle_p.h>

QT_BEGIN_NAMESPACE

class QGtkPainter
{

 public:
   QGtkPainter(QPainter *painter);
   GtkStyle *getStyle(GtkWidget *gtkWidget);
   GtkStateType gtkState(const QStyleOption *option);

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

   void paintBoxGap(GtkWidget *gtkWidget, const gchar *part, const QRect &rect, GtkStateType state, GtkShadowType shadow, 
      GtkPositionType gap_side, gint x, gint width, GtkStyle *style);

   void paintBox(GtkWidget *gtkWidget, const gchar *part, const QRect &rect, GtkStateType state, GtkShadowType shadow, 
      GtkStyle *style,const QString &pmKey = QString());

   void paintHline(GtkWidget *gtkWidget, const gchar *part, const QRect &rect, GtkStateType state, GtkStyle *style,
      int x1, int x2, int y, const QString &pmKey = QString());

   void paintVline(GtkWidget *gtkWidget, const gchar *part, const QRect &rect, GtkStateType state, GtkStyle *style,
      int y1, int y2, int x, const QString &pmKey = QString());

   void paintExpander(GtkWidget *gtkWidget, const gchar *part, const QRect &rect, GtkStateType state,
      GtkExpanderStyle expander_state, GtkStyle *style, const QString &pmKey = QString());

   void paintFocus(GtkWidget *gtkWidget, const gchar *part, const QRect &rect, GtkStateType state, GtkStyle *style,
      const QString &pmKey = QString());

   void paintResizeGrip(GtkWidget *gtkWidget, const gchar *part, const QRect &rect, GtkStateType state,
      GtkShadowType shadow, GdkWindowEdge edge, GtkStyle *style, const QString &pmKey = QString());

   void paintArrow(GtkWidget *gtkWidget, const gchar *part, const QRect &arrowrect, GtkArrowType arrow_type,
      GtkStateType state, GtkShadowType shadow, gboolean fill, GtkStyle *style, const QString &pmKey = QString());

   void paintHandle(GtkWidget *gtkWidget, const gchar *part, const QRect &rect, GtkStateType state, GtkShadowType shadow, 
      GtkOrientation orientation, GtkStyle *style);

   void paintSlider(GtkWidget *gtkWidget, const gchar *part, const QRect &rect, GtkStateType state, GtkShadowType shadow,
      GtkStyle *style, GtkOrientation orientation, const QString &pmKey = QString());

   void paintShadow(GtkWidget *gtkWidget, const gchar *part, const QRect &rect, GtkStateType state, GtkShadowType shadow,
      GtkStyle *style, const QString &pmKey = QString());

   void paintFlatBox(GtkWidget *gtkWidget, const gchar *part, const QRect &rect, GtkStateType state, GtkShadowType shadow,
      GtkStyle *style, const QString & = QString());

   void paintExtention(GtkWidget *gtkWidget, const gchar *part, const QRect &rect, GtkStateType state,
      GtkShadowType shadow,GtkPositionType gap_pos, GtkStyle *style);

   void paintOption(GtkWidget *gtkWidget, const QRect &rect, GtkStateType state, GtkShadowType shadow, GtkStyle *style,
      const QString &detail);

   void paintCheckbox(GtkWidget *gtkWidget, const QRect &rect, GtkStateType state, GtkShadowType shadow, GtkStyle *style,
      const QString &detail);

   static QPixmap getIcon(const char *iconName, GtkIconSize size = GTK_ICON_SIZE_BUTTON);

 private:
   QPixmap renderTheme(uchar *bdata, uchar *wdata, const QRect &);

   GtkWidget *m_window;
   QPainter *m_painter;
   bool m_alpha;
   bool m_hflipped;
   bool m_vflipped;
   bool m_usePixmapCache;
   QRect m_cliprect;

};

QT_END_NAMESPACE

#endif //!defined(QT_NO_STYLE_QGTK)

#endif // QGTKPAINTER_H
