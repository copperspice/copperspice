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

#include "translatormessage.h"

#include <qplatformdefs.h>

#ifndef QT_NO_TRANSLATION

#include <QDataStream>
#include <QDebug>

#include <stdlib.h>


QT_BEGIN_NAMESPACE

TranslatorMessage::TranslatorMessage()
   : m_lineNumber(-1), m_type(Unfinished), m_utf8(false), m_nonUtf8(false), m_plural(false)
{
}

TranslatorMessage::TranslatorMessage(const QString &context,
                                     const QString &sourceText, const QString &comment,
                                     const QString &userData,
                                     const QString &fileName, int lineNumber, const QStringList &translations,
                                     Type type, bool plural)
   : m_context(context), m_sourcetext(sourceText), m_comment(comment),
     m_userData(userData),
     m_translations(translations), m_fileName(fileName), m_lineNumber(lineNumber),
     m_type(type), m_utf8(false), m_nonUtf8(false), m_plural(plural)
{
}

void TranslatorMessage::addReference(const QString &fileName, int lineNumber)
{
   if (m_fileName.isEmpty()) {
      m_fileName = fileName;
      m_lineNumber = lineNumber;
   } else {
      m_extraRefs.append(Reference(fileName, lineNumber));
   }
}

void TranslatorMessage::addReferenceUniq(const QString &fileName, int lineNumber)
{
   if (m_fileName.isEmpty()) {
      m_fileName = fileName;
      m_lineNumber = lineNumber;
   } else {
      if (fileName == m_fileName && lineNumber == m_lineNumber) {
         return;
      }
      if (!m_extraRefs.isEmpty()) // Rather common case, so special-case it
         foreach (const Reference & ref, m_extraRefs)
         if (fileName == ref.fileName() && lineNumber == ref.lineNumber()) {
            return;
         }
      m_extraRefs.append(Reference(fileName, lineNumber));
   }
}

void TranslatorMessage::clearReferences()
{
   m_fileName.clear();
   m_lineNumber = -1;
   m_extraRefs.clear();
}

void TranslatorMessage::setReferences(const TranslatorMessage::References &refs0)
{
   if (!refs0.isEmpty()) {
      References refs = refs0;
      const Reference &ref = refs.takeFirst();
      m_fileName = ref.fileName();
      m_lineNumber = ref.lineNumber();
      m_extraRefs = refs;
   } else {
      clearReferences();
   }
}

TranslatorMessage::References TranslatorMessage::allReferences() const
{
   References refs;
   if (!m_fileName.isEmpty()) {
      refs.append(Reference(m_fileName, m_lineNumber));
      refs += m_extraRefs;
   }
   return refs;
}

static bool needs8BitHelper(const QString &ba)
{
   for (int i = ba.size(); --i >= 0; )
      if (ba.at(i).unicode() >= 0x80) {
         return true;
      }
   return false;
}

bool TranslatorMessage::needs8Bit() const
{
   //dump();
   return needs8BitHelper(m_sourcetext)
          || needs8BitHelper(m_comment)
          || needs8BitHelper(m_context);
}


bool TranslatorMessage::hasExtra(const QString &key) const
{
   return m_extra.contains(key);
}

QString TranslatorMessage::extra(const QString &key) const
{
   return m_extra[key];
}

void TranslatorMessage::setExtra(const QString &key, const QString &value)
{
   m_extra[key] = value;
}

void TranslatorMessage::unsetExtra(const QString &key)
{
   m_extra.remove(key);
}

void TranslatorMessage::dump() const
{
   qDebug()
         << "\nId                : " << m_id
         << "\nContext           : " << m_context
         << "\nSource            : " << m_sourcetext
         << "\nComment           : " << m_comment
         << "\nUserData          : " << m_userData
         << "\nExtraComment      : " << m_extraComment
         << "\nTranslatorComment : " << m_translatorComment
         << "\nTranslations      : " << m_translations
         << "\nFileName          : " << m_fileName
         << "\nLineNumber        : " << m_lineNumber
         << "\nType              : " << m_type
         << "\nPlural            : " << m_plural
         << "\nExtra             : " << m_extra;
}


QT_END_NAMESPACE

#endif // QT_NO_TRANSLATION
