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

#ifndef QSCROLLAREA_H
#define QSCROLLAREA_H

#include <QtGui/qabstractscrollarea.h>

QT_BEGIN_NAMESPACE

#ifndef QT_NO_SCROLLAREA

class QScrollAreaPrivate;

class Q_GUI_EXPORT QScrollArea : public QAbstractScrollArea
{
   GUI_CS_OBJECT(QScrollArea)

   GUI_CS_PROPERTY_READ(widgetResizable, widgetResizable)
   GUI_CS_PROPERTY_WRITE(widgetResizable, setWidgetResizable)
   GUI_CS_PROPERTY_READ(alignment, alignment)
   GUI_CS_PROPERTY_WRITE(alignment, setAlignment)

 public:
   explicit QScrollArea(QWidget *parent = nullptr);
   ~QScrollArea();

   QWidget *widget() const;
   void setWidget(QWidget *widget);
   QWidget *takeWidget();

   bool widgetResizable() const;
   void setWidgetResizable(bool resizable);

   QSize sizeHint() const override;
   bool focusNextPrevChild(bool next) override;

   Qt::Alignment alignment() const;
   void setAlignment(Qt::Alignment);

   void ensureVisible(int x, int y, int xmargin = 50, int ymargin = 50);
   void ensureWidgetVisible(QWidget *childWidget, int xmargin = 50, int ymargin = 50);

 protected:
   QScrollArea(QScrollAreaPrivate &dd, QWidget *parent = nullptr);
   bool event(QEvent *) override;
   bool eventFilter(QObject *, QEvent *) override;
   void resizeEvent(QResizeEvent *) override;
   void scrollContentsBy(int dx, int dy) override;

 private:
   Q_DECLARE_PRIVATE(QScrollArea)
   Q_DISABLE_COPY(QScrollArea)
};

#endif // QT_NO_SCROLLAREA

QT_END_NAMESPACE

#endif // QSCROLLAREA_H
