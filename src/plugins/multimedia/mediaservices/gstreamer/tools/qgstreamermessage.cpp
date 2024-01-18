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

#include <qgstreamermessage_p.h>

#include <gst/gst.h>

QGstreamerMessage::QGstreamerMessage()
   : m_message(nullptr)
{
}

QGstreamerMessage::QGstreamerMessage(GstMessage *message)
   : m_message(message)
{
   gst_message_ref(m_message);
}

QGstreamerMessage::QGstreamerMessage(QGstreamerMessage const &m):
   m_message(m.m_message)
{
   gst_message_ref(m_message);
}

QGstreamerMessage::~QGstreamerMessage()
{
   if (m_message != nullptr) {
      gst_message_unref(m_message);
   }
}

GstMessage *QGstreamerMessage::rawMessage() const
{
   return m_message;
}

QGstreamerMessage &QGstreamerMessage::operator=(QGstreamerMessage const &rhs)
{
   if (rhs.m_message != m_message) {
      if (rhs.m_message != nullptr) {
         gst_message_ref(rhs.m_message);
      }

      if (m_message != nullptr) {
         gst_message_unref(m_message);
      }

      m_message = rhs.m_message;
   }

   return *this;
}
