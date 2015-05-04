/***********************************************************************
*
* Copyright (c) 2012-2015 Barbara Geller
* Copyright (c) 2012-2015 Ansel Sermersheim
* Copyright (c) 2012-2014 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
*
* This file is part of CopperSpice.
*
* CopperSpice is free software: you can redistribute it and/or 
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 as published by the Free Software Foundation.
*
* CopperSpice is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with CopperSpice.  If not, see 
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#ifndef QCOMMANDLINKBUTTON_H
#define QCOMMANDLINKBUTTON_H

#include <QtGui/qpushbutton.h>

QT_BEGIN_NAMESPACE

class QCommandLinkButtonPrivate;

class Q_GUI_EXPORT QCommandLinkButton: public QPushButton
{
   CS_OBJECT(QCommandLinkButton)

   GUI_CS_PROPERTY_READ(description, description)
   GUI_CS_PROPERTY_WRITE(description, setDescription)
   GUI_CS_PROPERTY_READ(flat, isFlat)
   GUI_CS_PROPERTY_WRITE(flat, setFlat)
   GUI_CS_PROPERTY_DESIGNABLE(flat, false)

 public:
   explicit QCommandLinkButton(QWidget *parent = 0);
   explicit QCommandLinkButton(const QString &text, QWidget *parent = 0);
   QCommandLinkButton(const QString &text, const QString &description, QWidget *parent = 0);
   QString description() const;
   void setDescription(const QString &description);

 protected:
   QSize sizeHint() const;
   int heightForWidth(int) const;
   QSize minimumSizeHint() const;
   bool event(QEvent *e);
   void paintEvent(QPaintEvent *);

 private:
   Q_DISABLE_COPY(QCommandLinkButton)
   Q_DECLARE_PRIVATE(QCommandLinkButton)
};

QT_END_NAMESPACE

#endif // QCOMMANDLINKBUTTON
