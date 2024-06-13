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

#include <translator.h>

#include <qbytearray.h>
#include <qdebug.h>
#include <qtextcodec.h>
#include <qtextstream.h>
#include <qxmlstreamreader.h>

#include <algorithm>

static const QString text_byte                = "byte";
static const QString text_catalog             = "catalog";
static const QString text_comment             = "comment";
static const QString text_context             = "context";
static const QString text_defaultcodec        = "defaultcodec";
static const QString text_dependencies        = "dependencies";
static const QString text_dependency          = "dependency";
static const QString text_extracomment        = "extracomment";
static const QString text_filename            = "filename";
static const QString text_id                  = "id";
static const QString text_language            = "language";
static const QString text_lengthvariant       = "lengthvariant";
static const QString text_line                = "line";
static const QString text_location            = "location";
static const QString text_message             = "message";
static const QString text_name                = "name";
static const QString text_numerus             = "numerus";
static const QString text_numerusform         = "numerusform";
static const QString text_obsolete            = "obsolete";
static const QString text_oldcomment          = "oldcomment";
static const QString text_oldsource           = "oldsource";
static const QString text_source              = "source";
static const QString text_sourcelanguage      = "sourcelanguage";
static const QString text_translation         = "translation";
static const QString text_translatorcomment   = "translatorcomment";
static const QString text_TS                  = "TS";
static const QString text_type                = "type";
static const QString text_unfinished          = "unfinished";
static const QString text_userdata            = "userdata";
static const QString text_variants            = "variants";
static const QString text_value               = "value";
static const QString text_vanished            = "vanished";
static const QString text_yes                 = "yes";

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

   if (hasError() && error() == CustomError) {
      // raised by readContents
      return;
   }

   const QString loc = QString("in %1 at Line: %2 Column: %3").formatArg(m_cd.m_sourceFileName).formatArg(lineNumber()).formatArg(columnNumber());

   switch (tokenType()) {
      case NoToken:
      case Invalid:
      default:
         raiseError(QString("Parse error %1: \n%2").formatArg(loc).formatArg(errorString()));
         break;

      case StartElement:
         raiseError(QString("Unexpected tag <%1> \n%2").formatArg(name().toString()).formatArg(loc));
         break;

      case Characters: {
         QString tok = text().toString();

         if (tok.length() > 30) {
            tok = tok.left(30) + "[...]";
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

   int n = value.toInteger<uint>(nullptr, base);

   return (n != 0) ? QString(QChar(n)) : QString();
}

QString TSReader::readContents()
{
   QString result;

   while (! atEnd()) {
      readNext();

      if (isEndElement()) {
         break;

      } else if (isCharacters()) {
         result += text();

      } else if (elementStarts(text_byte)) {
         // <byte value="...">
         result += byteValue(attributes().value(text_value).toString());
         readNext();

         if (! isEndElement()) {
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
   if (attributes().value(text_variants) == text_yes) {
      QString result;

      while (! atEnd()) {
         readNext();

         if (isEndElement()) {
            break;

         } else if (isWhiteSpace()) {
            // ignore these, just whitespace

         } else if (elementStarts(text_lengthvariant)) {
            if (! result.isEmpty()) {
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
   static const QString text_extrans("extra-");

   while (! atEnd()) {
      readNext();

      if (isStartDocument()) {
         // ignore state

      } else if (isEndDocument()) {
         // ignore state

      } else if (isDTD()) {
         // ignore state

      } else if (elementStarts(text_TS)) {
         QHash<QString, int> currentLine;
         QString currentFile;
         bool maybeRelative = false, maybeAbsolute = false;

         QXmlStreamAttributes atts = attributes();

         translator.setLanguageCode(atts.value(text_language).toString());
         translator.setSourceLanguageCode(atts.value(text_sourcelanguage).toString());

         while (! atEnd()) {
            readNext();

            if (isEndElement()) {
               break;

            } else if (isWhiteSpace()) {
               // ignore these, just whitespace

            } else if (elementStarts(text_defaultcodec)) {

               readElementText();
               m_cd.appendError("Warning: ignoring <defaultcodec> element");

            } else if (isStartElement() && name().toString().startsWith(text_extrans)) {

               QString tag = name().toString();
               translator.setExtra(tag.mid(6), readContents());

            } else if (elementStarts(text_dependencies)) {
               /*
               * <dependencies>
               *   <dependency catalog="qtsystems_no"/>
               *   <dependency catalog="qtbase_no"/>
               * </dependencies>
               **/

               QStringList dependencies;

               while (! atEnd()) {
                  readNext();

                  if (isEndElement()) {
                     // </dependencies> found, finish local loop
                     break;

                  } else if (elementStarts(text_dependency)) {
                     // <dependency>
                     QXmlStreamAttributes atts = attributes();
                     dependencies.append(atts.value(text_catalog).toString());

                     while (! atEnd()) {
                        readNext();

                        if (isEndElement()) {
                           // </dependency> found, finish local loop
                           break;
                        }
                     }
                  }
               }

               translator.setDependencies(dependencies);

            } else if (elementStarts(text_context)) {
               // <context>
               QString context;

               while (! atEnd()) {
                  readNext();

                  if (isEndElement()) {
                     // </context> found, finish local loop
                     break;

                  } else if (isWhiteSpace()) {
                     // ignore these, just whitespace

                  } else if (elementStarts(text_name)) {
                     // <name>
                     context = readElementText();

                  } else if (elementStarts(text_message)) {
                     // <message>
                     QList<TranslatorMessage::Reference> refs;
                     QString currentMsgFile = currentFile;

                     TranslatorMessage msg;
                     msg.setId(attributes().value(text_id).toString());
                     msg.setContext(context);
                     msg.setType(TranslatorMessage::Type::Finished);
                     msg.setPlural(attributes().value(text_numerus) == text_yes);

                     while (! atEnd()) {
                        readNext();

                        if (isEndElement()) {
                           // </message> found, finish local loop
                           msg.setReferences(refs);
                           translator.append(msg);
                           break;

                        } else if (isWhiteSpace()) {
                           // ignore these, just whitespace

                        } else if (elementStarts(text_source)) {
                           // <source>...</source>
                           msg.setSourceText(readContents());

                        } else if (elementStarts(text_oldsource)) {
                           // <oldsource>...</oldsource>
                           msg.setOldSourceText(readContents());

                        } else if (elementStarts(text_oldcomment)) {
                           // <oldcomment>...</oldcomment>
                           msg.setOldComment(readContents());

                        } else if (elementStarts(text_extracomment)) {
                           // <extracomment>...</extracomment>
                           msg.setExtraComment(readContents());

                        } else if (elementStarts(text_translatorcomment)) {
                           // <translatorcomment>...</translatorcomment>
                           msg.setTranslatorComment(readContents());

                        } else if (elementStarts(text_location)) {
                           // <location/>
                           maybeAbsolute = true;
                           QXmlStreamAttributes atts = attributes();

                           QString fileName = atts.value(text_filename).toString();

                           if (fileName.isEmpty()) {
                              fileName = currentMsgFile;
                              maybeRelative = true;

                           } else {
                              if (refs.isEmpty()) {
                                 currentFile = fileName;
                              }

                              currentMsgFile = fileName;
                           }

                           const QString lin = atts.value(text_line).toString();

                           if (lin.isEmpty()) {
                              refs.append(TranslatorMessage::Reference(fileName, -1));

                           } else {
                              bool bOK;
                              int lineNo = lin.toInteger<int>(&bOK);

                              if (bOK) {
                                 if (lin.startsWith('+') || lin.startsWith('-')) {
                                    lineNo = (currentLine[fileName] += lineNo);
                                    maybeRelative = true;
                                 }
                                 refs.append(TranslatorMessage::Reference(fileName, lineNo));
                              }
                           }
                           readContents();

                        } else if (elementStarts(text_comment)) {
                           // <comment>...</comment>
                           msg.setComment(readContents());

                        } else if (elementStarts(text_userdata)) {
                           // <userdata>...</userdata>
                           msg.setUserData(readContents());

                        } else if (elementStarts(text_translation)) {
                           // <translation>
                           QXmlStreamAttributes atts = attributes();
                           QStringView type = atts.value(text_type);

                           if (type == text_unfinished) {
                              msg.setType(TranslatorMessage::Type::Unfinished);

                           } else if (type == text_vanished) {
                              msg.setType(TranslatorMessage::Type::Vanished);

                           } else if (type == text_obsolete) {
                              msg.setType(TranslatorMessage::Type::Obsolete);

                           }

                           if (msg.isPlural()) {
                              QStringList translations;

                              while (! atEnd()) {
                                 readNext();

                                 if (isEndElement()) {
                                    break;

                                 } else if (isWhiteSpace()) {
                                    // ignore these, just whitespace

                                 } else if (elementStarts(text_numerusform)) {
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


                        } else if (isStartElement() && name().toString().startsWith(text_extrans)) {
                           // <extra-...>
                           QString tag = name().toString();
                           msg.setExtra(tag.mid(6), readContents());

                        } else {
                           handleError();
                        }
                     }

                  } else {
                     handleError();
                  }
               }

            } else {
               handleError();
            }

            translator.setLocationsType(maybeRelative ? Translator::RelativeLocations :
                                        maybeAbsolute ? Translator::AbsoluteLocations : Translator::NoLocations);
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
            result += "&quot;";
            break;

         case '&':
            result += "&amp;";
            break;

         case '>':
            result += "&gt;";
            break;

         case '<':
            result += "&lt;";
            break;

         case '\'':
            result += "&apos;";
            break;

         default:
            if (c < 0x20 && c != '\r' && c != '\n' && c != '\t') {
               result += numericEntity(c.unicode());

            } else {
               result += c;
            }
      }
   }

   return result;
}

static void writeExtras(QTextStream &t, const char *indent, const QHash<QString, QString> &extras,
            const QRegularExpression &drops)
{
   QStringList list;

   for (auto iter = extras.cbegin(); iter != extras.cend(); ++iter) {
      QRegularExpressionMatch match = drops.match(iter.key());

      if (! match.hasMatch()) {
         list.append("<extra-" + iter.key() + '>' + protect(iter.value()) + "</extra-" + iter.key() + ">");
      }
   }
   list.sort();

   for (const QString &out : list) {
      t << indent << out << endl;
   }
}

static void writeVariants(QTextStream &t, const char *indent, const QString &input)
{
   int offset;

   if ((offset = input.indexOf(QChar(Translator::BinaryVariantSeparator))) >= 0) {
      t << " variants=\"yes\">";
      int start = 0;

      while (true) {
         t << "\n    " << indent << "<lengthvariant>"
                       << protect(input.mid(start, offset - start))
                       << "</lengthvariant>";

         if (offset == input.length()) {
            break;
         }

         start = offset + 1;
         offset = input.indexOf(QChar(Translator::BinaryVariantSeparator), start);

         if (offset < 0) {
            offset = input.length();
         }
      }

      t << "\n" << indent;

   } else {
      t << ">" << protect(input);
   }
}

bool saveTS(const Translator &translator, QIODevice &dev, ConversionData &cd)
{
   bool result = true;
   QTextStream t(&dev);

   t.setCodec(QTextCodec::codecForName("UTF-8"));
   t << "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n<!DOCTYPE TS>\n";
   t << "<TS version=\"2.1\"";

   QString languageCode = translator.languageCode();

   if (! languageCode.isEmpty() && languageCode != "C") {
      t << " language=\"" << languageCode << "\"";
   }

   languageCode = translator.sourceLanguageCode();

   if (! languageCode.isEmpty() && languageCode != "C") {
      t << " sourcelanguage=\"" << languageCode << "\"";
   }
   t << ">\n";

   QStringList deps = translator.dependencies();

   if (!deps.isEmpty()) {
      t << "<dependencies>\n";

      for (const QString &dep : deps) {
         t << "<dependency catalog=\"" << dep << "\"/>\n";
      }

      t << "</dependencies>\n";
   }

   QRegularExpression drops(cd.dropTags().join("|"));

   writeExtras(t, "    ", translator.extras(), drops);

   QHash<QString, QList<TranslatorMessage> > messageOrder;
   QList<QString> contextOrder;

   for (const TranslatorMessage &msg : translator.messages()) {

      if ((msg.type() == TranslatorMessage::Type::Obsolete || msg.type() == TranslatorMessage::Type::Vanished) &&
            msg.translation().isEmpty()) {
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

   for (const QString &context : contextOrder) {

      t << "<context>\n"
        "    <name>";

      t << protect(context)
        << "</name>\n";

      for (const TranslatorMessage &msg : messageOrder[context]) {

         t << "    <message";

         if (! msg.id().isEmpty()) {
            t << " id=\"" << msg.id() << "\"";
         }

         if (msg.isPlural()) {
            t << " numerus=\"yes\"";
         }

         t << ">\n";

         if (translator.locationsType() != Translator::NoLocations) {
            QString cfile = currentFile;
            bool first = true;

            for (const TranslatorMessage::Reference &ref : msg.allReferences()) {
               QString fn = cd.m_targetDir.relativeFilePath(ref.fileName()).replace(QChar('\\'), QChar('/'));

               int ln = ref.lineNumber();
               QString ld;

               if (translator.locationsType() == Translator::RelativeLocations) {
                  if (ln != -1) {
                     int dlt = ln - currentLine[fn];

                     if (dlt >= 0) {
                        ld.append('+');
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

               if (! fn.isEmpty()) {
                  t << " filename=\"" << fn << "\"";
               }

               if (! ld.isEmpty()) {
                  t << " line=\"" << ld << "\"";
               }

               t << "/>\n";
            }
         }

         t << "        <source>"
           << protect(msg.sourceText())
           << "</source>\n";

         if (! msg.oldSourceText().isEmpty()) {
            t << "        <oldsource>" << protect(msg.oldSourceText()) << "</oldsource>\n";
         }

         if (! msg.comment().isEmpty()) {
            t << "        <comment>"
              << protect(msg.comment())
              << "</comment>\n";
         }

         if (! msg.oldComment().isEmpty()) {
            t << "        <oldcomment>" << protect(msg.oldComment()) << "</oldcomment>\n";
         }

         if (! msg.extraComment().isEmpty()) {
            t << "        <extracomment>" << protect(msg.extraComment())
              << "</extracomment>\n";
         }

         if (! msg.translatorComment().isEmpty()) {
            t << "        <translatorcomment>" << protect(msg.translatorComment())
              << "</translatorcomment>\n";
         }

         t << "        <translation";

         if (msg.type() == TranslatorMessage::Type::Unfinished) {
            t << " type=\"unfinished\"";

         } else if (msg.type() == TranslatorMessage::Type::Vanished) {
            t << " type=\"vanished\"";

         } else if (msg.type() == TranslatorMessage::Type::Obsolete) {
            t << " type=\"obsolete\"";
         }

         if (msg.isPlural()) {
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


         writeExtras(t, "        ", msg.extras(), drops);


         if (! msg.userData().isEmpty()) {
            t << "        <userdata>" << msg.userData() << "</userdata>\n";
         }

         t << "    </message>\n";
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

int initTS()
{
   Translator::FileFormat format;

   format.extension   = "ts";
   format.fileType    = Translator::FileFormat::TranslationSource;
   format.priority    = 0;

   format.description = "Translate source text";
   format.loader      = &loadTS;
   format.saver       = &saveTS;

   Translator::registerFileFormat(format);

   return 1;
}

Q_CONSTRUCTOR_FUNCTION(initTS)
