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

#ifndef PLAYLISTFILEPARSER_P_H
#define PLAYLISTFILEPARSER_P_H

#include <qnetwork_request.h>

class QPlaylistFileParserPrivate;

class Q_MULTIMEDIA_EXPORT QPlaylistFileParser : public QObject
{
   MULTI_CS_OBJECT(QPlaylistFileParser)

 public:
   enum FileType {
      UNKNOWN,
      M3U,
      M3U8, // UTF-8 version of M3U
      PLS
   };

   enum ParserError {
      NoError,
      FormatError,
      FormatNotSupportedError,
      NetworkError
   };

   QPlaylistFileParser(QObject *parent = nullptr);

   QPlaylistFileParser(const QPlaylistFileParser &) = delete;
   QPlaylistFileParser &operator=(const QPlaylistFileParser &) = delete;

   static FileType findPlaylistType(const QString &uri, const QString &mime, const QByteArray &data);

   void start(const QNetworkRequest &request, bool utf8 = false);
   void stop();

   MULTI_CS_SIGNAL_1(Public, void newItem(const QVariant &content))
   MULTI_CS_SIGNAL_2(newItem, content)
   MULTI_CS_SIGNAL_1(Public, void finished())
   MULTI_CS_SIGNAL_2(finished)
   MULTI_CS_SIGNAL_1(Public, void error(QPlaylistFileParser::ParserError err, const QString &errorMsg))
   MULTI_CS_SIGNAL_2(error, err, errorMsg)

 private:
   Q_DECLARE_PRIVATE(QPlaylistFileParser)

   QPlaylistFileParserPrivate *d_ptr;

   MULTI_CS_SLOT_1(Private, void _q_handleData())
   MULTI_CS_SLOT_2(_q_handleData)

   MULTI_CS_SLOT_1(Private, void _q_handleError())
   MULTI_CS_SLOT_2(_q_handleError)

   MULTI_CS_SLOT_1(Private, void _q_handleParserError(QPlaylistFileParser::ParserError err, const QString &errorMsg))
   MULTI_CS_SLOT_2(_q_handleParserError)

   MULTI_CS_SLOT_1(Private, void _q_handleParserFinished())
   MULTI_CS_SLOT_2(_q_handleParserFinished)
};

#endif




