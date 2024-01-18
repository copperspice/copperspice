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

#ifndef QDECLARATIVESYSTEMPALETTE_P_H
#define QDECLARATIVESYSTEMPALETTE_P_H

#include <qdeclarative.h>
#include <QtCore/qobject.h>
#include <QPalette>

QT_BEGIN_NAMESPACE

class QDeclarativeSystemPalettePrivate;

class QDeclarativeSystemPalette : public QObject
{
   DECL_CS_OBJECT(QDeclarativeSystemPalette)

   CS_ENUM(ColorGroup)
   Q_DECLARE_PRIVATE(QDeclarativeSystemPalette)

   DECL_CS_PROPERTY_READ(colorGroup, colorGroup)
   DECL_CS_PROPERTY_WRITE(colorGroup, setColorGroup)
   DECL_CS_PROPERTY_NOTIFY(colorGroup, paletteChanged)
   DECL_CS_PROPERTY_READ(window, window)
   DECL_CS_PROPERTY_NOTIFY(window, paletteChanged)
   DECL_CS_PROPERTY_READ(windowText, windowText)
   DECL_CS_PROPERTY_NOTIFY(windowText, paletteChanged)
   DECL_CS_PROPERTY_READ(base, base)
   DECL_CS_PROPERTY_NOTIFY(base, paletteChanged)
   DECL_CS_PROPERTY_READ(text, text)
   DECL_CS_PROPERTY_NOTIFY(text, paletteChanged)
   DECL_CS_PROPERTY_READ(alternateBase, alternateBase)
   DECL_CS_PROPERTY_NOTIFY(alternateBase, paletteChanged)
   DECL_CS_PROPERTY_READ(button, button)
   DECL_CS_PROPERTY_NOTIFY(button, paletteChanged)
   DECL_CS_PROPERTY_READ(buttonText, buttonText)
   DECL_CS_PROPERTY_NOTIFY(buttonText, paletteChanged)
   DECL_CS_PROPERTY_READ(light, light)
   DECL_CS_PROPERTY_NOTIFY(light, paletteChanged)
   DECL_CS_PROPERTY_READ(midlight, midlight)
   DECL_CS_PROPERTY_NOTIFY(midlight, paletteChanged)
   DECL_CS_PROPERTY_READ(dark, dark)
   DECL_CS_PROPERTY_NOTIFY(dark, paletteChanged)
   DECL_CS_PROPERTY_READ(mid, mid)
   DECL_CS_PROPERTY_NOTIFY(mid, paletteChanged)
   DECL_CS_PROPERTY_READ(shadow, shadow)
   DECL_CS_PROPERTY_NOTIFY(shadow, paletteChanged)
   DECL_CS_PROPERTY_READ(highlight, highlight)
   DECL_CS_PROPERTY_NOTIFY(highlight, paletteChanged)
   DECL_CS_PROPERTY_READ(highlightedText, highlightedText)
   DECL_CS_PROPERTY_NOTIFY(highlightedText, paletteChanged)

 public:
   QDeclarativeSystemPalette(QObject *parent = nullptr);
   ~QDeclarativeSystemPalette();

   enum ColorGroup { Active = QPalette::Active, Inactive = QPalette::Inactive, Disabled = QPalette::Disabled };

   QColor window() const;
   QColor windowText() const;

   QColor base() const;
   QColor text() const;
   QColor alternateBase() const;

   QColor button() const;
   QColor buttonText() const;

   QColor light() const;
   QColor midlight() const;
   QColor dark() const;
   QColor mid() const;
   QColor shadow() const;

   QColor highlight() const;
   QColor highlightedText() const;

   QDeclarativeSystemPalette::ColorGroup colorGroup() const;
   void setColorGroup(QDeclarativeSystemPalette::ColorGroup);

   DECL_CS_SIGNAL_1(Public, void paletteChanged())
   DECL_CS_SIGNAL_2(paletteChanged)

 private:
   bool eventFilter(QObject *watched, QEvent *event);
   bool event(QEvent *event);

};

QT_END_NAMESPACE

QML_DECLARE_TYPE(QDeclarativeSystemPalette)


#endif // QDECLARATIVESYSTEMPALETTE_H
