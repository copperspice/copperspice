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

#include <lupdate.h>
#include <similartext.h>
#include <translator.h>

#include <qcoreapplication.h>
#include <qdebug.h>
#include <qmap.h>
#include <qstringlist.h>
#include <qtextcodec.h>
#include <qvector.h>

static bool isDigitFriendly(QChar c)
{
   return c.isPunct() || c.isSpace();
}

static int numberLength(const QString &s, int i)
{
   if (i >= s.size() || !s.at(i).isDigit()) {
      return 0;
   }

   int pos = i;

   do {
      ++i;

   } while (i < s.size()
            && (s.at(i).isDigit()
                || (isDigitFriendly(s[i])
                    && i + 1 < s.size()
                    && (s[i + 1].isDigit()
                        || (isDigitFriendly(s[i + 1])
                            && i + 2 < s.size()
                            && s[i + 2].isDigit())))));
   return i - pos;
}


/*
  Returns a version of 'key' where all numbers have been replaced by zeroes.  If
  there were none, returns "".
*/
static QString zeroKey(const QString &key)
{
   QString zeroed;
   bool metSomething = false;

   for (int i = 0; i < key.size(); ++i) {
      int len = numberLength(key, i);

      if (len > 0) {
         i += len;
         zeroed.append('0');
         metSomething = true;

      } else {
         zeroed.append(key.at(i));
      }
   }

   return metSomething ? zeroed : QString();
}

static QString translationAttempt(const QString &oldTranslation, const QString &oldSource, const QString &newSource)
{
   int p = zeroKey(oldSource).count(QChar('0'));
   QString attempt;
   QStringList oldNumbers;
   QStringList newNumbers;
   QVector<bool> met(p);
   QVector<int> matchedYet(p);
   int i, j;
   int k = 0, ell, best;
   int m, n;
   int pass;

   /*
     This algorithm is hard to follow, so we'll consider an example
     all along: oldTranslation is "XeT 3.0", oldSource is "TeX 3.0"
     and newSource is "XeT 3.1".

     First, we set up two tables: oldNumbers and newNumbers. In our
     example, oldNumber[0] is "3.0" and newNumber[0] is "3.1".
   */
   for (i = 0, j = 0; i < oldSource.size(); i++, j++) {
      m = numberLength(oldSource, i);
      n = numberLength(newSource, j);
      if (m > 0) {
         oldNumbers.append(oldSource.mid(i, m + 1));
         newNumbers.append(newSource.mid(j, n + 1));
         i += m;
         j += n;
         met[k] = false;
         matchedYet[k] = 0;
         k++;
      }
   }

   /*
     We now go over the old translation, "XeT 3.0", one letter at a
     time, looking for numbers found in oldNumbers. Whenever such a
     number is met, it is replaced with its newNumber equivalent. In
     our example, the "3.0" of "XeT 3.0" becomes "3.1".
   */
   for (i = 0; i < oldTranslation.length(); i++) {
      attempt += oldTranslation[i];
      for (k = 0; k < p; k++) {
         if (oldTranslation[i] == oldNumbers[k][matchedYet[k]]) {
            matchedYet[k]++;
         } else {
            matchedYet[k] = 0;
         }
      }

      /*
        Let's find out if the last character ended a match. We make
        two passes over the data. In the first pass, we try to
        match only numbers that weren't matched yet; if that fails,
        the second pass does the trick. This is useful in some
        suspicious cases, flagged below.
      */
      for (pass = 0; pass < 2; pass++) {
         best = p; // an impossible value
         for (k = 0; k < p; k++) {
            if ((!met[k] || pass > 0) &&
                  matchedYet[k] == oldNumbers[k].length() &&
                  numberLength(oldTranslation, i + 1 - matchedYet[k]) == matchedYet[k]) {
               // the longer the better
               if (best == p || matchedYet[k] > matchedYet[best]) {
                  best = k;
               }
            }
         }
         if (best != p) {
            attempt.truncate(attempt.length() - matchedYet[best]);
            attempt += newNumbers[best];
            met[best] = true;
            for (k = 0; k < p; k++) {
               matchedYet[k] = 0;
            }
            break;
         }
      }
   }

   /*
     We flag two kinds of suspicious cases. They are identified as
     such with comments such as "{2000?}" at the end.

     Example of the first kind: old source text "TeX 3.0" translated
     as "XeT 2.0" is flagged "TeX 2.0 {3.0?}", no matter what the
     new text is.
   */

   for (k = 0; k < p; k++) {
      if (! met[k]) {
         attempt += " {" + newNumbers[k] + "?}";
      }
   }

   /*
     Example of the second kind: "1 of 1" translated as "1 af 1",
     with new source text "1 of 2", generates "1 af 2 {1 or 2?}"
     because it's not clear which of "1 af 2" and "2 af 1" is right.
   */
   for (k = 0; k < p; k++) {
      for (ell = 0; ell < p; ell++) {
         if (k != ell && oldNumbers[k] == oldNumbers[ell] && newNumbers[k] < newNumbers[ell])
            attempt += " {" + newNumbers[k] + " or " + newNumbers[ell] + "?}";
      }
   }
   return attempt;
}


/*
  Augments a Translator with translations easily derived from
  similar existing (probably obsolete) translations.

  For example, if "TeX 3.0" is translated as "XeT 3.0" and "TeX 3.1"
  has no translation, "XeT 3.1" is added to the translator and is
  marked Unfinished.

  Returns the number of additional messages that this heuristic translated.
*/
int applyNumberHeuristic(Translator &tor)
{
   QMap<QString, QPair<QString, QString>> translated;
   QVector<bool> untranslated(tor.messageCount());
   int inserted = 0;

   for (int i = 0; i < tor.messageCount(); ++i) {
      const TranslatorMessage &msg = tor.message(i);
      bool hasTranslation = msg.isTranslated();

      if (msg.type() == TranslatorMessage::Type::Unfinished) {
         if (!hasTranslation) {
            untranslated[i] = true;
         }

      } else if (hasTranslation && msg.translations().count() == 1) {
         const QString &key = zeroKey(msg.sourceText());
         if (!key.isEmpty()) {
            translated.insert(key, qMakePair(msg.sourceText(), msg.translation()));
         }
      }
   }

   for (int i = 0; i < tor.messageCount(); ++i) {
      if (untranslated[i]) {
         TranslatorMessage &msg = tor.message(i);
         const QString &key = zeroKey(msg.sourceText());

         if (!key.isEmpty()) {
            QMap<QString, QPair<QString, QString>>::const_iterator t = translated.constFind(key);

            if (t != translated.constEnd() && t->first != msg.sourceText()) {
               msg.setTranslation(translationAttempt(t->second, t->first, msg.sourceText()));
               inserted++;
            }
         }
      }
   }
   return inserted;
}


/*
  Augments a Translator with trivially derived translations.

  For example, if "Enabled:" is consistendly translated as "Eingeschaltet:" no
  matter the context or the comment, "Eingeschaltet:" is added as the
  translation of any untranslated "Enabled:" text and is marked Unfinished.

  Returns the number of additional messages that this heuristic translated.
*/

int applySameTextHeuristic(Translator &tor)
{
   QMap<QString, QStringList> translated;
   QMap<QString, bool> avoid; // Want a QTreeSet, in fact
   QVector<bool> untranslated(tor.messageCount());
   int inserted = 0;

   for (int i = 0; i < tor.messageCount(); ++i) {
      const TranslatorMessage &msg = tor.message(i);
      if (!msg.isTranslated()) {
         if (msg.type() == TranslatorMessage::Type::Unfinished) {
            untranslated[i] = true;
         }

      } else {
         const QString &key = msg.sourceText();
         QMap<QString, QStringList>::const_iterator t = translated.constFind(key);

         if (t != translated.constEnd()) {
            /*
              The same source text is translated at least two
              different ways. Do nothing then.
            */
            if (*t != msg.translations()) {
               translated.remove(key);
               avoid.insert(key, true);
            }

         } else if (!avoid.contains(key)) {
            translated.insert(key, msg.translations());
         }
      }
   }

   for (int i = 0; i < tor.messageCount(); ++i) {
      if (untranslated[i]) {
         TranslatorMessage &msg = tor.message(i);
         QMap<QString, QStringList>::const_iterator t = translated.constFind(msg.sourceText());

         if (t != translated.constEnd()) {
            msg.setTranslations(*t);
            ++inserted;
         }
      }
   }
   return inserted;
}

/*
  Merges two Translator objects. The first one
  is a set of source texts and translations for a previous version of
  the internationalized program; the second one is a set of fresh
  source texts newly extracted from the source code, without any
  translation yet.
*/

Translator merge(const Translator &tor, const Translator &virginTor, const QList<Translator> &aliens,
                 UpdateOptions options, QString &err)
{
   int known = 0;
   int neww = 0;
   int obsoleted = 0;
   int similarTextHeuristicCount = 0;

   Translator outTor;
   outTor.setLanguageCode(tor.languageCode());
   outTor.setSourceLanguageCode(tor.sourceLanguageCode());
   outTor.setLocationsType(tor.locationsType());

   /*
     The types of all the messages from the vernacular translator
     are updated according to the virgin translator.
   */
   for (TranslatorMessage m : tor.messages()) {
      TranslatorMessage::Type newType = TranslatorMessage::Type::Finished;

      if (m.sourceText().isEmpty() && m.id().isEmpty()) {
         // context/file comment
         int mvi = virginTor.find(m.context());

         if (mvi >= 0) {
            m.setComment(virginTor.constMessage(mvi).comment());
         }

      } else {
         QHash<QString, QString> extras;

         const TranslatorMessage *mv;
         int mvi = virginTor.find(m);

         if (mvi < 0) {
            if (! (options & HeuristicSimilarText)) {

            makeObsolete:

               switch (m.type()) {
                  case TranslatorMessage::Type::Finished:
                     newType = TranslatorMessage::Type::Vanished;
                     ++obsoleted;
                     break;

                  case TranslatorMessage::Type::Unfinished:

                     newType = TranslatorMessage::Type::Obsolete;
                     ++obsoleted;
                     break;

                  default:
                     newType = m.type();
                     break;
               }

               m.clearReferences();

            } else {
               mvi = virginTor.find(m.context(), m.comment(), m.allReferences());

               if (mvi < 0) {
                  // did not find it in the virgin, mark it as obsolete
                  goto makeObsolete;
               }

               mv = &virginTor.constMessage(mvi);

               // Do not just accept it if its on the same line number,
               // but different source text.
               // Also check if the texts are more or less similar before
               // we consider them to represent the same message

               if (getSimilarityScore(m.sourceText(), mv->sourceText()) < textSimilarityThreshold) {
                  goto makeObsolete;
               }

               // It is just slightly modified, assume that it is the same string
               extras = mv->extras();
               // Mark it as unfinished. (Since the source text
               // was changed it might require re-translating...)

               newType = TranslatorMessage::Type::Unfinished;
               ++similarTextHeuristicCount;
               ++neww;

               goto outdateSource;
            }

         } else {
            mv = &virginTor.message(mvi);
            extras = mv->extras();

            if (! mv->id().isEmpty()
                  && (mv->context() != m.context()
                      || mv->sourceText() != m.sourceText()
                      || mv->comment() != m.comment())) {
               known++;
               newType = TranslatorMessage::Type::Unfinished;
               m.setContext(mv->context());
               m.setComment(mv->comment());

               if (mv->sourceText() != m.sourceText()) {

               outdateSource:
                  m.setOldSourceText(m.sourceText());
                  m.setSourceText(mv->sourceText());
                  const QString &oldpluralsource = m.extra("po-msgid_plural");

                  if (!oldpluralsource.isEmpty()) {
                     extras.insert("po-old_msgid_plural", oldpluralsource);
                  }
               }

            } else {
               switch (m.type()) {
                  case TranslatorMessage::Type::Finished:
                  default:
                     if (m.isPlural() == mv->isPlural()) {
                        newType = TranslatorMessage::Type::Finished;
                     } else {
                        newType = TranslatorMessage::Type::Unfinished;
                     }

                     ++known;
                     break;

                  case TranslatorMessage::Type::Unfinished:
                     newType = TranslatorMessage::Type::Unfinished;
                     ++known;
                     break;

                  case TranslatorMessage::Type::Vanished:
                     newType = TranslatorMessage::Type::Finished;
                     ++neww;
                     break;

                  case TranslatorMessage::Type::Obsolete:
                     newType = TranslatorMessage::Type::Unfinished;
                     ++neww;
                     break;
               }
            }

            // Always get the filename and linenumber info from the
            // virgin Translator, in case it has changed location.
            // This should also enable us to read a file that does not
            // have the <location> element.
            // why not use operator=()? Because it overwrites e.g. userData

            m.setReferences(mv->allReferences());
            m.setPlural(mv->isPlural());
            m.setExtras(extras);
            m.setExtraComment(mv->extraComment());
            m.setId(mv->id());
         }
      }

      m.setType(newType);
      outTor.append(m);
   }

   /*
     Messages found only in the virgin translator are added to the
     vernacular translator.
   */
   for (const TranslatorMessage &mv : virginTor.messages()) {
      if (mv.sourceText().isEmpty() && mv.id().isEmpty()) {
         if (tor.find(mv.context()) >= 0) {
            continue;
         }

      } else {
         if (tor.find(mv) >= 0) {
            continue;
         }

         if (options & HeuristicSimilarText) {
            int mi = tor.find(mv.context(), mv.comment(), mv.allReferences());

            if (mi >= 0) {
               if (getSimilarityScore(tor.constMessage(mi).sourceText(), mv.sourceText()) >= textSimilarityThreshold) {
                  continue;
               }
            }
         }
      }

      if (options & NoLocations) {
         outTor.append(mv);

      } else {
         outTor.appendSorted(mv);
      }

      if (! mv.sourceText().isEmpty() || !mv.id().isEmpty()) {
         ++neww;
      }
   }

   /*
   "Alien" translators can be used to augment the vernacular translator.
   */

   for (const Translator &alf : aliens) {

      for (TranslatorMessage mv : alf.messages()) {

         if (mv.sourceText().isEmpty() || ! mv.isTranslated()) {
            continue;
         }

         int mvi = outTor.find(mv);

         if (mvi >= 0) {
            TranslatorMessage &tm = outTor.message(mvi);

            if (tm.type() != TranslatorMessage::Type::Finished && ! tm.isTranslated()) {
               tm.setTranslations(mv.translations());
               --neww;
               ++known;
            }

         } else {

            // Add the unmatched messages as obsoletes, so the Linguist GUI
            // will offer them as possible translations.


            mv.clearReferences();
            mv.setType(mv.type() == TranslatorMessage::Type::Finished
                       ? TranslatorMessage::Type::Vanished : TranslatorMessage::Type::Obsolete);

            if (options & NoLocations) {
               outTor.append(mv);
            } else {
               outTor.appendSorted(mv);
            }

            ++known;
            ++obsoleted;
         }
      }
   }

   /*
     The same-text heuristic handles cases where a message has an
     obsolete counterpart with a different context or comment.
   */
   int sameTextHeuristicCount = (options & HeuristicSameText) ? applySameTextHeuristic(outTor) : 0;

   /*
     The number heuristic handles cases where a message has an
     obsolete counterpart with mostly numbers differing in the
     source text.
   */
   int sameNumberHeuristicCount = (options & HeuristicNumber) ? applyNumberHeuristic(outTor) : 0;

   if (options & Verbose) {
      int totalFound = neww + known;

      err += QString("    Found %1 source text(s) (%2 new and %3 already existing)\n")
             .formatArg(totalFound).formatArg(neww).formatArg(known);

      if (obsoleted) {
         if (options & NoObsolete) {
            err += QString("    Removed %1 obsolete entries\n").formatArg(obsoleted);

         } else {
            err += QString("    Kept %1 obsolete entries\n").formatArg(obsoleted);
         }
      }

      if (sameNumberHeuristicCount) {
         err += QString("    Number heuristic provided %1 translation(s)\n").formatArg(sameNumberHeuristicCount);
      }

      if (sameTextHeuristicCount) {
         err += QString("    Same-text heuristic provided %1 translation(s)\n").formatArg(sameTextHeuristicCount);
      }

      if (similarTextHeuristicCount) {
         err += QString("    Similar-text heuristic provided %1 translation(s)\n").formatArg(similarTextHeuristicCount);
      }
   }

   return outTor;
}

