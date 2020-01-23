/***********************************************************************
*
* Copyright (c) 2012-2020 Barbara Geller
* Copyright (c) 2012-2020 Ansel Sermersheim
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

#include "phrase.h"
#include "translator.h"

#include <QApplication>
#include <QFile>
#include <QFileInfo>
#include <QMessageBox>
#include <QRegExp>
#include <QTextCodec>
#include <QTextStream>
#include <QXmlAttributes>
#include <QXmlDefaultHandler>
#include <QXmlParseException>

static QString protect(const QString &str)
{
   QString p = str;
   p.replace(QLatin1Char('&'),  QLatin1String("&amp;"));
   p.replace(QLatin1Char('\"'), QLatin1String("&quot;"));
   p.replace(QLatin1Char('>'),  QLatin1String("&gt;"));
   p.replace(QLatin1Char('<'),  QLatin1String("&lt;"));
   p.replace(QLatin1Char('\''), QLatin1String("&apos;"));
   return p;
}

Phrase::Phrase()
   : shrtc(-1), m_phraseBook(0)
{
}

Phrase::Phrase(const QString &source, const QString &target,
               const QString &definition, int sc)
   : shrtc(sc), s(source), t(target), d(definition),
     m_phraseBook(0)
{
}

Phrase::Phrase(const QString &source, const QString &target,
               const QString &definition, PhraseBook *phraseBook)
   : shrtc(-1), s(source), t(target), d(definition),
     m_phraseBook(phraseBook)
{
}

void Phrase::setSource(const QString &ns)
{
   if (s == ns) {
      return;
   }
   s = ns;
   if (m_phraseBook) {
      m_phraseBook->phraseChanged(this);
   }
}

void Phrase::setTarget(const QString &nt)
{
   if (t == nt) {
      return;
   }
   t = nt;
   if (m_phraseBook) {
      m_phraseBook->phraseChanged(this);
   }
}

void Phrase::setDefinition(const QString &nd)
{
   if (d == nd) {
      return;
   }
   d = nd;
   if (m_phraseBook) {
      m_phraseBook->phraseChanged(this);
   }
}

bool operator==(const Phrase &p, const Phrase &q)
{
   return p.source() == q.source() && p.target() == q.target() &&
          p.definition() == q.definition() && p.phraseBook() == q.phraseBook();
}

class QphHandler : public QXmlDefaultHandler
{
 public:
   QphHandler(PhraseBook *phraseBook)
      : pb(phraseBook), ferrorCount(0) { }

   virtual bool startElement(const QString &namespaceURI,
                             const QString &localName, const QString &qName,
                             const QXmlAttributes &atts);
   virtual bool endElement(const QString &namespaceURI,
                           const QString &localName, const QString &qName);
   virtual bool characters(const QString &ch);
   virtual bool fatalError(const QXmlParseException &exception);

   QString language() const {
      return m_language;
   }
   QString sourceLanguage() const {
      return m_sourceLanguage;
   }

 private:
   PhraseBook *pb;
   QString source;
   QString target;
   QString definition;
   QString m_language;
   QString m_sourceLanguage;

   QString accum;
   int ferrorCount;
};

bool QphHandler::startElement(const QString & /* namespaceURI */,
                              const QString & /* localName */,
                              const QString &qName,
                              const QXmlAttributes &atts)
{
   if (qName == QLatin1String("QPH")) {
      m_language = atts.value(QLatin1String("language"));
      m_sourceLanguage = atts.value(QLatin1String("sourcelanguage"));
   } else if (qName == QLatin1String("phrase")) {
      source.truncate(0);
      target.truncate(0);
      definition.truncate(0);
   }
   accum.truncate(0);
   return true;
}

bool QphHandler::endElement(const QString & /* namespaceURI */,
                            const QString & /* localName */,
                            const QString &qName)
{
   if (qName == QLatin1String("source")) {
      source = accum;
   } else if (qName == QLatin1String("target")) {
      target = accum;
   } else if (qName == QLatin1String("definition")) {
      definition = accum;
   } else if (qName == QLatin1String("phrase")) {
      pb->m_phrases.append(new Phrase(source, target, definition, pb));
   }
   return true;
}

bool QphHandler::characters(const QString &ch)
{
   accum += ch;
   return true;
}

bool QphHandler::fatalError(const QXmlParseException &exception)
{
   if (ferrorCount++ == 0) {
      QString msg = PhraseBook::tr("Parse error at line %1, column %2 (%3).")
                    .arg(exception.lineNumber()).arg(exception.columnNumber())
                    .arg(exception.message());
      QMessageBox::information(0,
                               QObject::tr("Qt Linguist"), msg);
   }
   return false;
}

PhraseBook::PhraseBook() :
   m_changed(false),
   m_language(QLocale::C),
   m_sourceLanguage(QLocale::C),
   m_country(QLocale::AnyCountry),
   m_sourceCountry(QLocale::AnyCountry)
{
}

PhraseBook::~PhraseBook()
{
   qDeleteAll(m_phrases);
}

void PhraseBook::setLanguageAndCountry(QLocale::Language lang, QLocale::Country country)
{
   if (m_language == lang && m_country == country) {
      return;
   }
   m_language = lang;
   m_country = country;
   setModified(true);
}

void PhraseBook::setSourceLanguageAndCountry(QLocale::Language lang, QLocale::Country country)
{
   if (m_sourceLanguage == lang && m_sourceCountry == country) {
      return;
   }
   m_sourceLanguage = lang;
   m_sourceCountry = country;
   setModified(true);
}

bool PhraseBook::load(const QString &fileName, bool *langGuessed)
{
   QFile f(fileName);
   if (!f.open(QIODevice::ReadOnly)) {
      return false;
   }

   m_fileName = fileName;

   QXmlInputSource in(&f);
   QXmlSimpleReader reader;

   reader.setFeature(QLatin1String("http://xml.org/sax/features/namespaces"), false);
   reader.setFeature(QLatin1String("http://xml.org/sax/features/namespace-prefixes"), true);
   reader.setFeature(QLatin1String("http://copperspice.com/xml/features/report-whitespace-only-CharData"), false);

   QphHandler *hand = new QphHandler(this);
   reader.setContentHandler(hand);
   reader.setErrorHandler(hand);

   bool ok = reader.parse(in);
   reader.setContentHandler(0);
   reader.setErrorHandler(0);

   Translator::languageAndCountry(hand->language(), &m_language, &m_country);
   *langGuessed = false;
   if (m_language == QLocale::C) {
      QLocale sys;
      m_language = sys.language();
      m_country = sys.country();
      *langGuessed = true;
   }

   QString lang = hand->sourceLanguage();
   if (lang.isEmpty()) {
      m_sourceLanguage = QLocale::C;
      m_sourceCountry = QLocale::AnyCountry;
   } else {
      Translator::languageAndCountry(lang, &m_sourceLanguage, &m_sourceCountry);
   }

   delete hand;
   f.close();
   if (!ok) {
      qDeleteAll(m_phrases);
      m_phrases.clear();
   } else {
      emit listChanged();
   }

   return ok;
}

bool PhraseBook::save(const QString &fileName)
{
   QFile f(fileName);
   if (!f.open(QIODevice::WriteOnly)) {
      return false;
   }

   m_fileName = fileName;

   QTextStream t(&f);
   t.setCodec( QTextCodec::codecForName("UTF-8") );

   t << "<!DOCTYPE QPH>\n<QPH";
   if (sourceLanguage() != QLocale::C)
      t << " sourcelanguage=\""
        << Translator::makeLanguageCode(sourceLanguage(), sourceCountry()) << '"';

   if (language() != QLocale::C) {
      t << " language=\"" << Translator::makeLanguageCode(language(), country()) << '"';
   }
   t << ">\n";
   for (Phrase * p : m_phrases) {
      t << "<phrase>\n";
      t << "    <source>" << protect( p->source() ) << "</source>\n";
      t << "    <target>" << protect( p->target() ) << "</target>\n";

      if (!p->definition().isEmpty())
         t << "    <definition>" << protect( p->definition() )
           << "</definition>\n";
      t << "</phrase>\n";
   }

   t << "</QPH>\n";
   f.close();
   setModified(false);

   return true;
}

void PhraseBook::append(Phrase *phrase)
{
   m_phrases.append(phrase);
   phrase->setPhraseBook(this);
   setModified(true);
   emit listChanged();
}

void PhraseBook::remove(Phrase *phrase)
{
   m_phrases.removeOne(phrase);
   phrase->setPhraseBook(0);
   setModified(true);
   emit listChanged();
}

void PhraseBook::setModified(bool modified)
{
   if (m_changed != modified) {
      emit modifiedChanged(modified);
      m_changed = modified;
   }
}

void PhraseBook::phraseChanged(Phrase *p)
{
   setModified(true);
}

QString PhraseBook::friendlyPhraseBookName() const
{
   if (!m_fileName.isEmpty()) {
      return QFileInfo(m_fileName).fileName();
   }

   return QString();
}


