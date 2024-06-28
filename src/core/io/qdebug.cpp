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

#include <qdebug.h>

#include <qmetaobject.h>

#include <qtools_p.h>

QDebug::~QDebug()
{
   if (! --m_stream->ref) {
      if (m_stream->m_addSpace && m_stream->buffer.endsWith(' ')) {
         m_stream->buffer.chop(1);
      }

      if (m_stream->message_output) {
         qt_message_output(m_stream->type, m_stream->buffer);
      }

      delete m_stream;
   }
}

void QDebug::putString(QStringView str)
{
   m_stream->ts << str;
}

void QDebug::putByteArray(const QByteArray &str)
{
   m_stream->ts << str;
}

QDebug &QDebug::resetFormat()
{
   m_stream->ts.reset();

   m_stream->m_addSpace = true;
   m_stream->m_flags = 0;

   m_stream->setVerbosity(Stream::defaultVerbosity);

   return *this;
}

class QDebugStateSaverPrivate
{
 public:
   QDebugStateSaverPrivate(QDebug &debug)
      : m_dbg(debug), m_spaces(debug.autoInsertSpaces()), m_streamParams(debug.m_stream->ts.getParams())
   {
      m_flags     = m_dbg.m_stream->m_flags;
      m_verbosity = m_dbg.m_stream->m_verbosity;
   }

   void restoreState() {
      const bool currentSpaces = m_dbg.autoInsertSpaces();

      if (currentSpaces && ! m_spaces) {
         if (m_dbg.m_stream->buffer.endsWith(' ')) {
            m_dbg.m_stream->buffer.chop(1);
         }
      }

      m_dbg.setAutoInsertSpaces(m_spaces);
      m_dbg.m_stream->ts.setParams(m_streamParams);

      m_dbg.m_stream->m_flags     = m_flags;
      m_dbg.m_stream->m_verbosity = m_verbosity;

      if (! currentSpaces && m_spaces) {
         m_dbg.m_stream->ts << ' ';
      }
   }

   QDebug &m_dbg;
   const bool m_spaces;
   int m_flags;
   int m_verbosity;

   const QTextStream::Params m_streamParams;
};

QDebugStateSaver::QDebugStateSaver(QDebug &debug)
   : d_ptr(new QDebugStateSaverPrivate(debug))
{
}

QDebugStateSaver::~QDebugStateSaver()
{
   d_ptr->restoreState();
}
