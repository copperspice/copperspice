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

#ifndef QTESTLITEMIME_H
#define QTESTLITEMIME_H

#include <qdnd_p.h>
#include <QtGui/QClipboard>
#include <qxlibintegration.h>
#include <qxlibclipboard.h>

class QXlibMime : public QInternalMimeData {
    Q_OBJECT
public:
    QXlibMime();
    ~QXlibMime();

    static QList<Atom> mimeAtomsForFormat(Display *display, const QString &format);
    static QString mimeAtomToString(Display *display, Atom a);
    static bool mimeDataForAtom(Display *display, Atom a, QMimeData *mimeData, QByteArray *data, Atom *atomFormat, int *dataFormat);
    static QStringList mimeFormatsForAtom(Display *display, Atom a);
    static Atom mimeStringToAtom(Display *display, const QString &mimeType);
    static QVariant mimeConvertToFormat(Display *display, Atom a, const QByteArray &data, const QString &format, QVariant::Type requestedType, const QByteArray &encoding);
    static Atom mimeAtomForFormat(Display *display, const QString &format, QVariant::Type requestedType, const QList<Atom> &atoms, QByteArray *requestedEncoding);
};

#endif // QTESTLITEMIME_H
