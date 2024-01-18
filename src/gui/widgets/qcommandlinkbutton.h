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

#ifndef QCOMMANDLINKBUTTON_H
#define QCOMMANDLINKBUTTON_H

#include <qpushbutton.h>

class QCommandLinkButtonPrivate;

class Q_GUI_EXPORT QCommandLinkButton: public QPushButton
{
   GUI_CS_OBJECT(QCommandLinkButton)

   GUI_CS_PROPERTY_READ(description, description)
   GUI_CS_PROPERTY_WRITE(description, setDescription)
   GUI_CS_PROPERTY_READ(flat, isFlat)
   GUI_CS_PROPERTY_WRITE(flat, setFlat)
   GUI_CS_PROPERTY_DESIGNABLE(flat, false)

 public:
   explicit QCommandLinkButton(QWidget *parent = nullptr);
   explicit QCommandLinkButton(const QString &text, QWidget *parent = nullptr);
   QCommandLinkButton(const QString &text, const QString &description, QWidget *parent = nullptr);

   QCommandLinkButton(const QCommandLinkButton &) = delete;
   QCommandLinkButton &operator=(const QCommandLinkButton &) = delete;

   ~QCommandLinkButton();

   QString description() const;
   void setDescription(const QString &description);

 protected:
   QSize sizeHint() const override;
   int heightForWidth(int width) const override;
   QSize minimumSizeHint() const override;
   bool event(QEvent *event) override;
   void paintEvent(QPaintEvent *event) override;

 private:
   Q_DECLARE_PRIVATE(QCommandLinkButton)
};

#endif
