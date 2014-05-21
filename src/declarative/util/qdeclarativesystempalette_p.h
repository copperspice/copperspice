/***********************************************************************
*
* Copyright (c) 2012-2014 Barbara Geller
* Copyright (c) 2012-2014 Ansel Sermersheim
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

#ifndef QDECLARATIVESYSTEMPALETTE_H
#define QDECLARATIVESYSTEMPALETTE_H

#include <qdeclarative.h>

#include <QtCore/qobject.h>
#include <QPalette>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

class QDeclarativeSystemPalettePrivate;

class QDeclarativeSystemPalette : public QObject
{
    CS_OBJECT(QDeclarativeSystemPalette)

    CS_ENUM(ColorGroup)
    Q_DECLARE_PRIVATE(QDeclarativeSystemPalette)

    CS_PROPERTY_READ(colorGroup, colorGroup)
    CS_PROPERTY_WRITE(colorGroup, setColorGroup)
    CS_PROPERTY_NOTIFY(colorGroup, paletteChanged)
    CS_PROPERTY_READ(window, window)
    CS_PROPERTY_NOTIFY(window, paletteChanged)
    CS_PROPERTY_READ(windowText, windowText)
    CS_PROPERTY_NOTIFY(windowText, paletteChanged)
    CS_PROPERTY_READ(base, base)
    CS_PROPERTY_NOTIFY(base, paletteChanged)
    CS_PROPERTY_READ(text, text)
    CS_PROPERTY_NOTIFY(text, paletteChanged)
    CS_PROPERTY_READ(alternateBase, alternateBase)
    CS_PROPERTY_NOTIFY(alternateBase, paletteChanged)
    CS_PROPERTY_READ(button, button)
    CS_PROPERTY_NOTIFY(button, paletteChanged)
    CS_PROPERTY_READ(buttonText, buttonText)
    CS_PROPERTY_NOTIFY(buttonText, paletteChanged)
    CS_PROPERTY_READ(light, light)
    CS_PROPERTY_NOTIFY(light, paletteChanged)
    CS_PROPERTY_READ(midlight, midlight)
    CS_PROPERTY_NOTIFY(midlight, paletteChanged)
    CS_PROPERTY_READ(dark, dark)
    CS_PROPERTY_NOTIFY(dark, paletteChanged)
    CS_PROPERTY_READ(mid, mid)
    CS_PROPERTY_NOTIFY(mid, paletteChanged)
    CS_PROPERTY_READ(shadow, shadow)
    CS_PROPERTY_NOTIFY(shadow, paletteChanged)
    CS_PROPERTY_READ(highlight, highlight)
    CS_PROPERTY_NOTIFY(highlight, paletteChanged)
    CS_PROPERTY_READ(highlightedText, highlightedText)
    CS_PROPERTY_NOTIFY(highlightedText, paletteChanged)

public:
    QDeclarativeSystemPalette(QObject *parent=0);
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

public:
    CS_SIGNAL_1(Public, void paletteChanged())
    CS_SIGNAL_2(paletteChanged) 

private:
    bool eventFilter(QObject *watched, QEvent *event);
    bool event(QEvent *event);

};

QT_END_NAMESPACE

QML_DECLARE_TYPE(QDeclarativeSystemPalette)

QT_END_HEADER

#endif // QDECLARATIVESYSTEMPALETTE_H
