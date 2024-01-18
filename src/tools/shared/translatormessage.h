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

#ifndef TRANSLATORMESSAGE_H
#define TRANSLATORMESSAGE_H

#include <qstring.h>
#include <qstringlist.h>
#include <qhash.h>

class TranslatorMessage
{
 public:
   enum class Type {
      Unfinished,
      Finished,
      Vanished,
      Obsolete
   };

   enum class SaveMode {
      Everything,
      Stripped
   };

   class Reference
   {

    public:
      Reference(const QString &fname, int l)
         : m_fileName(fname), m_lineNumber(l) {
      }

      bool operator==(const Reference &other) const {
         return fileName() == other.fileName() && lineNumber() == other.lineNumber();
      }

      QString fileName() const {
         return m_fileName;
      }

      int lineNumber() const {
         return m_lineNumber;
      }

    private:
      QString m_fileName;
      int m_lineNumber;
   };

   TranslatorMessage();

   TranslatorMessage(const QString &context, const QString &sourceText, const QString &comment,
            const QString &userData, const QString &fileName, int lineNumber,
            const QStringList &translations = QStringList(),
            Type type = TranslatorMessage::Type::Unfinished, bool plural = false);

   uint hash() const;

   QString id() const {
      return m_id;
   }

   void setId(const QString &id) {
      m_id = id;
   }

   QString context() const {
      return m_context;
   }

   void setContext(const QString &context) {
      m_context = context;
   }

   QString sourceText() const {
      return m_sourcetext;
   }

   void setSourceText(const QString &sourcetext) {
      m_sourcetext = sourcetext;
   }

   QString oldSourceText() const {
      return m_oldsourcetext;
   }

   void setOldSourceText(const QString &oldsourcetext) {
      m_oldsourcetext = oldsourcetext;
   }

   QString comment() const {
      return m_comment;
   }

   void setComment(const QString &comment) {
      m_comment = comment;
   }

   QString oldComment() const {
      return m_oldcomment;
   }

   void setOldComment(const QString &oldcomment) {
      m_oldcomment = oldcomment;
   }

   QStringList translations() const {
      return m_translations;
   }

   void setTranslations(const QStringList &translations) {
      m_translations = translations;
   }

   QString translation() const {
      return m_translations.value(0);
   }

   void setTranslation(const QString &translation) {
      m_translations = QStringList(translation);
   }

   void appendTranslation(const QString &translation) {
      m_translations.append(translation);
   }

   bool isTranslated() const {
      for (const QString &trans : m_translations) {
         if (! trans.isEmpty()) {
            return true;
         }
      }

      return false;
   }

   QString fileName() const {
      return m_fileName;
   }

   void setFileName(const QString &fileName) {
      m_fileName = fileName;
   }

   int lineNumber() const {
      return m_lineNumber;
   }

   void setLineNumber(int lineNumber) {
      m_lineNumber = lineNumber;
   }

   void clearReferences();
   void setReferences(const QList<Reference> &refs);
   void addReference(const QString &fileName, int lineNumber);

   void addReference(const Reference &ref) {
      addReference(ref.fileName(), ref.lineNumber());
   }

   void addReferenceUniq(const QString &fileName, int lineNumber);

   QList<Reference> extraReferences() const {
      return m_extraRefs;
   }

   QList<Reference> allReferences() const;
   QString userData() const {
      return m_userData;
   }

   void setUserData(const QString &userData) {
      m_userData = userData;
   }

   QString extraComment() const {
      return m_extraComment;
   }

   void setExtraComment(const QString &extraComment) {
      m_extraComment = extraComment;
   }

   QString translatorComment() const {
      return m_translatorComment;
   }

   void setTranslatorComment(const QString &translatorComment) {
      m_translatorComment = translatorComment;
   }

   bool isNull() const {
      return m_sourcetext.isEmpty() && m_lineNumber == -1 && m_translations.isEmpty();
   }

   Type type() const {
      return m_type;
   }

   void setType(Type t) {
      m_type = t;
   }

   bool isPlural() const {
      return m_plural;
   }

   void setPlural(bool isplural) {
      m_plural = isplural;
   }

   // use '<fileformat>:' as prefix for file format specific members,
   // e.g. "po-msgid_plural"

   QString extra(const QString &ba) const;
   void setExtra(const QString &ba, const QString &var);
   bool hasExtra(const QString &ba) const;

   const QHash<QString, QString> &extras() const {
      return m_extra;
   }

   void setExtras(const QHash<QString, QString> &extras) {
      m_extra = extras;
   }

   void unsetExtra(const QString &key);
   void dump() const;

 private:
   QString     m_id;
   QString     m_context;
   QString     m_sourcetext;
   QString     m_oldsourcetext;
   QString     m_comment;
   QString     m_oldcomment;
   QString     m_userData;
   QString     m_extraComment;
   QString     m_translatorComment;
   QStringList m_translations;
   QString     m_fileName;
   int         m_lineNumber;

   QHash<QString, QString>   m_extra;       // PO flags, PO plurals
   QList<Reference>  m_extraRefs;

   TranslatorMessage::Type m_type;
   bool m_plural;
};

#endif
