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

#include <messagemodel.h>

#include <qapplication.h>
#include <qcoreapplication.h>
#include <qalgorithms.h>
#include <qdebug.h>
#include <qtextcodec.h>
#include <qmessagebox.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qtextdocument.h>

#include <qtranslator_p.h>

#include <limits.h>

MessageItem::MessageItem(const TranslatorMessage &message)
   : m_message(message), m_danger(false)
{
   if (m_message.translation().isEmpty()) {
      m_message.setTranslation(QString());
   }
}

bool MessageItem::compare(const QString &findText, bool matchSubstring, Qt::CaseSensitivity cs) const
{
   return matchSubstring
          ? text().indexOf(findText, 0, cs) >= 0
          : text().compare(findText, cs) == 0;
}

ContextItem::ContextItem(const QString &context)
   : m_context(context), m_finishedCount(0), m_finishedDangerCount(0),
     m_unfinishedDangerCount(0), m_nonobsoleteCount(0)
{
}

void ContextItem::appendToComment(const QString &str)
{
   if (! m_comment.isEmpty()) {
      m_comment += "\n\n";
   }

   m_comment += str;
}

MessageItem *ContextItem::messageItem(int i) const
{
   if (i >= 0 && i < msgItemList.count()) {
      return const_cast<MessageItem *>(&msgItemList[i]);
   }

   Q_ASSERT(i >= 0 && i < msgItemList.count());

   return nullptr;
}

MessageItem *ContextItem::findMessage(const QString &sourcetext, const QString &comment) const
{
   for (int i = 0; i < messageCount(); ++i) {
      MessageItem *mi = messageItem(i);

      if (mi->text() == sourcetext && mi->comment() == comment) {
         return mi;
      }
   }

   return nullptr;
}

DataModel::DataModel(QObject *parent)
   : QObject(parent), m_modified(false), m_numMessages(0), m_srcWords(0), m_srcChars(0),
     m_srcCharsSpc(0), m_language(QLocale::Language(-1)), m_sourceLanguage(QLocale::Language(-1)),
     m_country(QLocale::Country(-1)), m_sourceCountry(QLocale::Country(-1))
{
}

QStringList DataModel::normalizedTranslations(const MessageItem &m) const
{
   return Translator::normalizedTranslations(m.message(), m_numerusForms.count());
}

ContextItem *DataModel::getContextItem(int context) const
{
   if (context >= 0 && context < m_contextList.count()) {
      return const_cast<ContextItem *>(&m_contextList[context]);
   }

   Q_ASSERT(context >= 0 && context < m_contextList.count());

   return nullptr;
}

MessageItem *DataModel::getMessageItem(const DataIndex &index) const
{
   if (ContextItem *subject = getContextItem(index.context())) {
      return subject->messageItem(index.message());
   }

   return nullptr;
}

ContextItem *DataModel::findContext(const QString &context) const
{
   for (int index = 0; index < m_contextList.count(); ++index) {
      ContextItem *subject = getContextItem(index);

      if (subject->context() == context) {
         return subject;
      }
   }

   return nullptr;
}

MessageItem *DataModel::findMessage(const QString &context, const QString &sourcetext,
               const QString &comment) const
{
   if (ContextItem *ctx = findContext(context)) {
      return ctx->findMessage(sourcetext, comment);
   }

   return nullptr;
}

static int calcMergeScore(const DataModel *one, const DataModel *two)
{
   int inBoth = 0;

   for (int index = 0; index < two->contextCount(); ++index) {
      ContextItem *subject1 = two->getContextItem(index);
      ContextItem *subject2 = one->findContext(subject1->context());

      if (subject2 != nullptr) {

         for (int j = 0; j < subject1->messageCount(); ++j) {
            MessageItem *msgCargo = subject1->messageItem(j);

            if (subject2->findMessage(msgCargo->text(), msgCargo->comment()) != nullptr) {
               ++inBoth;
            }
         }
      }
   }

   return inBoth * 100 / two->messageCount();
}

bool DataModel::isWellMergeable(const DataModel *other) const
{
   if (!other->messageCount() || !messageCount()) {
      return true;
   }

   return calcMergeScore(this, other) + calcMergeScore(other, this) > 90;
}

bool DataModel::load(const QString &fileName, bool *langGuessed, QWidget *parent, bool &waitCursor)
{
   Translator tor;
   ConversionData cd;

   bool ok = tor.load(fileName, cd, "auto");

   if (! ok) {
      if (waitCursor) {
         QApplication::restoreOverrideCursor();
         waitCursor = false;
      }

      QMessageBox::warning(parent, QObject::tr("Linguist"), cd.error());
      return false;
   }

   if (! tor.messageCount()) {
      if (waitCursor) {
         QApplication::restoreOverrideCursor();
         waitCursor = false;
      }

      QMessageBox::warning(parent, QObject::tr("Linguist"),
               tr("Translation file %1 is empty and will not be loaded.").formatArg(fileName.toHtmlEscaped()));

      return false;
   }

   Translator::Duplicates dupes = tor.resolveDuplicates();

   if (! dupes.byId.isEmpty() || ! dupes.byContents.isEmpty()) {
      QString err = tr("Duplicate messages found in '%1':").formatArg(fileName.toHtmlEscaped());
      int numdups = 0;

      for (int i : dupes.byId) {
         if (++numdups >= 5) {
            err += tr("<p>[more duplicates omitted]");
            goto doWarn;
         }

         err += tr("<p>* ID: %1").formatArg(tor.message(i).id().toHtmlEscaped());
      }

      for (int j : dupes.byContents) {
         const TranslatorMessage &msg = tor.message(j);
         if (++numdups >= 5) {
            err += tr("<p>[more duplicates omitted]");
            break;
         }

         err += tr("<p>* Context: %1<br>* Source: %2").formatArgs(msg.context().toHtmlEscaped(), msg.sourceText().toHtmlEscaped());

         if (! msg.comment().isEmpty()) {
            err += tr("<br>* Comment: %3").formatArg(msg.comment().toHtmlEscaped());
         }
      }

   doWarn:
      if (waitCursor) {
         QApplication::restoreOverrideCursor();
         waitCursor = false;
      }

      QMessageBox::warning(parent, QObject::tr("Linguist"), err);
   }

   m_srcFileName = fileName;

   m_relativeLocations = (tor.locationsType() == Translator::RelativeLocations);
   m_extra = tor.extras();
   m_contextList.clear();
   m_numMessages = 0;

   QHash<QString, int> contexts;

   m_srcWords    = 0;
   m_srcChars    = 0;
   m_srcCharsSpc = 0;

   for (const TranslatorMessage &item : tor.messages()) {
      if (! contexts.contains(item.context())) {
         contexts.insert(item.context(), m_contextList.size());
         m_contextList.append(ContextItem(item.context()));
      }

      ContextItem *subject = getContextItem(contexts.value(item.context()));

      MessageItem msgCargo(item);

      if (item.type() == TranslatorMessage::Type::Finished) {
         subject->incrementFinishedCount();
      }

      if (item.type() == TranslatorMessage::Type::Finished || item.type() == TranslatorMessage::Type::Unfinished) {
         doCharCounting(msgCargo.text(), m_srcWords, m_srcChars, m_srcCharsSpc);
         doCharCounting(msgCargo.pluralText(), m_srcWords, m_srcChars, m_srcCharsSpc);
         subject->incrementNonobsoleteCount();
      }

      subject->appendMessage(msgCargo);
      ++m_numMessages;
   }

   // Try to detect the correct language in the following order
   // 1. Look for the language attribute in the ts if that fails
   // 2. Guestimate the language from the filename  (expecting the qt_{en,de}.ts convention)
   //   if that fails
   // 3. Retrieve the locale from the system.

   *langGuessed = false;

   QString lang = tor.languageCode();

   if (lang.isEmpty()) {
      lang = QFileInfo(fileName).baseName();

      int pos = lang.indexOf('_');

      if (pos != -1 && pos + 3 == lang.length()) {
         lang = fileName.mid(pos + 1);
      } else {
         lang.clear();
      }

      *langGuessed = true;
   }

   QLocale::Language langLocale;
   QLocale::Country  countryLocale;

   Translator::languageAndCountry(lang, &langLocale, &countryLocale);

   if (langLocale == QLocale::C) {
      QLocale sys;
      langLocale    = sys.language();
      countryLocale = sys.country();

      *langGuessed = true;
   }

   if (! setLanguageAndCountry(langLocale, countryLocale)) {
      if (waitCursor) {
         QApplication::restoreOverrideCursor();
         waitCursor = false;
      }

      QMessageBox::warning(parent, QObject::tr("Linguist"),
               tr("Linguist does not know the plural rules for %1.\n"
               "Using a single universal form.").formatArg(m_localizedLanguage));
   }

   // Try to detect the correct source language in the following order
   // 1. Look for the language attribute in the ts
   //   if that fails
   // 2. Assume English

   lang = tor.sourceLanguageCode();

   if (lang.isEmpty()) {
      langLocale    = QLocale::C;
      countryLocale = QLocale::AnyCountry;

   } else {
      Translator::languageAndCountry(lang, &langLocale, &countryLocale);
   }

   setSourceLanguageAndCountry(langLocale, countryLocale);
   setModified(false);

   return true;
}

bool DataModel::save(const QString &fileName, QWidget *parent)
{
   Translator tor;
   for (DataModelIterator it(this); it.isValid(); ++it) {
      tor.append(it.current()->message());
   }

   tor.setLanguageCode(Translator::makeLanguageCode(m_language, m_country));
   tor.setSourceLanguageCode(Translator::makeLanguageCode(m_sourceLanguage, m_sourceCountry));

   tor.setLocationsType(m_relativeLocations ? Translator::RelativeLocations
                        : Translator::AbsoluteLocations);
   tor.setExtras(m_extra);
   ConversionData cd;

   tor.normalizeTranslations(cd);

   bool ok = tor.save(fileName, cd, "auto");
   if (ok) {
      setModified(false);
   }

   if (! cd.error().isEmpty()) {
      QMessageBox::warning(parent, QObject::tr("Linguist"), cd.error());
   }

   return ok;
}

bool DataModel::saveAs(const QString &newFileName, QWidget *parent)
{
   if (!save(newFileName, parent)) {
      return false;
   }

   m_srcFileName = newFileName;
   return true;
}

bool DataModel::release(const QString &fileName, bool verbose, bool ignoreUnfinished,
                        TranslatorMessage::SaveMode mode, QWidget *parent)
{
   QFile file(fileName);
   if (!file.open(QIODevice::WriteOnly)) {
      QMessageBox::warning(parent, QObject::tr("Linguist"),
                           tr("Unable to create '%2': %1").formatArg(file.errorString()).formatArg(fileName));
      return false;
   }

   Translator tor;
   QLocale locale(m_language, m_country);
   tor.setLanguageCode(locale.name());
   for (DataModelIterator it(this); it.isValid(); ++it) {
      tor.append(it.current()->message());
   }

   ConversionData cd;
   cd.m_verbose = verbose;
   cd.m_ignoreUnfinished = ignoreUnfinished;
   cd.m_saveMode = mode;

   bool ok = saveQM(tor, file, cd);
   if (! ok) {
      QMessageBox::warning(parent, QObject::tr("Linguist"), cd.error());
   }
   return ok;
}

void DataModel::doCharCounting(const QString &text, int &trW, int &trC, int &trCS)
{
   trCS += text.length();
   bool inWord = false;

   for (int i = 0; i < text.length(); ++i) {
      if (text[i].isLetterOrNumber() || text[i] == QChar('_')) {
         if (! inWord) {
            ++trW;
            inWord = true;
         }
      } else {
         inWord = false;
      }

      if (!text[i].isSpace()) {
         trC++;
      }
   }
}

bool DataModel::setLanguageAndCountry(QLocale::Language lang, QLocale::Country country)
{
   if (m_language == lang && m_country == country) {
      return true;
   }

   m_language = lang;
   m_country  = country;

   if (lang == QLocale::C || uint(lang) > uint(QLocale::LastLanguage)) {
      // default
      lang = QLocale::English;
   }

   QVector<std::variant<CountGuide, int>> data;
   bool ok = getCountInfo(lang, country, &data, &m_numerusForms, nullptr);

   m_localizedLanguage = QCoreApplication::translate("MessageEditor", QLocale::languageToString(lang).constData());
   m_countRefNeeds.clear();

   for (int index = 0; index < data.size(); ++index) {
      bool x1 = true;
      bool x2 = false;

      const CountGuide *ptr = std::get_if<CountGuide>(&data.at(index));

      if (ptr != nullptr && *ptr == CountGuide::Equal) {
         x1 = false;
      }

      if (! x1) {
         int tmpSize = data.size() - 2;

         if (index != tmpSize) {
            const CountGuide *ptr = std::get_if<CountGuide>(&data.at(index + 2));

            if (ptr != nullptr && *ptr != CountGuide::LastEntry) {
               x2 = true;
            }
         }
      }

      m_countRefNeeds.append(x1 || x2);

      while (true) {
         ++index;

         if (index >= data.size()) {
            break;
         }

         const CountGuide *ptr = std::get_if<CountGuide>(&data.at(index));

         if (ptr != nullptr && *ptr == CountGuide::LastEntry) {
            break;
         }
      }
   }

   m_countRefNeeds.append(true);

   if (! ok) {
      m_numerusForms.clear();
      m_numerusForms << tr("Universal Form");
   }

   emit languageChanged();
   setModified(true);

   return ok;
}

void DataModel::setSourceLanguageAndCountry(QLocale::Language lang, QLocale::Country country)
{
   if (m_sourceLanguage == lang && m_sourceCountry == country) {
      return;
   }

   m_sourceLanguage = lang;
   m_sourceCountry  = country;
   setModified(true);
}

void DataModel::updateStatistics()
{
   int trW  = 0;
   int trC  = 0;
   int trCS = 0;

   for (DataModelIterator it(this); it.isValid(); ++it) {
      const MessageItem *mi = it.current();

      if (mi->isFinished()) {
         for (const QString & trnsl : mi->translations()) {
            doCharCounting(trnsl, trW, trC, trCS);
         }
      }
   }

   emit statsChanged(m_srcWords, m_srcChars, m_srcCharsSpc, trW, trC, trCS);
}

void DataModel::setModified(bool isModified)
{
   if (m_modified == isModified) {
      return;
   }

   m_modified = isModified;
   emit modifiedChanged();
}

QString DataModel::prettifyPlainFileName(const QString &fn)
{
   static QString workdir = QDir::currentPath() + '/';

   return QDir::toNativeSeparators(fn.startsWith(workdir) ? fn.mid(workdir.length()) : fn);
}

QString DataModel::prettifyFileName(const QString &fn)
{
   if (fn.startsWith('=')) {
      return '=' + prettifyPlainFileName(fn.mid(1));

   } else {
      return prettifyPlainFileName(fn);
   }
}

DataModelIterator::DataModelIterator(DataModel *model, int context, int message)
   : DataIndex(context, message), m_model(model)
{
}

bool DataModelIterator::isValid() const
{
   return m_context < m_model->m_contextList.count();
}

void DataModelIterator::operator++()
{
   ++m_message;
   if (m_message >= m_model->m_contextList.at(m_context).messageCount()) {
      ++m_context;
      m_message = 0;
   }
}

MessageItem *DataModelIterator::current() const
{
   return m_model->getMessageItem(*this);
}

MultiMessageItem::MultiMessageItem(const MessageItem *msgCargo)
   : m_id(msgCargo->id()), m_text(msgCargo->text()), m_pluralText(msgCargo->pluralText()), m_comment(msgCargo->comment()),
     m_nonnullCount(0), m_nonobsoleteCount(0), m_editableCount(0), m_unfinishedCount(0)
{
}

MultiContextItem::MultiContextItem(int oldCount, ContextItem *ctx, bool writable)
   : m_context(ctx->context()), m_comment(ctx->comment()), m_finishedCount(0),
     m_editableCount(0), m_nonobsoleteCount(0)
{
   QList<MessageItem *> mList;
   QList<MessageItem *> eList;

   for (int j = 0; j < ctx->messageCount(); ++j) {
      MessageItem *m = ctx->messageItem(j);
      mList.append(m);
      eList.append(nullptr);
      m_multiMessageList.append(MultiMessageItem(m));
   }

   for (int i = 0; i < oldCount; ++i) {
      m_messageLists.append(eList);
      m_writableMessageLists.append(nullptr);
      m_contextList.append(nullptr);
   }

   m_messageLists.append(mList);
   m_writableMessageLists.append(writable ? &m_messageLists.last() : nullptr);
   m_contextList.append(ctx);
}

void MultiContextItem::appendEmptyModel()
{
   QList<MessageItem *> eList;
   for (int j = 0; j < messageCount(); ++j) {
      eList.append(nullptr);
   }

   m_messageLists.append(eList);
   m_writableMessageLists.append(nullptr);
   m_contextList.append(nullptr);
}

void MultiContextItem::assignLastModel(ContextItem *ctx, bool writable)
{
   if (writable) {
      m_writableMessageLists.last() = &m_messageLists.last();
   }

   m_contextList.last() = ctx;
}

// this is not needed yet
void MultiContextItem::moveModel(int oldPos, int newPos)
{
   m_contextList.insert(newPos, m_contextList[oldPos]);
   m_messageLists.insert(newPos, m_messageLists[oldPos]);
   m_writableMessageLists.insert(newPos, m_writableMessageLists[oldPos]);
   removeModel(oldPos < newPos ? oldPos : oldPos + 1);
}

void MultiContextItem::removeModel(int pos)
{
   m_contextList.removeAt(pos);
   m_messageLists.removeAt(pos);
   m_writableMessageLists.removeAt(pos);
}

void MultiContextItem::putMessageItem(int pos, MessageItem *m)
{
   m_messageLists.last()[pos] = m;
}

void MultiContextItem::appendMessageItems(const QList<MessageItem *> &m)
{
   QList<MessageItem *> nullItems = m; // Basically, just a reservation

   for (int i = 0; i < nullItems.count(); ++i) {
      nullItems[i] = nullptr;
   }

   for (int i = 0; i < m_messageLists.count() - 1; ++i) {
      m_messageLists[i] += nullItems;
   }

   m_messageLists.last() += m;

   for (MessageItem * mi : m)
   m_multiMessageList.append(MultiMessageItem(mi));
}

void MultiContextItem::removeMultiMessageItem(int pos)
{
   for (int i = 0; i < m_messageLists.count(); ++i) {
      m_messageLists[i].removeAt(pos);
   }

   m_multiMessageList.removeAt(pos);
}

int MultiContextItem::firstNonobsoleteMessageIndex(int msgIdx) const
{
   for (int i = 0; i < m_messageLists.size(); ++i) {
      if (m_messageLists[i][msgIdx] && !m_messageLists[i][msgIdx]->isObsolete()) {
         return i;
      }
   }

   return -1;
}

int MultiContextItem::findMessage(const QString &sourcetext, const QString &comment) const
{
   for (int i = 0, cnt = messageCount(); i < cnt; ++i) {
      MultiMessageItem *mm = multiMessageItem(i);

      if (mm->text() == sourcetext && mm->comment() == comment) {
         return i;
      }
   }

   return -1;
}

int MultiContextItem::findMessageById(const QString &id) const
{
    for (int i = 0, cnt = messageCount(); i < cnt; ++i) {
        MultiMessageItem *mm = multiMessageItem(i);

        if (mm->id() == id) {
            return i;
        }
    }

    return -1;
}

static const uchar paletteRGBs[7][3] = {
   { 236, 244, 255 }, // blue
   { 236, 255, 255 }, // cyan
   { 236, 255, 232 }, // green
   { 255, 255, 230 }, // yellow
   { 255, 242, 222 }, // orange
   { 255, 236, 236 }, // red
   { 252, 236, 255 }  // purple
};

MultiDataModel::MultiDataModel(QObject *parent)
   : QObject(parent), m_numFinished(0), m_numEditable(0), m_numMessages(0), m_modified(false)
{
   for (int i = 0; i < 7; ++i) {
      m_colors[i] = QColor(paletteRGBs[i][0], paletteRGBs[i][1], paletteRGBs[i][2]);
   }

   m_bitmap = QBitmap(8, 8);
   m_bitmap.clear();

   QPainter p(&m_bitmap);

   for (int j = 0; j < 8; ++j) {
      for (int k = 0; k < 8; ++k) {
         if ((j + k) & 4) {
            p.drawPoint(j, k);
         }
      }
   }
}

MultiDataModel::~MultiDataModel()
{
   qDeleteAll(m_dataModels);
}

QBrush MultiDataModel::brushForModel(int model) const
{
   QBrush brush(m_colors[model % 7]);
   if (!isModelWritable(model)) {
      brush.setTexture(m_bitmap);
   }

   return brush;
}

bool MultiDataModel::isWellMergeable(const DataModel *dm) const
{
   if (! dm->messageCount() || ! messageCount()) {
      return true;
   }

   int inBothNew = 0;

   for (int index = 0; index < dm->contextCount(); ++index) {
      ContextItem *subject = dm->getContextItem(index);
      MultiContextItem *mc = findContext(subject->context());

      if (mc != nullptr) {

         for (int j = 0; j < subject->messageCount(); ++j) {
            MessageItem *msgCargo = subject->messageItem(j);

            if (mc->findMessage(msgCargo->text(), msgCargo->comment()) >= 0) {
               ++inBothNew;
            }
         }
      }
   }

   int newRatio  = inBothNew * 100 / dm->messageCount();
   int inBothOld = 0;

   for (int index = 0; index < contextCount(); ++index) {
      MultiContextItem *mc = multiContextItem(index);

      if (ContextItem *subject = dm->findContext(mc->context())) {

         for (int j = 0; j < mc->messageCount(); ++j) {
            MultiMessageItem *mm = mc->multiMessageItem(j);

            if (subject->findMessage(mm->text(), mm->comment()) != nullptr) {
               ++inBothOld;
            }
         }
      }
   }

   int oldRatio = inBothOld * 100 / messageCount();

   return newRatio + oldRatio > 90;
}

void MultiDataModel::append(DataModel *dm, bool readWrite)
{
   int insCol = modelCount() + 1;
   m_msgModel->beginInsertColumns(QModelIndex(), insCol, insCol);
   m_dataModels.append(dm);

   for (int j = 0; j < contextCount(); ++j) {
      m_msgModel->beginInsertColumns(m_msgModel->createIndex(j, 0), insCol, insCol);
      m_multiContextList[j].appendEmptyModel();
      m_msgModel->endInsertColumns();
   }

   m_msgModel->endInsertColumns();
   int appendedContexts = 0;

   for (int i = 0; i < dm->contextCount(); ++i) {
      ContextItem *subject = dm->getContextItem(i);

      int mcx = findContextIndex(subject->context());

      if (mcx >= 0) {
         MultiContextItem *mc = multiContextItem(mcx);

         mc->assignLastModel(subject, readWrite);
         QList<MessageItem *> appendItems;

         for (int j = 0; j < subject->messageCount(); ++j) {
            MessageItem *msgCargo = subject->messageItem(j);

            int msgIdx = -1;

            if (! msgCargo->id().isEmpty())  {
               // id based translation
               msgIdx = mc->findMessageById(msgCargo->id());
            }

            if (msgIdx == -1) {
               msgIdx = mc->findMessage(msgCargo->text(), msgCargo->comment());
            }

            if (msgIdx >= 0) {
               mc->putMessageItem(msgIdx, msgCargo);
            } else {
               appendItems.append(msgCargo);
            }
         }

         if (! appendItems.isEmpty()) {
            int msgCnt = mc->messageCount();
            m_msgModel->beginInsertRows(m_msgModel->createIndex(mcx, 0), msgCnt, msgCnt + appendItems.size() - 1);

            mc->appendMessageItems(appendItems);
            m_msgModel->endInsertRows();
            m_numMessages += appendItems.size();
         }

      } else {
         m_multiContextList << MultiContextItem(modelCount() - 1, subject, readWrite);
         m_numMessages += subject->messageCount();
         ++appendedContexts;
      }
   }

   if (appendedContexts) {
      // Do that en block to avoid itemview inefficiency. It doesn't hurt that we
      // announce the availability of the data "long" after it was actually added.
      m_msgModel->beginInsertRows(QModelIndex(), contextCount() - appendedContexts, contextCount() - 1);
      m_msgModel->endInsertRows();
   }

   dm->setWritable(readWrite);
   updateCountsOnAdd(modelCount() - 1, readWrite);

   connect(dm, &DataModel::modifiedChanged, this, &MultiDataModel::onModifiedChanged);
   connect(dm, &DataModel::languageChanged, this, &MultiDataModel::onLanguageChanged);
   connect(dm, &DataModel::statsChanged,    this, &MultiDataModel::statsChanged);

   emit modelAppended();
}

void MultiDataModel::close(int model)
{
   if (m_dataModels.count() == 1) {
      closeAll();

   } else {
      updateCountsOnRemove(model, isModelWritable(model));
      int delCol = model + 1;
      m_msgModel->beginRemoveColumns(QModelIndex(), delCol, delCol);

      for (int i = m_multiContextList.size(); --i >= 0;) {
         m_msgModel->beginRemoveColumns(m_msgModel->createIndex(i, 0), delCol, delCol);
         m_multiContextList[i].removeModel(model);
         m_msgModel->endRemoveColumns();
      }

      delete m_dataModels.takeAt(model);

      m_msgModel->endRemoveColumns();
      emit modelDeleted(model);

      for (int index = m_multiContextList.size()-1; index >= 0; --index) {

         MultiContextItem &mc   = m_multiContextList[index];
         QModelIndex contextIdx = m_msgModel->createIndex(index, 0);

         for (int j = mc.messageCount() - 1; j >= 0; --j) {
            if (mc.multiMessageItem(j)->isEmpty()) {
               m_msgModel->beginRemoveRows(contextIdx, j, j);
               mc.removeMultiMessageItem(j);
               m_msgModel->endRemoveRows();

               --m_numMessages;
            }
         }

         if (mc.messageCount() == 0) {
            m_msgModel->beginRemoveRows(QModelIndex(), index, index);
            m_multiContextList.removeAt(index);
            m_msgModel->endRemoveRows();
         }
      }

      onModifiedChanged();
   }
}

void MultiDataModel::closeAll()
{
   m_msgModel->beginResetModel();
   m_numFinished = 0;
   m_numEditable = 0;
   m_numMessages = 0;

   qDeleteAll(m_dataModels);

   m_dataModels.clear();
   m_multiContextList.clear();
   m_msgModel->endResetModel();

   emit allModelsDeleted();
   onModifiedChanged();
}

// not needed yet
void MultiDataModel::moveModel(int oldPos, int newPos)
{
   int delPos = oldPos < newPos ? oldPos : oldPos + 1;
   m_dataModels.insert(newPos, m_dataModels[oldPos]);
   m_dataModels.removeAt(delPos);

   for (int i = 0; i < m_multiContextList.size(); ++i) {
      m_multiContextList[i].moveModel(oldPos, newPos);
   }
}

QStringList MultiDataModel::prettifyFileNames(const QStringList &names)
{
   QStringList out;

   for (const QString & name : names) {
    out << DataModel::prettifyFileName(name);
   }

   return out;
}

QString MultiDataModel::condenseFileNames(const QStringList &names)
{
   if (names.isEmpty()) {
      return QString();
   }

   if (names.count() < 2) {
      return names.first();
   }

   QString prefix = names.first();
   if (prefix.startsWith('=')) {
      prefix.remove(0, 1);
   }

   QString suffix = prefix;

   for (int i = 1; i < names.count(); ++i) {
      QString fn = names[i];
      if (fn.startsWith('=')) {
         fn.remove(0, 1);
      }

      for (int j = 0; j < prefix.length(); ++j)
         if (fn[j] != prefix[j]) {
            if (j < prefix.length()) {
               while (j > 0 && prefix[j - 1].isLetterOrNumber()) {
                  --j;
               }
               prefix.truncate(j);
            }
            break;
         }

      int fnl = fn.length() - 1;
      int sxl = suffix.length() - 1;

      for (int k = 0; k <= sxl; ++k)
         if (fn[fnl - k] != suffix[sxl - k]) {
            if (k < sxl) {
               while (k > 0 && suffix[sxl - k + 1].isLetterOrNumber()) {
                  --k;
               }

               if (prefix.length() + k > fnl) {
                  --k;
               }

               suffix.remove(0, sxl - k + 1);
            }
            break;
         }
   }

   QString ret = prefix + '{';

   int pxl = prefix.length();
   int sxl = suffix.length();

   for (int j = 0; j < names.count(); ++j) {
      if (j) {
         ret += ',';
      }

      int off = pxl;

      QString fn = names[j];

      if (fn.startsWith('=')) {
         ret += '=';
         ++off;
      }
      ret += fn.mid(off, fn.length() - sxl - off);
   }

   ret += '}' + suffix;

   return ret;
}

QStringList MultiDataModel::srcFileNames(bool pretty) const
{
   QStringList retval;

   for (DataModel *item : m_dataModels) {

      if (item->isWritable()) {
          retval.append(item->srcFileName(pretty));

      } else {
         retval.append("=" + item->srcFileName(pretty));

      }
   }

   return retval;
}

QString MultiDataModel::condensedSrcFileNames(bool pretty) const
{
   return condenseFileNames(srcFileNames(pretty));
}

bool MultiDataModel::isModified() const
{
   for (const DataModel *mdl : m_dataModels) {
      if (mdl->isModified()) {
         return true;
      }
   }

   return false;
}

void MultiDataModel::onModifiedChanged()
{
   bool modified = isModified();

   if (modified != m_modified) {
      emit modifiedChanged(modified);
      m_modified = modified;
   }
}

void MultiDataModel::onLanguageChanged()
{
   int i = 0;
   while (sender() != m_dataModels[i]) {
      ++i;
   }

   emit languageChanged(i);
}

int MultiDataModel::isFileLoaded(const QString &name) const
{
   for (int i = 0; i < m_dataModels.size(); ++i) {
      if (m_dataModels[i]->srcFileName() == name) {
         return i;
      }
   }

   return -1;
}

int MultiDataModel::findContextIndex(const QString &context) const
{
   for (int i = 0; i < m_multiContextList.size(); ++i) {
      const MultiContextItem &mc = m_multiContextList[i];

      if (mc.context() == context) {
         return i;
      }
   }

   return -1;
}

MultiContextItem *MultiDataModel::findContext(const QString &context) const
{
   for (int i = 0; i < m_multiContextList.size(); ++i) {
      const MultiContextItem &mc = m_multiContextList[i];
      if (mc.context() == context) {
         return const_cast<MultiContextItem *>(&mc);
      }
   }

   return nullptr;
}

MessageItem *MultiDataModel::getMessageItem(const MultiDataIndex &index, int model) const
{
   if (index.context() < contextCount() && model >= 0 && model < modelCount()) {
      MultiContextItem *mc = multiContextItem(index.context());

      if (index.message() < mc->messageCount()) {
         return mc->messageItem(model, index.message());
      }
   }

   Q_ASSERT(model >= 0 && model < modelCount());
   Q_ASSERT(index.context() < contextCount());

   return nullptr;
}

void MultiDataModel::setTranslation(const MultiDataIndex &index, const QString &translation)
{
   MessageItem *msgCargo = getMessageItem(index);

   if (translation == msgCargo->translation()) {
      return;
   }

   msgCargo->setTranslation(translation);
   setModified(index.model(), true);
   emit translationChanged(index);
}

void MultiDataModel::setFinished(const MultiDataIndex &index, bool finished)
{
   MultiContextItem *mc = multiContextItem(index.context());
   MultiMessageItem *mm = mc->multiMessageItem(index.message());

   ContextItem *subject  = getContextItem(index);
   MessageItem *msgCargo = getMessageItem(index);

   TranslatorMessage::Type type = msgCargo->type();

   if (type == TranslatorMessage::Type::Unfinished && finished) {
      msgCargo->setType(TranslatorMessage::Type::Finished);
      mm->decrementUnfinishedCount();

      if (! mm->countUnfinished()) {
         incrementFinishedCount();
         mc->incrementFinishedCount();
         emit multiContextDataChanged(index);
      }

      subject->incrementFinishedCount();

      if (msgCargo->danger()) {
         subject->incrementFinishedDangerCount();
         subject->decrementUnfinishedDangerCount();

         if (! subject->unfinishedDangerCount() || subject->finishedCount() == subject->nonobsoleteCount()) {
            emit contextDataChanged(index);
         }

      } else if (subject->finishedCount() == subject->nonobsoleteCount()) {
         emit contextDataChanged(index);
      }

      emit messageDataChanged(index);
      setModified(index.model(), true);

   } else if (type == TranslatorMessage::Type::Finished && !finished) {
      msgCargo->setType(TranslatorMessage::Type::Unfinished);
      mm->incrementUnfinishedCount();

      if (mm->countUnfinished() == 1) {
         decrementFinishedCount();
         mc->decrementFinishedCount();
         emit multiContextDataChanged(index);
      }

      subject->decrementFinishedCount();

      if (msgCargo->danger()) {
         subject->decrementFinishedDangerCount();
         subject->incrementUnfinishedDangerCount();

         if (subject->unfinishedDangerCount() == 1 || subject->finishedCount() + 1 == subject->nonobsoleteCount()) {
            emit contextDataChanged(index);
         }

      } else if (subject->finishedCount() + 1 == subject->nonobsoleteCount()) {
         emit contextDataChanged(index);
      }

      emit messageDataChanged(index);
      setModified(index.model(), true);
   }
}

void MultiDataModel::setDanger(const MultiDataIndex &index, bool danger)
{
   ContextItem *subject  = getContextItem(index);
   MessageItem *msgCargo = getMessageItem(index);

   if (! msgCargo->danger() && danger) {

      if (msgCargo->isFinished()) {
         subject->incrementFinishedDangerCount();

         if (subject->finishedDangerCount() == 1) {
            emit contextDataChanged(index);
         }

      } else {
         subject->incrementUnfinishedDangerCount();

         if (subject->unfinishedDangerCount() == 1) {
            emit contextDataChanged(index);
         }
      }

      emit messageDataChanged(index);
      msgCargo->setDanger(danger);

   } else if (msgCargo->danger() && ! danger) {
      if (msgCargo->isFinished()) {
         subject->decrementFinishedDangerCount();

         if (! subject->finishedDangerCount()) {
            emit contextDataChanged(index);
         }

      } else {
         subject->decrementUnfinishedDangerCount();

         if (! subject->unfinishedDangerCount()) {
            emit contextDataChanged(index);
         }
      }

      emit messageDataChanged(index);
      msgCargo->setDanger(danger);
   }
}

void MultiDataModel::updateCountsOnAdd(int model, bool writable)
{
   for (int i = 0; i < m_multiContextList.size(); ++i) {
      MultiContextItem &mc = m_multiContextList[i];

      for (int j = 0; j < mc.messageCount(); ++j) {
         MessageItem *msgCargo = mc.messageItem(model, j);

         if (msgCargo != nullptr) {
            MultiMessageItem *mm = mc.multiMessageItem(j);
            mm->incrementNonnullCount();

            if (! msgCargo->isObsolete()) {
               if (writable) {
                  if (! mm->countEditable()) {
                     mc.incrementEditableCount();
                     incrementEditableCount();

                     if (msgCargo->isFinished()) {
                        mc.incrementFinishedCount();
                        incrementFinishedCount();

                     } else {
                        mm->incrementUnfinishedCount();
                     }

                  } else if (! msgCargo->isFinished()) {
                     if (! mm->isUnfinished()) {
                        mc.decrementFinishedCount();
                        decrementFinishedCount();
                     }

                     mm->incrementUnfinishedCount();
                  }
                  mm->incrementEditableCount();
               }

               mc.incrementNonobsoleteCount();
               mm->incrementNonobsoleteCount();
            }
         }
      }
   }
}

void MultiDataModel::updateCountsOnRemove(int model, bool writable)
{
   for (int i = 0; i < m_multiContextList.size(); ++i) {
      MultiContextItem &mc = m_multiContextList[i];

      for (int j = 0; j < mc.messageCount(); ++j) {
         MessageItem *msgCargo = mc.messageItem(model, j);

         if (msgCargo != nullptr) {
            MultiMessageItem *mm = mc.multiMessageItem(j);
            mm->decrementNonnullCount();

            if (! msgCargo->isObsolete()) {
               mm->decrementNonobsoleteCount();
               mc.decrementNonobsoleteCount();

               if (writable) {
                  mm->decrementEditableCount();

                  if (! mm->countEditable()) {
                     mc.decrementEditableCount();
                     decrementEditableCount();

                     if (msgCargo->isFinished()) {
                        mc.decrementFinishedCount();
                        decrementFinishedCount();
                     } else {
                        mm->decrementUnfinishedCount();
                     }

                  } else if (! msgCargo->isFinished()) {
                     mm->decrementUnfinishedCount();

                     if (! mm->isUnfinished()) {
                        mc.incrementFinishedCount();
                        incrementFinishedCount();
                     }
                  }
               }
            }
         }
      }
   }
}


MultiDataModelIterator::MultiDataModelIterator(MultiDataModel *dataModel, int model, int context, int message)
   : MultiDataIndex(model, context, message), m_dataModel(dataModel)
{
}

void MultiDataModelIterator::operator++()
{
   Q_ASSERT(isValid());
   ++m_message;

   if (m_message >= m_dataModel->m_multiContextList.at(m_context).messageCount()) {
      ++m_context;
      m_message = 0;
   }
}

bool MultiDataModelIterator::isValid() const
{
   return m_context < m_dataModel->m_multiContextList.count();
}

MessageItem *MultiDataModelIterator::current() const
{
   return m_dataModel->getMessageItem(*this);
}

MessageModel::MessageModel(QObject *parent, MultiDataModel *data)
   : QAbstractItemModel(parent), m_data(data)
{
   data->m_msgModel = this;

   connect(m_data, &MultiDataModel::multiContextDataChanged, this, &MessageModel::multiContextItemChanged);
   connect(m_data, &MultiDataModel::contextDataChanged,      this, &MessageModel::contextItemChanged);
   connect(m_data, &MultiDataModel::messageDataChanged,      this, &MessageModel::messageItemChanged);
}

QModelIndex MessageModel::index(int row, int column, const QModelIndex &parent) const
{
   if (! parent.isValid()) {
      return createIndex(row, column);
   }

   if (! parent.internalId()) {
      return createIndex(row, column, parent.row() + 1);
   }

   return QModelIndex();
}

QModelIndex MessageModel::parent(const QModelIndex &index) const
{
   if (index.internalId()) {
      return createIndex(index.internalId() - 1, 0);
   }

   return QModelIndex();
}

void MessageModel::multiContextItemChanged(const MultiDataIndex &index)
{
   QModelIndex idx = createIndex(index.context(), m_data->modelCount() + 2);
   emit dataChanged(idx, idx);
}

void MessageModel::contextItemChanged(const MultiDataIndex &index)
{
   QModelIndex idx = createIndex(index.context(), index.model() + 1);
   emit dataChanged(idx, idx);
}

void MessageModel::messageItemChanged(const MultiDataIndex &index)
{
   QModelIndex idx = createIndex(index.message(), index.model() + 1, index.context() + 1);
   emit dataChanged(idx, idx);
}

QModelIndex MessageModel::modelIndex(const MultiDataIndex &index)
{
   if (index.message() < 0) {
      // Should be unused case
      return createIndex(index.context(), index.model() + 1);
   }

   return createIndex(index.message(), index.model() + 1, index.context() + 1);
}

int MessageModel::rowCount(const QModelIndex &parent) const
{
   if (! parent.isValid()) {
      // contexts
      return m_data->contextCount();
   }

   if (! parent.internalId()) {
      // messages
      return m_data->multiContextItem(parent.row())->messageCount();
   }

   return 0;
}

int MessageModel::columnCount(const QModelIndex &parent) const
{
   if (! parent.isValid())  {
      return m_data->modelCount() + 3;
   }

   return m_data->modelCount() + 2;
}

QVariant MessageModel::data(const QModelIndex &index, int role) const
{
   static QVariant pxOn       = QVariant::fromValue(QPixmap(":/images/s_check_on.png"));
   static QVariant pxOff      = QVariant::fromValue(QPixmap(":/images/s_check_off.png"));
   static QVariant pxObsolete = QVariant::fromValue(QPixmap(":/images/s_check_obsolete.png"));
   static QVariant pxDanger   = QVariant::fromValue(QPixmap(":/images/s_check_danger.png"));
   static QVariant pxWarning  = QVariant::fromValue(QPixmap(":/images/s_check_warning.png"));
   static QVariant pxEmpty    = QVariant::fromValue(QPixmap(":/images/s_check_empty.png"));

   int row = index.row();
   int column = index.column() - 1;

   if (column < 0) {
      return QVariant();
   }

   int numLangs = m_data->modelCount();

   if (role == Qt::ToolTipRole && column < numLangs) {
      return tr("Completion status for %1").formatArg(m_data->model(column)->localizedLanguage());

   } else if (index.internalId()) {
      // this is a message
      int crow = index.internalId() - 1;
      MultiContextItem *mci = m_data->multiContextItem(crow);

      if (row >= mci->messageCount() || ! index.isValid()) {
         return QVariant();
      }

      if (role == Qt::DisplayRole || (role == Qt::ToolTipRole && column == numLangs)) {
         switch (column - numLangs) {
            case 0: {
               // Source text

               MultiMessageItem *msgItem = mci->multiMessageItem(row);
               if (msgItem->text().isEmpty()) {
                  if (mci->context().isEmpty()) {
                     return tr("<file header>");
                  } else {
                     return tr("<context comment>");
                  }
               }
               return msgItem->text().simplified();
            }

            default: // Status or dummy column => no text
               return QVariant();
         }

      } else if (role == Qt::DecorationRole && column < numLangs) {
         if (MessageItem *msgItem = mci->messageItem(column, row)) {
            switch (msgItem->message().type()) {
               case TranslatorMessage::Type::Unfinished:
                  if (msgItem->translation().isEmpty()) {
                     return pxEmpty;
                  }
                  if (msgItem->danger()) {
                     return pxDanger;
                  }
                  return pxOff;

               case TranslatorMessage::Type::Finished:
                  if (msgItem->danger()) {
                     return pxWarning;
                  }
                  return pxOn;

               default:
                  return pxObsolete;
            }
         }
         return QVariant();

      } else if (role == SortRole) {
         switch (column - numLangs) {
            case 0:
               // Source text
               return mci->multiMessageItem(row)->text().simplified().remove('&');

            case 1:
               // Dummy column
               return QVariant();

            default:
               if (MessageItem *msgItem = mci->messageItem(column, row)) {
                  int rslt = !msgItem->translation().isEmpty();
                  if (!msgItem->danger()) {
                     rslt |= 2;
                  }
                  if (msgItem->isObsolete()) {
                     rslt |= 8;
                  } else if (msgItem->isFinished()) {
                     rslt |= 4;
                  }
                  return rslt;
               }
               return INT_MAX;
         }
      } else if (role == Qt::ForegroundRole && column > 0
                 && mci->multiMessageItem(row)->isObsolete()) {
         return QBrush(Qt::darkGray);
      } else if (role == Qt::ForegroundRole && column == numLangs
                 && mci->multiMessageItem(row)->text().isEmpty()) {
         return QBrush(QColor(0, 0xa0, 0xa0));

      } else if (role == Qt::BackgroundRole) {
         if (column < numLangs && numLangs != 1) {
            return m_data->brushForModel(column);
         }
      }

   } else {
      // this is a context
      if (row >= m_data->contextCount() || !index.isValid()) {
         return QVariant();
      }

      MultiContextItem *mci = m_data->multiContextItem(row);

      if (role == Qt::DisplayRole || (role == Qt::ToolTipRole && column == numLangs)) {
         switch (column - numLangs) {
            case 0: {
               // Context
               if (mci->context().isEmpty()) {
                  return tr("<unnamed context>");
               }
               return mci->context().simplified();
            }

            case 1: {
               QString s = QString("%1/%2").formatArg(mci->getNumFinished()).formatArg(mci->getNumEditable());
               return s;
            }

            default:
               return QVariant(); // Status => no text
         }

      } else if (role == Qt::DecorationRole && column < numLangs) {
         ContextItem *subject = mci->contextItem(column);

         if (subject != nullptr) {
            if (subject->isObsolete()) {
               return pxObsolete;
            }

            if (subject->isFinished()) {
               return subject->finishedDangerCount() > 0 ? pxWarning : pxOn;
            }
            return subject->unfinishedDangerCount() > 0 ? pxDanger : pxOff;
         }

         return QVariant();

      } else if (role == SortRole) {
         switch (column - numLangs) {
            case 0:
               // Context (same as display role)
               return mci->context().simplified();

            case 1:
               // Items
               return mci->getNumEditable();

            default:
               // Percent
               ContextItem *subject = mci->contextItem(column);

               if (subject != nullptr) {
                  int totalItems = subject->nonobsoleteCount();
                  int percent    = totalItems ? (100 * subject->finishedCount()) / totalItems : 100;
                  int rslt       = percent * (((1 << 28) - 1) / 100) + totalItems;

                  if (subject->isObsolete()) {
                     rslt |= (1 << 30);

                  } else if (subject->isFinished()) {
                     rslt |= (1 << 29);

                     if (! subject->finishedDangerCount()) {
                        rslt |= (1 << 28);
                     }

                  } else {
                     if (! subject->unfinishedDangerCount()) {
                        rslt |= (1 << 28);
                     }
                  }

                  return rslt;
               }

               return INT_MAX;
         }

      } else if (role == Qt::ForegroundRole && column >= numLangs && m_data->multiContextItem(row)->isObsolete()) {
         return QBrush(Qt::darkGray);

      } else if (role == Qt::ForegroundRole && column == numLangs && m_data->multiContextItem(row)->context().isEmpty()) {
         return QBrush(QColor(0, 0xa0, 0xa0));

      } else if (role == Qt::BackgroundRole) {
         if (column < numLangs && numLangs != 1) {
            QBrush brush = m_data->brushForModel(column);

            if (row & 1) {
               brush.setColor(brush.color().darker(108));
            }

            return brush;
         }
      }
   }

   return QVariant();
}

MultiDataIndex MessageModel::dataIndex(const QModelIndex &index, int model) const
{
   Q_ASSERT(index.isValid());
   Q_ASSERT(index.internalId());

   return MultiDataIndex(model, index.internalId() - 1, index.row());
}

