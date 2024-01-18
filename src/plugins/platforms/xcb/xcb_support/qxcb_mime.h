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

#ifndef QXCB_MIME_H
#define QXCB_MIME_H

#include <qdnd_p.h>
#include <qclipboard.h>
#include <qxcb_integration.h>
#include <qxcb_connection.h>

#if ! (defined(QT_NO_DRAGANDDROP) && defined(QT_NO_CLIPBOARD))

class QXcbMime : public QInternalMimeData
{
   CS_OBJECT(QXcbMime)

 public:
   QXcbMime();
   ~QXcbMime();

   static QVector<xcb_atom_t> mimeAtomsForFormat(QXcbConnection *connection, const QString &format);
   static QString mimeAtomToString(QXcbConnection *connection, xcb_atom_t a);

   static bool mimeDataForAtom(QXcbConnection *connection, xcb_atom_t a, QMimeData *mimeData, QByteArray *data,
      xcb_atom_t *atomFormat, int *dataFormat);

   static QVariant mimeConvertToFormat(QXcbConnection *connection, xcb_atom_t a, const QByteArray &data, const QString &format,
      QVariant::Type requestedType, const QByteArray &encoding);

   static xcb_atom_t mimeAtomForFormat(QXcbConnection *connection, const QString &format, QVariant::Type requestedType,
      const QVector<xcb_atom_t> &atoms, QByteArray *requestedEncoding);
};

#endif

#endif // QXCBMIME_H
