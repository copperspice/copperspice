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

#include <playlistfileparser_p.h>

#include <qdebug.h>
#include <qfileinfo.h>
#include <qmediametadata.h>
#include <qnetwork_reply.h>
#include <qnetwork_request.h>

#include <qmediaobject_p.h>

class ParserBase : public QObject
{
   MULTI_CS_OBJECT(ParserBase)

 public:
   ParserBase(QObject *parent)
      : QObject(parent)
   {
   }

   virtual void parseLine(int lineIndex, const QString &line, const QUrl &root) = 0;

   MULTI_CS_SIGNAL_1(Public, void newItem(const QVariant &content))
   MULTI_CS_SIGNAL_2(newItem, content)

   MULTI_CS_SIGNAL_1(Public, void finished())
   MULTI_CS_SIGNAL_2(finished)

   MULTI_CS_SIGNAL_1(Public, void error(QPlaylistFileParser::ParserError err, const QString &errorMsg))
   MULTI_CS_SIGNAL_2(error, err, errorMsg)

 protected:
   QUrl expandToFullPath(const QUrl &root, const QString &line) {
      // On Linux, backslashes are not converted to forward slashes
      if (line.startsWith("//") || line.startsWith("\\\\")) {
         // Network share paths are not resolved
         return QUrl::fromLocalFile(line);
      }

      QUrl url(line);
      if (url.scheme().isEmpty()) {
         // Resolve it relative to root
         if (root.isLocalFile()) {
            return QUrl::fromUserInput(line, root.adjusted(QUrl::RemoveFilename).toLocalFile(), QUrl::AssumeLocalFile);
         } else {
            return root.resolved(url);
         }

      } else if (url.scheme().length() == 1) {
         // Assume it's a drive letter for a Windows path
         url = QUrl::fromLocalFile(line);
      }

      return url;
   }
};


/*
    Extended M3U directives

    #EXTM3U - header - must be first line of file
    #EXTINF - extra info - length (seconds), title
    #EXTINF - extra info - length (seconds), artist '-' title

    Example

    #EXTM3U
    #EXTINF:123, Sample artist - Sample title
    C:\Documents and Settings\I\My Music\Sample.mp3
    #EXTINF:321,Example Artist - Example title
    C:\Documents and Settings\I\My Music\Greatest Hits\Example.ogg
*/

class M3UParser : public ParserBase
{
 public:
   M3UParser(QObject *parent)
      : ParserBase(parent), m_extendedFormat(false)
   { }

   void parseLine(int lineIndex, const QString &line, const QUrl &root) override {
      if (line[0] == '#' ) {
         if (m_extendedFormat) {
            if (line.startsWith("#EXTINF:")) {
               m_extraInfo.clear();
               int artistStart = line.indexOf(",", 8);
               bool ok = false;

               int length = line.mid(8, artistStart < 8 ? -1 : artistStart - 8).trimmed().toInteger<int>(&ok);

               if (ok && length > 0) {
                  //convert from second to milisecond
                  m_extraInfo[QMediaMetaData::Duration] = QVariant(length * 1000);
               }

               if (artistStart > 0) {
                  int titleStart = getSplitIndex(line, artistStart);

                  if (titleStart > artistStart) {
                     m_extraInfo[QMediaMetaData::Author] = line.mid(artistStart + 1,
                           titleStart - artistStart - 1).trimmed().replace("--", "-");

                     m_extraInfo[QMediaMetaData::Title] = line.mid(titleStart + 1).trimmed().replace("--", "-");

                  } else {
                     m_extraInfo[QMediaMetaData::Title] = line.mid(artistStart + 1).trimmed().replace("--", "-");
                  }
               }

            }
         } else if (lineIndex == 0 && line.startsWith("#EXTM3U")) {
            m_extendedFormat = true;
         }

      } else {
         m_extraInfo["url"] = expandToFullPath(root, line);
         emit newItem(QVariant(m_extraInfo));
         m_extraInfo.clear();
      }
   }

   int getSplitIndex(const QString &line, int startPos) {
      if (startPos < 0) {
         startPos = 0;
      }

      QString::const_iterator iter = line.constBegin() + startPos;

      for (int i = startPos; i < line.length(); ++i, ++iter) {
         if (*iter == '-') {
            if (i == line.length() - 1) {
               return i;
            }

            ++i;
            ++iter;

            if (*iter != '-') {
               return i - 1;
            }
         }
      }

      return -1;
   }

 private:
   bool m_extendedFormat;
   QVariantMap m_extraInfo;
};


/*
The format is essentially that of an INI file structured as follows:

Header

    * [playlist] : This tag indicates that it is a Playlist File

Track Entry
Assuming track entry #X

    * FileX : Variable defining location of stream.
    * TitleX : Defines track title.
    * LengthX : Length in seconds of track. Value of -1 indicates indefinite.

Footer

    * NumberOfEntries : This variable indicates the number of tracks.
    * Version : Playlist version. Currently only a value of 2 is valid.

[playlist]

File1=Alternative\everclear - SMFTA.mp3

Title1=Everclear - So Much For The Afterglow

Length1=233

File2=http://www.site.com:8000/listen.pls

Title2=My Cool Stream

Length5=-1

NumberOfEntries=2

Version=2
*/


class PLSParser : public ParserBase
{
   MULTI_CS_OBJECT(PLSParser)

 public:
   PLSParser(QObject *parent)
      : ParserBase(parent)
   {
   }

   void parseLine(int, const QString &line, const QUrl &root) override {
      // ignore everything but 'File' entries, since that's the only thing we care about.
      if (! line.startsWith("File")) {
         return;
      }

      QString value = getValue(line);
      if (value.isEmpty()) {
         return;
      }

      emit newItem(expandToFullPath(root, value));
   }

   QString getValue(const QString &line) {
      int start = line.indexOf('=');

      if (start < 0) {
         return QString();
      }

      return line.mid(start + 1).trimmed();
   }
};

class QPlaylistFileParserPrivate
{
   Q_DECLARE_PUBLIC(QPlaylistFileParser)

 public:
   QPlaylistFileParserPrivate()
      : m_source(nullptr), m_scanIndex(0), m_utf8(false), m_lineIndex(-1),
        m_type(QPlaylistFileParser::UNKNOWN), m_currentParser(nullptr)
   {
   }

   void _q_handleData();
   void _q_handleError();
   void _q_handleParserError(QPlaylistFileParser::ParserError err, const QString &errorMsg);
   void _q_handleParserFinished();

   QNetworkReply *m_source;
   QByteArray m_buffer;
   int  m_scanIndex;
   QUrl m_root;
   bool m_utf8;
   int m_lineIndex;
   QPlaylistFileParser::FileType m_type;
   ParserBase *m_currentParser;
   QNetworkAccessManager m_mgr;

 private:
   QPlaylistFileParser *q_ptr;
   void processLine(int startIndex, int length);
};

#define LINE_LIMIT  4096
#define READ_LIMIT  64

void QPlaylistFileParserPrivate::processLine(int startIndex, int length)
{
   Q_Q(QPlaylistFileParser);
   ++m_lineIndex;

   if (! m_currentParser) {
      Q_ASSERT(! m_currentParser);

      QString mimeType = m_source->header(QNetworkRequest::ContentTypeHeader).toString();
      m_type = QPlaylistFileParser::findPlaylistType(m_root.toString(), mimeType, m_buffer);

      switch (m_type) {
         case QPlaylistFileParser::UNKNOWN:
            emit q->error(QPlaylistFileParser::FormatError,
               QPlaylistFileParser::tr("%1 playlist type is unknown").formatArg(m_root.toString()));
            q->stop();
            return;

         case QPlaylistFileParser::M3U:
            m_currentParser = new M3UParser(q);
            break;

         case QPlaylistFileParser::M3U8:
            m_currentParser = new M3UParser(q);
            m_utf8 = true;
            break;

         case QPlaylistFileParser::PLS:
            m_currentParser = new PLSParser(q);
            break;
      }

      Q_ASSERT(m_currentParser);

      QObject::connect(m_currentParser, &ParserBase::newItem,  q, &QPlaylistFileParser::newItem);
      QObject::connect(m_currentParser, &ParserBase::finished, q, &QPlaylistFileParser::_q_handleParserFinished);
      QObject::connect(m_currentParser, &ParserBase::error,    q, &QPlaylistFileParser::_q_handleParserError);
   }

   QString line;

   if (m_utf8) {
      line = QString::fromUtf8(m_buffer.constData() + startIndex, length).trimmed();
   } else {
      line = QString::fromLatin1(m_buffer.constData() + startIndex, length).trimmed();
   }

   if (line.isEmpty()) {
      return;
   }

   Q_ASSERT(m_currentParser);
   m_currentParser->parseLine(m_lineIndex, line, m_root);
}

void QPlaylistFileParserPrivate::_q_handleData()
{
   Q_Q(QPlaylistFileParser);

   while (m_source->bytesAvailable()) {
      int expectedBytes = qMin(READ_LIMIT, int(qMin(m_source->bytesAvailable(),
                  qint64(LINE_LIMIT - m_buffer.size()))));

      m_buffer.push_back(m_source->read(expectedBytes));
      int processedBytes = 0;

      while (m_scanIndex < m_buffer.length()) {
         char s = m_buffer[m_scanIndex];

         if (s == '\r' || s == '\n') {
            int l = m_scanIndex - processedBytes;
            if (l > 0) {
               processLine(processedBytes, l);
            }

            processedBytes = m_scanIndex + 1;

            if (! m_source) {
               // error happened, exit parsing
               return;
            }
         }
         m_scanIndex++;
      }

      if (m_buffer.length() - processedBytes >= LINE_LIMIT) {
         qWarning() << "error parsing playlist[" << m_root.toString() << "] with line content >= 4096 bytes.";

         emit q->error(QPlaylistFileParser::FormatError, QPlaylistFileParser::tr("invalid line in playlist file"));
         q->stop();

         return;
      }

      if (m_source->isFinished() && ! m_source->bytesAvailable()) {
         // last line
         processLine(processedBytes, -1);
         break;
      }

      Q_ASSERT(m_buffer.length() == m_scanIndex);
      if (processedBytes == 0) {
         continue;
      }

      int copyLength = m_buffer.length() - processedBytes;
      if (copyLength > 0) {
         Q_ASSERT(copyLength <= READ_LIMIT);
         m_buffer = m_buffer.right(copyLength);
      } else {
         m_buffer.clear();
      }

      m_scanIndex = 0;
   }

   if (m_source->isFinished()) {
      _q_handleParserFinished();
   }
}

void QPlaylistFileParserPrivate::_q_handleError()
{
   Q_Q(QPlaylistFileParser);
   emit q->error(QPlaylistFileParser::NetworkError, m_source->errorString());
   q->stop();
}

void QPlaylistFileParserPrivate::_q_handleParserError(QPlaylistFileParser::ParserError err, const QString &errorMsg)
{
   Q_Q(QPlaylistFileParser);
   emit q->error(err, errorMsg);
}

void QPlaylistFileParserPrivate::_q_handleParserFinished()
{
   Q_Q(QPlaylistFileParser);
   bool isParserValid = (m_currentParser != nullptr);

   if (!isParserValid) {
      emit q->error(QPlaylistFileParser::FormatNotSupportedError, QPlaylistFileParser::tr("Empty file provided"));
   }

   q->stop();

   if (isParserValid) {
      emit q->finished();
   }
}

QPlaylistFileParser::QPlaylistFileParser(QObject *parent)
   : QObject(parent), d_ptr(new QPlaylistFileParserPrivate)
{
   d_ptr->q_ptr = this;
}

QPlaylistFileParser::FileType QPlaylistFileParser::findPlaylistType(const QString &uri, const QString &mime,
                  const QByteArray &data)
{
   if (data.isEmpty()) {
      return UNKNOWN;
   }

   FileType uriType = UNKNOWN;
   QString suffix = QFileInfo(uri).suffix().toLower();

   if (suffix == "m3u") {
      uriType = M3U;
   }

   else if (suffix == "m3u8") {
      uriType = M3U8;
   }

   else if (suffix == "pls") {
      uriType = PLS;
   }

   FileType mimeType = UNKNOWN;
   if (mime == "text/uri-list" || mime == "audio/x-mpegurl" || mime == "audio/mpegurl") {
      mimeType = QPlaylistFileParser::M3U;
   }

   else if (mime == "application/x-mpegURL" || mime == "application/vnd.apple.mpegurl") {
      mimeType = QPlaylistFileParser::M3U8;
   }

   else if (mime == "audio/x-scpls") {
      mimeType = QPlaylistFileParser::PLS;
   }

   FileType bufferType = UNKNOWN;

   if (data.startsWith("#EXTM3U")) {
      bufferType = M3U;

   } else if (data.startsWith("[playlist]")) {
      bufferType = PLS;

   } else {
      // make sure every line is a text string
      bool printok = true;

      for (char ch : data) {
         if (! std::isprint(ch)) {
            printok = false;
            break;
         }
      }

      if (printok) {
         bufferType = M3U;
      }
   }

   if (bufferType == M3U && (uriType == M3U8 || mimeType == M3U8)) {
      bufferType = M3U8;
   }

   if (bufferType != UNKNOWN) {
      return bufferType;
   }

   if (uriType != UNKNOWN) {
      return uriType;
   }

   if (mimeType != UNKNOWN) {
      return mimeType;
   }

   return UNKNOWN;
}

void QPlaylistFileParser::start(const QNetworkRequest &request, bool utf8)
{
   Q_D(QPlaylistFileParser);
   stop();

   d->m_type = UNKNOWN;
   d->m_utf8 = utf8;
   d->m_root = request.url();

   if (d->m_root.isLocalFile() && ! QFile::exists(d->m_root.toLocalFile())) {
      emit error(NetworkError, tr("%1 does not exist").formatArg(d->m_root.toString()));
      return;
   }

   d->m_source = d->m_mgr.get(request);

   connect(d->m_source, &QNetworkReply::readyRead, this, &QPlaylistFileParser::_q_handleData);
   connect(d->m_source, &QNetworkReply::finished,  this, &QPlaylistFileParser::_q_handleData);
   connect(d->m_source, &QNetworkReply::error,     this, &QPlaylistFileParser::_q_handleError);

   d->_q_handleData();
}

void QPlaylistFileParser::stop()
{
   Q_D(QPlaylistFileParser);

   if (d->m_currentParser) {
      disconnect(d->m_currentParser, &ParserBase::newItem,  this, &QPlaylistFileParser::newItem);
      disconnect(d->m_currentParser, &ParserBase::finished, this, &QPlaylistFileParser::_q_handleParserFinished);
      disconnect(d->m_currentParser, &ParserBase::error,    this, &QPlaylistFileParser::_q_handleParserError);

      d->m_currentParser->deleteLater();
      d->m_currentParser = nullptr;
   }

   d->m_buffer.clear();
   d->m_scanIndex = 0;
   d->m_lineIndex = -1;

   if (d->m_source) {
      disconnect(d->m_source, &QNetworkReply::readyRead, this, &QPlaylistFileParser::_q_handleData);
      disconnect(d->m_source, &QNetworkReply::finished,  this, &QPlaylistFileParser::_q_handleData);
      disconnect(d->m_source, &QNetworkReply::error,     this, &QPlaylistFileParser::_q_handleError);

      d->m_source->deleteLater();
      d->m_source = nullptr;
   }
}

void QPlaylistFileParser::_q_handleData()
{
   Q_D(QPlaylistFileParser);
   d->_q_handleData();
}

void QPlaylistFileParser::_q_handleError()
{
   Q_D(QPlaylistFileParser);
   d->_q_handleError();
}

void QPlaylistFileParser::_q_handleParserError(QPlaylistFileParser::ParserError err, const QString &errorMsg)
{
   Q_D(QPlaylistFileParser);
   d->_q_handleParserError(err, errorMsg);
}

void QPlaylistFileParser::_q_handleParserFinished()
{
   Q_D(QPlaylistFileParser);
   d->_q_handleParserFinished();
}

