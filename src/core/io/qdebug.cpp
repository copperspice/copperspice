/***********************************************************************
*
* Copyright (c) 2012-2023 Barbara Geller
* Copyright (c) 2012-2023 Ansel Sermersheim
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

#ifdef QT_NO_DEBUG
#undef QT_NO_DEBUG
#endif

#ifdef qDebug
#undef qDebug
#endif

#include <qdebug.h>
#include <qmetaobject.h>

#include <qtools_p.h>

QDebug::~QDebug()
{
    if (! --stream->ref) {
        if (stream->space && stream->buffer.endsWith(' ')) {
            stream->buffer.chop(1);
        }

        if (stream->message_output) {
            qt_message_output(stream->type, stream->buffer);
        }

        delete stream;
    }
}

void QDebug::putString(QStringView str)
{
   stream->ts << str;
}

void QDebug::putByteArray(const QByteArray &str)
{
   stream->ts << str;
}

QDebug &QDebug::resetFormat()
{
    stream->ts.reset();

    stream->space = true;
    stream->m_flags = 0;

    stream->setVerbosity(Stream::defaultVerbosity);

    return *this;
}

class QDebugStateSaverPrivate
{
public:
   QDebugStateSaverPrivate(QDebug &dbg)
        : m_dbg(dbg), m_spaces(dbg.autoInsertSpaces()), m_streamParams(dbg.stream->ts.getParams())
   {
      m_flags     = m_dbg.stream->m_flags;
      m_verbosity = m_dbg.stream->m_verbosity;
   }

   void restoreState()
   {
      const bool currentSpaces = m_dbg.autoInsertSpaces();

      if (currentSpaces && ! m_spaces)
         if (m_dbg.stream->buffer.endsWith(' ')) {
             m_dbg.stream->buffer.chop(1);
         }

      m_dbg.setAutoInsertSpaces(m_spaces);
      m_dbg.stream->ts.setParams(m_streamParams);

      m_dbg.stream->m_flags     = m_flags;
      m_dbg.stream->m_verbosity = m_verbosity;

      if (! currentSpaces && m_spaces) {
         m_dbg.stream->ts << ' ';
      }
   }

   QDebug &m_dbg;
   const bool m_spaces;
   int m_flags;
   int m_verbosity;

   const QTextStream::Params m_streamParams;
};

QDebugStateSaver::QDebugStateSaver(QDebug &dbg)
    : d_ptr(new QDebugStateSaverPrivate(dbg))
{
}

QDebugStateSaver::~QDebugStateSaver()
{
   d_ptr->restoreState();
}
