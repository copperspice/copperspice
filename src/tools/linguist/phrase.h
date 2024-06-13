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

#ifndef PHRASE_H
#define PHRASE_H

#include <qlist.h>
#include <qlocale.h>
#include <qobject.h>
#include <qstring.h>

class PhraseBook;
class QphHandler;

class Phrase
{
 public:
   Phrase();
   Phrase(const QString &source, const QString &target, const QString &definition, int sc = -1);
   Phrase(const QString &source, const QString &target, const QString &definition, PhraseBook *phraseBook);

   QString source() const {
      return s;
   }

   void setSource(const QString &ns);

   QString target() const {
      return t;
   }

   void setTarget(const QString &nt);

   QString definition() const {
      return d;
   }

   void setDefinition (const QString &nd);

   int shortcut() const {
      return shrtc;
   }

   PhraseBook *phraseBook() const {
      return m_phraseBook;
   }

   void setPhraseBook(PhraseBook *book) {
      m_phraseBook = book;
   }

 private:
   int shrtc;
   QString s;
   QString t;
   QString d;
   PhraseBook *m_phraseBook;
};

bool operator==(const Phrase &p, const Phrase &q);

inline bool operator!=(const Phrase &p, const Phrase &q)
{
   return !(p == q);
}

class PhraseBook : public QObject
{
   CS_OBJECT(PhraseBook)

 public:
   PhraseBook();
   ~PhraseBook();
   bool load(const QString &fileName, bool *langGuessed);
   bool save(const QString &fileName);

   QList<Phrase *> phrases() const {
      return m_phrases;
   }

   void append(Phrase *phrase);
   void remove(Phrase *phrase);

   QString fileName() const {
      return m_fileName;
   }

   QString friendlyPhraseBookName() const;
   bool isModified() const {
      return m_changed;
   }

   void setLanguageAndCountry(QLocale::Language lang, QLocale::Country country);
   QLocale::Language language() const {
      return m_language;
   }

   QLocale::Country country() const {
      return m_country;
   }

   void setSourceLanguageAndCountry(QLocale::Language lang, QLocale::Country country);
   QLocale::Language sourceLanguage() const {
      return m_sourceLanguage;
   }

   QLocale::Country sourceCountry() const {
      return m_sourceCountry;
   }

   CS_SIGNAL_1(Public, void modifiedChanged(bool changed))
   CS_SIGNAL_2(modifiedChanged,changed)

   CS_SIGNAL_1(Public, void listChanged())
   CS_SIGNAL_2(listChanged)

 private:
   // Prevent copying
   PhraseBook(const PhraseBook &);
   PhraseBook &operator=(const PhraseBook &);

   void setModified(bool modified);
   void phraseChanged(Phrase *phrase);

   QList<Phrase *> m_phrases;
   QString m_fileName;
   bool m_changed;

   QLocale::Language m_language;
   QLocale::Language m_sourceLanguage;
   QLocale::Country m_country;
   QLocale::Country m_sourceCountry;

   friend class QphHandler;
   friend class Phrase;
};

#endif
