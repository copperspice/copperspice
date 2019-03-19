/***********************************************************************
*
* Copyright (c) 2012-2019 Barbara Geller
* Copyright (c) 2012-2019 Ansel Sermersheim
*
* Copyright (C) 2015 The Qt Company Ltd.
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

#include <algorithm>

#include <translator.h>

#include <QtCore/QByteArray>
#include <QtCore/QDebug>
#include <QtCore/QTextCodec>
#include <QtCore/QTextStream>

#include <QtXml/QXmlStreamReader>
#include <QtXml/QXmlStreamAttribute>

#define STRINGIFY_INTERNAL(x) #x
#define STRINGIFY(x) STRINGIFY_INTERNAL(x)
#define STRING(s) static QString str##s(QLatin1String(STRINGIFY(s)))

QT_BEGIN_NAMESPACE

/*
 * The encodings are a total mess.
 * A Translator has a codecForTr(). Each message's text will be passed to tr()
 * in that encoding or as UTF-8 to trUtf8() if it is flagged as such.
 * For ts 2.0, the file content is always uniformly in UTF-8. The file stores
 * the codecForTr default and marks deviating messages accordingly.
 * For ts 1.1, the file content is in mixed encoding. Each message is encoded
 * the way it will be passed to tr() (with 8-bit characters encoded as numeric
 * entities) or trUtf8(). The file stores the encoding and codecForTr in one
 * attribute, for both the default and each deviating message.
 */


QDebug &operator<<(QDebug &d, const QXmlStreamAttribute &attr)
{
   return d << "[" << attr.name().toString() << "," << attr.value().toString() << "]";
}


class TSReader : public QXmlStreamReader
{
 public:
   TSReader(QIODevice &dev, ConversionData &cd)
      : QXmlStreamReader(&dev), m_cd(cd) {
   }

   // the "real thing"
   bool read(Translator &translator);

 private:
   bool elementStarts(const QString &str) const {
      return isStartElement() && name() == str;
   }

   bool isWhiteSpace() const {
      return isCharacters() && text().toString().trimmed().isEmpty();
   }

   // needed to expand <byte ... />
   QString readContents();

   // needed to join <lengthvariant>s
   QString readTransContents();

   void handleError();

   ConversionData &m_cd;
};

void TSReader::handleError()
{
   if (isComment()) {
      return;
   }

   if (hasError() && error() == CustomError) { // raised by readContents
      return;
   }

   const QString loc = QString("at %3:%1:%2").formatArg(lineNumber()).formatArg(columnNumber()).formatArg(m_cd.m_sourceFileName);

   switch (tokenType()) {
      case NoToken:    // Cannot happen
      default:          // likewise
      case Invalid:
         raiseError(QString("Parse error %1: %2").formatArg(loc).formatArg(errorString()));
         break;

      case StartElement:
         raiseError(QString("Unexpected tag <%1> %2").formatArg(name().toString()).formatArg(loc));
         break;

      case Characters: {
         QString tok = text().toString();
         if (tok.length() > 30) {
            tok = tok.left(30) + QLatin1String("[...]");
         }
         raiseError(QString("Unexpected characters '%1' %2").formatArg(tok).formatArg(loc));
      }
      break;

      case EntityReference:
         raiseError(QString("Unexpected entity '&%1;' %2").formatArg(name().toString()).formatArg(loc));
         break;

      case ProcessingInstruction:
         raiseError(QString("Unexpected processing instruction %1").formatArg(loc));
         break;
   }
}

static QString byteValue(QString value)
{
   int base = 10;

   if (value.startsWith("x")) {
      base = 16;
      value.remove(0, 1);
   }

   int n = value.toInteger<uint>(0, base);
   return (n != 0) ? QString(QChar(n)) : QString();
}

QString TSReader::readContents()
{
   STRING(byte);
   STRING(value);

   QString result;
   while (!atEnd()) {
      readNext();

      if (isEndElement()) {
         break;

      } else if (isCharacters()) {
         result += text();

      } else if (elementStarts(strbyte)) {
         // <byte value="...">
         result += byteValue(attributes().value(strvalue).toString());
         readNext();
         if (!isEndElement()) {
            handleError();
            break;
         }

      } else {
         handleError();
         break;
      }
   }

   return result;
}

QString TSReader::readTransContents()
{
   STRING(lengthvariant);
   STRING(variants);
   STRING(yes);

   if (attributes().value(strvariants) == stryes) {
      QString result;
      while (!atEnd()) {
         readNext();
         if (isEndElement()) {
            break;
         } else if (isWhiteSpace()) {
            // ignore these, just whitespace
         } else if (elementStarts(strlengthvariant)) {
            if (!result.isEmpty()) {
               result += QChar(Translator::BinaryVariantSeparator);
            }
            result += readContents();
         } else {
            handleError();
            break;
         }
      }
      return result;
   } else {
      return readContents();
   }
}

bool TSReader::read(Translator &translator)
{
   STRING(both);
   STRING(byte);
   STRING(comment);
   STRING(context);
   STRING(defaultcodec);
   STRING(encoding);
   STRING(extracomment);
   STRING(filename);
   STRING(id);
   STRING(language);
   STRING(line);
   STRING(location);
   STRING(message);
   STRING(name);
   STRING(numerus);
   STRING(numerusform);
   STRING(obsolete);
   STRING(oldcomment);
   STRING(oldsource);
   STRING(source);
   STRING(sourcelanguage);
   STRING(translation);
   STRING(translatorcomment);
   STRING(true);
   STRING(TS);
   STRING(type);
   STRING(unfinished);
   STRING(userdata);
   STRING(utf8);
   STRING(value);
   //STRING(version);
   STRING(yes);

   static const QString strextrans(QLatin1String("extra-"));
   static const QString strUtf8(QLatin1String("UTF-8"));

   while (!atEnd()) {
      readNext();
      if (isStartDocument()) {
         // <!DOCTYPE TS>
         //qDebug() << attributes();
      } else if (isEndDocument()) {
         // <!DOCTYPE TS>
         //qDebug() << attributes();
      } else if (isDTD()) {
         // <!DOCTYPE TS>
         //qDebug() << tokenString();
      } else if (elementStarts(strTS)) {
         // <TS>
         //qDebug() << "TS " << attributes();
         QHash<QString, int> currentLine;
         QString currentFile;
         bool maybeRelative = false, maybeAbsolute = false;

         QXmlStreamAttributes atts = attributes();
         //QString version = atts.value(strversion).toString();
         translator.setLanguageCode(atts.value(strlanguage).toString());
         translator.setSourceLanguageCode(atts.value(strsourcelanguage).toString());
         while (!atEnd()) {
            readNext();
            if (isEndElement()) {
               // </TS> found, finish local loop
               break;
            } else if (isWhiteSpace()) {
               // ignore these, just whitespace
            } else if (elementStarts(strdefaultcodec)) {
               // <defaultcodec>
               const QString &codec = readElementText();
               if (!codec.isEmpty()) {
                  translator.setCodecName(codec.toLatin1());
               }
               // </defaultcodec>
            } else if (isStartElement()
                       && name().toString().startsWith(strextrans)) {
               // <extra-...>
               QString tag = name().toString();
               translator.setExtra(tag.mid(6), readContents());
               // </extra-...>
            } else if (elementStarts(strcontext)) {
               // <context>
               QString context;
               while (!atEnd()) {
                  readNext();
                  if (isEndElement()) {
                     // </context> found, finish local loop
                     break;
                  } else if (isWhiteSpace()) {
                     // ignore these, just whitespace
                  } else if (elementStarts(strname)) {
                     // <name>
                     context = readElementText();
                     // </name>
                  } else if (elementStarts(strmessage)) {
                     // <message>
                     TranslatorMessage::References refs;
                     QString currentMsgFile = currentFile;

                     TranslatorMessage msg;
                     msg.setId(attributes().value(strid).toString());
                     msg.setContext(context);
                     msg.setType(TranslatorMessage::Finished);
                     msg.setPlural(attributes().value(strnumerus) == stryes);
                     const QStringView &utf8Attr = attributes().value(strutf8);
                     msg.setNonUtf8(utf8Attr == strboth);
                     msg.setUtf8(msg.isNonUtf8() || utf8Attr == strtrue
                                 ||  attributes().value(strencoding) == strUtf8);
                     while (!atEnd()) {
                        readNext();
                        if (isEndElement()) {
                           // </message> found, finish local loop
                           msg.setReferences(refs);
                           translator.append(msg);
                           break;
                        } else if (isWhiteSpace()) {
                           // ignore these, just whitespace
                        } else if (elementStarts(strsource)) {
                           // <source>...</source>
                           msg.setSourceText(readContents());
                        } else if (elementStarts(stroldsource)) {
                           // <oldsource>...</oldsource>
                           msg.setOldSourceText(readContents());
                        } else if (elementStarts(stroldcomment)) {
                           // <oldcomment>...</oldcomment>
                           msg.setOldComment(readContents());
                        } else if (elementStarts(strextracomment)) {
                           // <extracomment>...</extracomment>
                           msg.setExtraComment(readContents());
                        } else if (elementStarts(strtranslatorcomment)) {
                           // <translatorcomment>...</translatorcomment>
                           msg.setTranslatorComment(readContents());

                        } else if (elementStarts(strlocation)) {
                           // <location/>
                           maybeAbsolute = true;
                           QXmlStreamAttributes atts = attributes();
                           QString fileName = atts.value(strfilename).toString();

                           if (fileName.isEmpty()) {
                              fileName = currentMsgFile;
                              maybeRelative = true;
                           } else {
                              if (refs.isEmpty()) {
                                 currentFile = fileName;
                              }
                              currentMsgFile = fileName;
                           }

                           const QString lin = atts.value(strline).toString();

                           if (lin.isEmpty()) {
                              refs.append(TranslatorMessage::Reference(fileName, -1));
                           } else {
                              bool bOK;
                              int lineNo = lin.toInteger<int>(&bOK);

                              if (bOK) {
                                 if (lin.startsWith(QLatin1Char('+')) || lin.startsWith(QLatin1Char('-'))) {
                                    lineNo = (currentLine[fileName] += lineNo);
                                    maybeRelative = true;
                                 }
                                 refs.append(TranslatorMessage::Reference(fileName, lineNo));
                              }
                           }
                           readContents();
                        } else if (elementStarts(strcomment)) {
                           // <comment>...</comment>
                           msg.setComment(readContents());
                        } else if (elementStarts(struserdata)) {
                           // <userdata>...</userdata>
                           msg.setUserData(readContents());
                        } else if (elementStarts(strtranslation)) {
                           // <translation>
                           QXmlStreamAttributes atts = attributes();
                           QStringView type = atts.value(strtype);
                           if (type == strunfinished) {
                              msg.setType(TranslatorMessage::Unfinished);
                           } else if (type == strobsolete) {
                              msg.setType(TranslatorMessage::Obsolete);
                           }
                           if (msg.isPlural()) {
                              QStringList translations;
                              while (!atEnd()) {
                                 readNext();
                                 if (isEndElement()) {
                                    break;
                                 } else if (isWhiteSpace()) {
                                    // ignore these, just whitespace
                                 } else if (elementStarts(strnumerusform)) {
                                    translations.append(readTransContents());
                                 } else {
                                    handleError();
                                    break;
                                 }
                              }
                              msg.setTranslations(translations);
                           } else {
                              msg.setTranslation(readTransContents());
                           }
                           // </translation>
                        } else if (isStartElement()
                                   && name().toString().startsWith(strextrans)) {
                           // <extra-...>
                           QString tag = name().toString();
                           msg.setExtra(tag.mid(6), readContents());
                           // </extra-...>
                        } else {
                           handleError();
                        }
                     }
                     // </message>
                  } else {
                     handleError();
                  }
               }

               // </context>
            } else {
               handleError();
            }

            translator.setLocationsType(maybeRelative ? Translator::RelativeLocations :
                                        maybeAbsolute ? Translator::AbsoluteLocations :
                                        Translator::NoLocations);
         } // </TS>
      } else {
         handleError();
      }
   }

   if (hasError()) {
      m_cd.appendError(errorString());
      return false;
   }

   return true;
}

static QString numericEntity(int ch)
{
   return (ch <= 0x20 ? QString("<byte value=\"x%1\"/>") : QString("&#x%1;")).formatArg(ch, 0, 16);
}

static QString protect(const QString &str)
{
   QString result;

   for (QChar c : str) {

      switch (c.unicode()) {
         case '\"':
            result += QLatin1String("&quot;");
            break;

         case '&':
            result += QLatin1String("&amp;");
            break;

         case '>':
            result += QLatin1String("&gt;");
            break;

         case '<':
            result += QLatin1String("&lt;");
            break;

         case '\'':
            result += QLatin1String("&apos;");
            break;

         default:
            if (c < 0x20 && c != '\r' && c != '\n' && c != '\t') {
               result += numericEntity(c.unicode());

            } else {
               // this also covers surrogates
               result += c;
            }
      }
   }

   return result;
}

static QString evilBytes(const QString &str, bool isUtf8, int format, const QByteArray &codecName)
{
   if (isUtf8) {
      return protect(str);
   }

   if (format == 20) {
      return protect(str);
   }

   if (codecName == "UTF-8") {
      return protect(str);
   }

   QTextCodec *codec = QTextCodec::codecForName(codecName);
   if (! codec) {
      return protect(str);
   }

   QString t = QString::fromLatin1(codec->fromUnicode(protect(str)).data());
   int len = (int) t.length();

   QString result;
   // FIXME: Factor is sensible only for latin scripts, probably.

   for (int k = 0; k < len; k++) {
      if (t[k].unicode() >= 0x7f) {
         result += numericEntity(t[k].unicode());
      } else {
         result += t[k];
      }
   }

   return result;
}

static void writeExtras(QTextStream &t, const char *indent,
                        const TranslatorMessage::ExtraData &extras, const QRegularExpression &drops)
{
   for (Translator::ExtraData::const_iterator it = extras.begin(); it != extras.end(); ++it) {
      QRegularExpressionMatch match = drops.match(it.key());

      if (! match.hasMatch()) {
         t << indent << "<extra-" << it.key() << '>'
           << protect(it.value())
           << "</extra-" << it.key() << ">\n";
      }
   }
}

static void writeVariants(QTextStream &t, const char *indent, const QString &input)
{
   int offset;

   if ((offset = input.indexOf(QChar(Translator::BinaryVariantSeparator))) >= 0) {
      t << " variants=\"yes\">";
      int start = 0;
      forever {
         t << "\n    " << indent << "<lengthvariant>"
         << protect(input.mid(start, offset - start))
         << "</lengthvariant>";
         if (offset == input.length())
         {
            break;
         }
         start = offset + 1;
         offset = input.indexOf(QChar(Translator::BinaryVariantSeparator), start);
         if (offset < 0)
         {
            offset = input.length();
         }
      }
      t << "\n" << indent;
   } else {
      t << ">" << protect(input);
   }
}

bool saveTS(const Translator &translator, QIODevice &dev, ConversionData &cd, int format)
{
   bool result = true;
   QTextStream t(&dev);

   t.setCodec(QTextCodec::codecForName("UTF-8"));

   bool trIsUtf8   = (translator.codecName() == "UTF-8");
   bool fileIsUtf8 = (format == 20 || trIsUtf8);

   // The xml prolog allows processors to easily detect the correct encoding
   t << "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n<!DOCTYPE TS>\n";

   if (format == 11) {
      t << "<TS version=\"1.1\"";
   } else {
      t << "<TS version=\"2.0\"";
   }

   QString languageCode = translator.languageCode();
   if (!languageCode.isEmpty() && languageCode != QLatin1String("C")) {
      t << " language=\"" << languageCode << "\"";
   }
   if (format == 20) {
      languageCode = translator.sourceLanguageCode();
      if (!languageCode.isEmpty() && languageCode != QLatin1String("C")) {
         t << " sourcelanguage=\"" << languageCode << "\"";
      }
   }
   t << ">\n";

   QByteArray codecName = translator.codecName();
   if (codecName != "ISO-8859-1") {
      t << "<defaultcodec>" << codecName << "</defaultcodec>\n";
   }

   QRegularExpression drops(cd.dropTags().join("|"));

   if (format == 20) {
      writeExtras(t, "    ", translator.extras(), drops);
   }

   QHash<QString, QList<TranslatorMessage> > messageOrder;
   QList<QString> contextOrder;

   foreach (const TranslatorMessage & msg, translator.messages()) {
      // no need for such noise
      if (msg.type() == TranslatorMessage::Obsolete && msg.translation().isEmpty()) {
         continue;
      }

      QList<TranslatorMessage> &context = messageOrder[msg.context()];
      if (context.isEmpty()) {
         contextOrder.append(msg.context());
      }
      context.append(msg);
   }
   if (cd.sortContexts()) {
      std::sort(contextOrder.begin(), contextOrder.end());
   }

   QHash<QString, int> currentLine;
   QString currentFile;
   foreach (const QString & context, contextOrder) {
      const TranslatorMessage &firstMsg = messageOrder[context].first();
      t << "<context" << ((!fileIsUtf8 && firstMsg.isUtf8()) ? " encoding=\"UTF-8\"" : "") << ">\n";

      t << "    <name>"
        << evilBytes(context, firstMsg.isUtf8() || fileIsUtf8, format, codecName)
        << "</name>\n";
      foreach (const TranslatorMessage & msg, messageOrder[context]) {
         //msg.dump();

         bool isUtf8 = msg.isUtf8();
         bool second = false;
         forever {

            t << "    <message";
            if (!msg.id().isEmpty())
            {
               t << " id=\"" << msg.id() << "\"";
            }
            if (!trIsUtf8)
            {
               if (format == 11) {
                  if (isUtf8) {
                     t << " encoding=\"UTF-8\"";
                  }
               } else {
                  if (msg.isUtf8()) {
                     if (msg.isNonUtf8()) {
                        t << " utf8=\"both\"";
                     } else {
                        t << " utf8=\"true\"";
                     }
                  }
               }
            }
            if (msg.isPlural())
            {
               t << " numerus=\"yes\"";
            }
            t << ">\n";
            if (translator.locationsType() != Translator::NoLocations)
            {
               QString cfile = currentFile;
               bool first = true;
               foreach (const TranslatorMessage::Reference & ref, msg.allReferences()) {
                  QString fn = cd.m_targetDir.relativeFilePath(ref.fileName())
                               .replace(QLatin1Char('\\'), QLatin1Char('/'));
                  int ln = ref.lineNumber();
                  QString ld;
                  if (translator.locationsType() == Translator::RelativeLocations) {
                     if (ln != -1) {
                        int dlt = ln - currentLine[fn];
                        if (dlt >= 0) {
                           ld.append(QLatin1Char('+'));
                        }
                        ld.append(QString::number(dlt));
                        currentLine[fn] = ln;
                     }

                     if (fn != cfile) {
                        if (first) {
                           currentFile = fn;
                        }
                        cfile = fn;
                     } else {
                        fn.clear();
                     }
                     first = false;
                  } else {
                     if (ln != -1) {
                        ld = QString::number(ln);
                     }
                  }
                  t << "        <location";
                  if (!fn.isEmpty()) {
                     t << " filename=\"" << fn << "\"";
                  }
                  if (!ld.isEmpty()) {
                     t << " line=\"" << ld << "\"";
                  }
                  t << "/>\n";
               }
            }

            t << "        <source>"
              << evilBytes(msg.sourceText(), isUtf8, format, codecName)
              << "</source>\n";

            if (format != 11 && !msg.oldSourceText().isEmpty())
            {
               t << "        <oldsource>" << protect(msg.oldSourceText()) << "</oldsource>\n";
            }

            if (!msg.comment().isEmpty())
            {
               t << "        <comment>"
                 << evilBytes(msg.comment(), isUtf8, format, codecName)
                 << "</comment>\n";
            }

            if (format != 11)
            {

               if (!msg.oldComment().isEmpty()) {
                  t << "        <oldcomment>" << protect(msg.oldComment()) << "</oldcomment>\n";
               }

               if (!msg.extraComment().isEmpty())
                  t << "        <extracomment>" << protect(msg.extraComment())
                    << "</extracomment>\n";

               if (!msg.translatorComment().isEmpty())
                  t << "        <translatorcomment>" << protect(msg.translatorComment())
                    << "</translatorcomment>\n";

            }

            t << "        <translation";
            if (msg.type() == TranslatorMessage::Unfinished)
            {
               t << " type=\"unfinished\"";
            } else if (msg.type() == TranslatorMessage::Obsolete)
            {
               t << " type=\"obsolete\"";
            }
            if (msg.isPlural())
            {
               t << ">";
               const QStringList &translns = msg.translations();
               for (int j = 0; j < translns.count(); ++j) {
                  t << "\n            <numerusform";
                  writeVariants(t, "            ", translns[j]);
                  t << "</numerusform>";
               }
               t << "\n        ";
            } else {
               writeVariants(t, "        ", msg.translation());
            }
            t << "</translation>\n";

            if (format != 11)
            {
               writeExtras(t, "        ", msg.extras(), drops);
            }

            if (!msg.userData().isEmpty())
            {
               t << "        <userdata>" << msg.userData() << "</userdata>\n";
            }
            t << "    </message>\n";

            if (format != 11 || second || !msg.isUtf8() || !msg.isNonUtf8())
            {
               break;
            }
            isUtf8 = false;
            second = true;
         }
      }
      t << "</context>\n";
   }

   t << "</TS>\n";
   return result;
}

bool loadTS(Translator &translator, QIODevice &dev, ConversionData &cd)
{
   TSReader reader(dev, cd);
   return reader.read(translator);
}

bool saveTS11(const Translator &translator, QIODevice &dev, ConversionData &cd)
{
   return saveTS(translator, dev, cd, 11);
}

bool saveTS20(const Translator &translator, QIODevice &dev, ConversionData &cd)
{
   return saveTS(translator, dev, cd, 20);
}

int initTS()
{
   Translator::FileFormat format;

   format.extension = QLatin1String("ts11");
   format.fileType = Translator::FileFormat::TranslationSource;
   format.priority = -1;
   format.description = QObject::tr("Qt translation sources (format 1.1)");
   format.loader = &loadTS;
   format.saver = &saveTS11;
   Translator::registerFileFormat(format);

   format.extension = QLatin1String("ts20");
   format.fileType = Translator::FileFormat::TranslationSource;
   format.priority = -1;
   format.description = QObject::tr("Qt translation sources (format 2.0)");
   format.loader = &loadTS;
   format.saver = &saveTS20;
   Translator::registerFileFormat(format);

   // "ts" is always the latest. right now it's ts20.
   format.extension = QLatin1String("ts");
   format.fileType = Translator::FileFormat::TranslationSource;
   format.priority = 0;
   format.description = QObject::tr("Qt translation sources (latest format)");
   format.loader = &loadTS;
   format.saver = &saveTS20;
   Translator::registerFileFormat(format);

   return 1;
}

Q_CONSTRUCTOR_FUNCTION(initTS)

QT_END_NAMESPACE
