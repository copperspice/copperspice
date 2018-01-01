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

#ifndef QGRAPHICSWIDGET_P_H
#define QGRAPHICSWIDGET_P_H

#include <qgraphicsitem_p.h>
#include <qgraphicswidget.h>
#include <QtGui/qfont.h>
#include <QtGui/qpalette.h>
#include <QtGui/qsizepolicy.h>
#include <QtGui/qstyle.h>

QT_BEGIN_NAMESPACE

class QGraphicsLayout;
class QStyleOptionTitleBar;

#if !defined(QT_NO_GRAPHICSVIEW)

class QGraphicsWidgetPrivate : public QGraphicsItemPrivate
{
   Q_DECLARE_PUBLIC(QGraphicsWidget)
 public:
   QGraphicsWidgetPrivate()
      : margins(0),
        layout(0),
        inheritedPaletteResolveMask(0),
        inheritedFontResolveMask(0),
        inSetGeometry(0),
        polished(0),
        inSetPos(0),
        autoFillBackground(0),
        refCountInvokeRelayout(0),
        focusPolicy(Qt::NoFocus),
        focusNext(0),
        focusPrev(0),
        windowFlags(0),
        windowData(0),
        setWindowFrameMargins(false),
        windowFrameMargins(0) {
   }
   virtual ~QGraphicsWidgetPrivate();

   void init(QGraphicsItem *parentItem, Qt::WindowFlags wFlags);
   qreal titleBarHeight(const QStyleOptionTitleBar &options) const;

   // Margins
   enum {Left, Top, Right, Bottom};
   mutable qreal *margins;
   void ensureMargins() const;

   void fixFocusChainBeforeReparenting(QGraphicsWidget *newParent, QGraphicsScene *oldScene, QGraphicsScene *newScene = 0);
   void setLayout_helper(QGraphicsLayout *l);

   // Layouts
   QGraphicsLayout *layout;
   void setLayoutDirection_helper(Qt::LayoutDirection direction);
   void resolveLayoutDirection();
   void _q_relayout();

   // Style
   QPalette palette;
   uint inheritedPaletteResolveMask;
   void setPalette_helper(const QPalette &palette);
   void resolvePalette(uint inheritedMask) override;
   void updatePalette(const QPalette &palette);
   QPalette naturalWidgetPalette() const;
   QFont font;
   uint inheritedFontResolveMask;
   void setFont_helper(const QFont &font);
   void resolveFont(uint inheritedMask) override;
   void updateFont(const QFont &font);
   QFont naturalWidgetFont() const;

   // Window specific
   void initStyleOptionTitleBar(QStyleOptionTitleBar *option);
   void adjustWindowFlags(Qt::WindowFlags *wFlags);
   void windowFrameMouseReleaseEvent(QGraphicsSceneMouseEvent *event);
   void windowFrameMousePressEvent(QGraphicsSceneMouseEvent *event);
   void windowFrameMouseMoveEvent(QGraphicsSceneMouseEvent *event);
   void windowFrameHoverMoveEvent(QGraphicsSceneHoverEvent *event);
   void windowFrameHoverLeaveEvent(QGraphicsSceneHoverEvent *event);
   bool hasDecoration() const;

   // Private Properties
   qreal width() const override;
   void setWidth(qreal) override;
   void resetWidth() override;

   qreal height() const override;
   void setHeight(qreal) override;
   void resetHeight() override;
   void setGeometryFromSetPos();

   // State
   inline int attributeToBitIndex(Qt::WidgetAttribute att) const {
      int bit = -1;
      switch (att) {
         case Qt::WA_SetLayoutDirection:
            bit = 0;
            break;
         case Qt::WA_RightToLeft:
            bit = 1;
            break;
         case Qt::WA_SetStyle:
            bit = 2;
            break;
         case Qt::WA_Resized:
            bit = 3;
            break;
         case Qt::WA_DeleteOnClose:
            bit = 4;
            break;
         case Qt::WA_NoSystemBackground:
            bit = 5;
            break;
         case Qt::WA_OpaquePaintEvent:
            bit = 6;
            break;
         case Qt::WA_SetPalette:
            bit = 7;
            break;
         case Qt::WA_SetFont:
            bit = 8;
            break;
         case Qt::WA_WindowPropagation:
            bit = 9;
            break;
         default:
            break;
      }
      return bit;
   }
   inline void setAttribute(Qt::WidgetAttribute att, bool value) {
      int bit = attributeToBitIndex(att);
      if (bit == -1) {
         qWarning("QGraphicsWidget::setAttribute: unsupported attribute %d", int(att));
         return;
      }
      if (value) {
         attributes |= (1 << bit);
      } else {
         attributes &= ~(1 << bit);
      }
   }
   inline bool testAttribute(Qt::WidgetAttribute att) const {
      int bit = attributeToBitIndex(att);
      if (bit == -1) {
         return false;
      }
      return (attributes & (1 << bit)) != 0;
   }
   // 32 bits
   quint32 attributes : 10;
   quint32 inSetGeometry : 1;
   quint32 polished: 1;
   quint32 inSetPos : 1;
   quint32 autoFillBackground : 1;
   quint32 refCountInvokeRelayout : 16;
   quint32 padding : 2;    // feel free to use

   // Focus
   Qt::FocusPolicy focusPolicy;
   QGraphicsWidget *focusNext;
   QGraphicsWidget *focusPrev;

   // Windows
   Qt::WindowFlags windowFlags;
   struct WindowData {
      QString windowTitle;
      QStyle::SubControl hoveredSubControl;
      Qt::WindowFrameSection grabbedSection;
      uint buttonMouseOver : 1;
      uint buttonSunken : 1;
      QRectF startGeometry;
      QRect buttonRect;
      WindowData()
         : hoveredSubControl(QStyle::SC_None)
         , grabbedSection(Qt::NoSection)
         , buttonMouseOver(false)
         , buttonSunken(false) {
      }
   } *windowData;
   void ensureWindowData();

   bool setWindowFrameMargins;
   mutable qreal *windowFrameMargins;
   void ensureWindowFrameMargins() const;

#ifndef QT_NO_ACTION
   QList<QAction *> actions;
#endif
};

#endif

QT_END_NAMESPACE

#endif //QGRAPHICSWIDGET_P_H

