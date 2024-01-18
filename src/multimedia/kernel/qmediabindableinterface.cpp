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

#include <qmediabindableinterface.h>

/*!
    \class QMediaBindableInterface
    \inmodule QtMultimedia

    \ingroup multimedia
    \ingroup multimedia_core


    \brief The QMediaBindableInterface class is the base class for objects extending media objects functionality.

    \sa
*/

/*!
    Destroys a media helper object.
*/

QMediaBindableInterface::~QMediaBindableInterface()
{
}

/*!
    \fn QMediaBindableInterface::mediaObject() const;

    Return the currently attached media object.
*/


/*!
    \fn QMediaBindableInterface::setMediaObject(QMediaObject *object);

    Attaches to the media \a object.
    Returns true if attached successfully, otherwise returns false.
*/

