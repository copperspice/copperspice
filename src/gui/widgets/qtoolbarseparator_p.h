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

#ifndef QToolBarSeparator_P_H
#define QToolBarSeparator_P_H

#include <qwidget.h>

#ifndef QT_NO_TOOLBAR

class QStyleOption;
class QToolBar;

class QToolBarSeparator : public QWidget
{
   GUI_CS_OBJECT(QToolBarSeparator)
   Qt::Orientation orient;

 public:
   explicit QToolBarSeparator(QToolBar *parent);

   Qt::Orientation orientation() const;

   QSize sizeHint() const override;

   void paintEvent(QPaintEvent *) override;
   void initStyleOption(QStyleOption *option) const;

   GUI_CS_SLOT_1(Public, void setOrientation(Qt::Orientation orientation))
   GUI_CS_SLOT_2(setOrientation)
};

#endif // QT_NO_TOOLBAR



#endif // QDYNAMICTOOLBARSEPARATOR_P_H
