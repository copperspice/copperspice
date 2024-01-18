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

#ifndef QSVGWIDGET_H
#define QSVGWIDGET_H

#include <qglobal.h>

#ifndef QT_NO_SVGWIDGET

#include <qwidget.h>

class QSvgWidgetPrivate;
class QPaintEvent;
class QSvgRenderer;

class Q_SVG_EXPORT QSvgWidget : public QWidget
{
   SVG_CS_OBJECT(QSvgWidget)

 public:
   QSvgWidget(QWidget *parent = nullptr);
   QSvgWidget(const QString &file, QWidget *parent = nullptr);

   QSvgWidget(const QSvgWidget &) = delete;
   QSvgWidget &operator=(const QSvgWidget &) = delete;

   ~QSvgWidget();

   QSvgRenderer *renderer() const;
   QSize sizeHint() const override;

   SVG_CS_SLOT_1(Public, void load(const QString &file))
   SVG_CS_SLOT_OVERLOAD(load, (const QString &))

   SVG_CS_SLOT_1(Public, void load(const QByteArray &contents))
   SVG_CS_SLOT_OVERLOAD(load, (const QByteArray &))

 protected:
   void paintEvent(QPaintEvent *event) override;

 private:
   Q_DECLARE_PRIVATE(QSvgWidget)
};

#endif // QT_NO_SVGWIDGET
#endif
