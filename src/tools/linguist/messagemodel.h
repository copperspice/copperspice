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

#ifndef MESSAGEMODEL_H
#define MESSAGEMODEL_H

#include <translator.h>

#include <qabstractitemmodel.h>
#include <qbitmap.h>
#include <qcolor.h>
#include <qhash.h>
#include <qlist.h>
#include <qlocale.h>
#include <qxmldefaulthandler.h>

class DataModel;
class MessageModel;
class MultiDataModel;

class MessageItem
{
 public:
   MessageItem(const TranslatorMessage &message);

   bool danger() const {
      return m_danger;
   }

   void setDanger(bool danger) {
      m_danger = danger;
   }

   void setTranslation(const QString &translation) {
      m_message.setTranslation(translation);
   }

   QString id() const {
      return m_message.id();
   }

   QString context() const {
      return m_message.context();
   }

   QString text() const {
      return m_message.sourceText();
   }

   QString pluralText() const {
      return m_message.extra("po-msgid_plural");
   }

   QString comment() const {
      return m_message.comment();
   }

   QString fileName() const {
      return m_message.fileName();
   }

   QString extraComment() const {
      return m_message.extraComment();
   }

   QString translatorComment() const {
      return m_message.translatorComment();
   }

   void setTranslatorComment(const QString &cmt) {
      m_message.setTranslatorComment(cmt);
   }

   int lineNumber() const {
      return m_message.lineNumber();
   }

   QString translation() const {
      return m_message.translation();
   }

   QStringList translations() const {
      return m_message.translations();
   }

   void setTranslations(const QStringList &translations) {
      m_message.setTranslations(translations);
   }

   TranslatorMessage::Type type() const {
      return m_message.type();
   }

   void setType(TranslatorMessage::Type type) {
      m_message.setType(type);
   }

   bool isFinished() const {
      return type() == TranslatorMessage::Type::Finished;
   }

   bool isObsolete() const {
      return type() == TranslatorMessage::Type::Obsolete || type() == TranslatorMessage::Type::Vanished;
   }

   const TranslatorMessage &message() const {
      return m_message;
   }

   bool compare(const QString &findText, bool matchSubstring, Qt::CaseSensitivity cs) const;

 private:
   TranslatorMessage m_message;
   bool m_danger;
};

class ContextItem
{
 public:
   ContextItem(const QString &context);

   int finishedDangerCount() const {
      return m_finishedDangerCount;
   }

   int unfinishedDangerCount() const {
      return m_unfinishedDangerCount;
   }

   int finishedCount() const {
      return m_finishedCount;
   }

   int unfinishedCount() const {
      return m_nonobsoleteCount - m_finishedCount;
   }

   int nonobsoleteCount() const {
      return m_nonobsoleteCount;
   }

   QString context() const {
      return m_context;
   }

   QString comment() const {
      return m_comment;
   }

   QString fullContext() const {
      return m_comment.trimmed();
   }

   // for item status in context list
   bool isObsolete() const {
      return !nonobsoleteCount();
   }

   bool isFinished() const {
      return unfinishedCount() == 0;
   }

   MessageItem *messageItem(int i) const;

   int messageCount() const {
      return msgItemList.count();
   }

   MessageItem *findMessage(const QString &sourcetext, const QString &comment) const;

 private:
   friend class DataModel;
   friend class MultiDataModel;
   void appendMessage(const MessageItem &msg) {
      msgItemList.append(msg);
   }

   void appendToComment(const QString &x);
   void incrementFinishedCount() {
      ++m_finishedCount;
   }
   void decrementFinishedCount() {
      --m_finishedCount;
   }

   void incrementFinishedDangerCount() {
      ++m_finishedDangerCount;
   }
   void decrementFinishedDangerCount() {
      --m_finishedDangerCount;
   }

   void incrementUnfinishedDangerCount() {
      ++m_unfinishedDangerCount;
   }
   void decrementUnfinishedDangerCount() {
      --m_unfinishedDangerCount;
   }

   void incrementNonobsoleteCount() {
      ++m_nonobsoleteCount;
   }

   QString m_comment;
   QString m_context;
   int m_finishedCount;
   int m_finishedDangerCount;
   int m_unfinishedDangerCount;
   int m_nonobsoleteCount;
   QList<MessageItem> msgItemList;
};

class DataIndex
{
 public:
   DataIndex()
      : m_context(-1), m_message(-1)
   {
   }

   DataIndex(int context, int message) : m_context(context), m_message(message) {}

   int context() const {
      return m_context;
   }

   int message() const {
      return m_message;
   }

   bool isValid() const {
      return m_context >= 0;
   }

 protected:
   int m_context;
   int m_message;
};

class DataModelIterator : public DataIndex
{
 public:
   DataModelIterator() = delete;
   DataModelIterator(DataModel *model, int contextNo = 0, int messageNo = 0);

   MessageItem *current() const;
   bool isValid() const;
   void operator++();

 private:
   DataModel *m_model;        // not owned
};

class DataModel : public QObject
{
   CS_OBJECT(DataModel)

 public:
   enum FindLocation {
      NoLocation   = 0,
      SourceText   = 0x1,
      Translations = 0x2,
      Comments     = 0x4
   };

   DataModel(QObject *parent = nullptr);

   // Specializations
   int contextCount() const {
      return m_contextList.count();
   }

   ContextItem *findContext(const QString &context) const;
   MessageItem *findMessage(const QString &context, const QString &sourcetext, const QString &comment) const;

   ContextItem *getContextItem(int index) const;
   MessageItem *getMessageItem(const DataIndex &index) const;

   int messageCount() const {
      return m_numMessages;
   }

   bool isEmpty() const {
      return m_numMessages == 0;
   }

   bool isModified() const {
      return m_modified;
   }

   void setModified(bool dirty);

   bool isWritable() const {
      return m_writable;
   }

   void setWritable(bool writable) {
      m_writable = writable;
   }

   bool isWellMergeable(const DataModel *other) const;

   bool load(const QString &fileName, bool *langGuessed, QWidget *parent, bool &waitCursor);

   bool save(QWidget *parent) {
      return save(m_srcFileName, parent);
   }

   bool saveAs(const QString &newFileName, QWidget *parent);
   bool release(const QString &fileName, bool verbose, bool ignoreUnfinished,
            TranslatorMessage::SaveMode mode, QWidget *parent);

   QString srcFileName(bool pretty = false) const {
      return pretty ? prettifyPlainFileName(m_srcFileName) : m_srcFileName;
   }

   static QString prettifyPlainFileName(const QString &fn);
   static QString prettifyFileName(const QString &fn);

   bool setLanguageAndCountry(QLocale::Language lang, QLocale::Country country);
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

   const QString &localizedLanguage() const {
      return m_localizedLanguage;
   }

   const QStringList &numerusForms() const {
      return m_numerusForms;
   }

   const QList<bool> &countRefNeeds() const {
      return m_countRefNeeds;
   }

   QStringList normalizedTranslations(const MessageItem &m) const;
   void doCharCounting(const QString &text, int &trW, int &trC, int &trCS);
   void updateStatistics();

   int getSrcWords() const {
      return m_srcWords;
   }

   int getSrcChars() const {
      return m_srcChars;
   }

   int getSrcCharsSpc() const {
      return m_srcCharsSpc;
   }

   CS_SIGNAL_1(Public, void statsChanged(int words,int characters,int cs,int words2,int characters2,int cs2))
   CS_SIGNAL_2(statsChanged,words,characters,cs,words2,characters2,cs2)

   CS_SIGNAL_1(Public, void progressChanged(int finishedCount,int oldFinishedCount))
   CS_SIGNAL_2(progressChanged,finishedCount,oldFinishedCount)

   CS_SIGNAL_1(Public, void languageChanged())
   CS_SIGNAL_2(languageChanged)

   CS_SIGNAL_1(Public, void modifiedChanged())
   CS_SIGNAL_2(modifiedChanged)

 private:
   friend class DataModelIterator;
   QList<ContextItem> m_contextList;

   bool save(const QString &fileName, QWidget *parent);
   void updateLocale();

   bool m_writable;
   bool m_modified;

   int m_numMessages;

   // For statistics
   int m_srcWords;
   int m_srcChars;
   int m_srcCharsSpc;

   QString m_srcFileName;
   QLocale::Language m_language;
   QLocale::Language m_sourceLanguage;
   QLocale::Country m_country;
   QLocale::Country m_sourceCountry;

   bool m_relativeLocations;

   QHash<QString, QString> m_extra;

   QString m_localizedLanguage;
   QStringList m_numerusForms;
   QList<bool> m_countRefNeeds;
};

struct MultiMessageItem
{
 public:
   MultiMessageItem(const MessageItem *m);

   QString id() const {
      return m_id;
   }

   QString text() const {
      return m_text;
   }

   QString pluralText() const {
      return m_pluralText;
   }

   QString comment() const {
      return m_comment;
   }

   bool isEmpty() const {
      return !m_nonnullCount;
   }

   bool isObsolete() const {
      return m_nonnullCount && !m_nonobsoleteCount;
   }

   int countNonobsolete() const {
      return m_nonobsoleteCount;
   }

   int countEditable() const {
      return m_editableCount;
   }

   bool isUnfinished() const {
      return m_unfinishedCount != 0;
   }

   int countUnfinished() const {
      return m_unfinishedCount;
   }

 private:
   friend class MultiDataModel;
   void incrementNonnullCount() {
      ++m_nonnullCount;
   }

   void decrementNonnullCount() {
      --m_nonnullCount;
   }

   void incrementNonobsoleteCount() {
      ++m_nonobsoleteCount;
   }

   void decrementNonobsoleteCount() {
      --m_nonobsoleteCount;
   }

   void incrementEditableCount() {
      ++m_editableCount;
   }

   void decrementEditableCount() {
      --m_editableCount;
   }

   void incrementUnfinishedCount() {
      ++m_unfinishedCount;
   }

   void decrementUnfinishedCount() {
      --m_unfinishedCount;
   }

   QString m_id;
   QString m_text;
   QString m_pluralText;
   QString m_comment;

   int m_nonnullCount;        // all
   int m_nonobsoleteCount;    // all
   int m_editableCount;       // read-write
   int m_unfinishedCount;     // read-write
};

struct MultiContextItem
{
 public:
   MultiContextItem(int oldCount, ContextItem *ctx, bool writable);

   ContextItem *contextItem(int model) const {
      return m_contextList[model];
   }

   MultiMessageItem *multiMessageItem(int msgIdx) const {
      return const_cast<MultiMessageItem *>(&m_multiMessageList[msgIdx]);
   }

   MessageItem *messageItem(int model, int msgIdx) const {
      return m_messageLists[model][msgIdx];
   }

   int firstNonobsoleteMessageIndex(int msgIdx) const;
   int findMessage(const QString &sourcetext, const QString &comment) const;
   int findMessageById(const QString &id) const;

   QString context() const {
      return m_context;
   }

   QString comment() const {
      return m_comment;
   }

   int messageCount() const {
      return m_messageLists.isEmpty() ? 0 : m_messageLists[0].count();
   }

   // For item count in context list
   int getNumFinished() const {
      return m_finishedCount;
   }

   int getNumEditable() const {
      return m_editableCount;
   }

   // For background in context list
   bool isObsolete() const {
      return messageCount() && ! m_nonobsoleteCount;
   }

 private:
   friend class MultiDataModel;
   void appendEmptyModel();
   void assignLastModel(ContextItem *ctx, bool writable);
   void removeModel(int pos);
   void moveModel(int oldPos, int newPos); // newPos is *before* removing at oldPos
   void putMessageItem(int pos, MessageItem *m);
   void appendMessageItems(const QList<MessageItem *> &m);
   void removeMultiMessageItem(int pos);

   void incrementFinishedCount() {
      ++m_finishedCount;
   }

   void decrementFinishedCount() {
      --m_finishedCount;
   }

   void incrementEditableCount() {
      ++m_editableCount;
   }

   void decrementEditableCount() {
      --m_editableCount;
   }

   void incrementNonobsoleteCount() {
      ++m_nonobsoleteCount;
   }

   void decrementNonobsoleteCount() {
      --m_nonobsoleteCount;
   }

   QString m_context;
   QString m_comment;
   QList<MultiMessageItem> m_multiMessageList;
   QList<ContextItem *> m_contextList;

   // The next two could be in the MultiMessageItems, but are here for efficiency
   QList<QList<MessageItem *> > m_messageLists;
   QList<QList<MessageItem *> *> m_writableMessageLists;
   int m_finishedCount; // read-write
   int m_editableCount; // read-write
   int m_nonobsoleteCount; // all (note: this counts messages, not multi-messages)
};

class MultiDataIndex
{
 public:
   MultiDataIndex()
      : m_model(-1), m_context(-1), m_message(-1)
   {
   }

   MultiDataIndex(int model, int context, int message)
      : m_model(model), m_context(context), m_message(message)
   {
   }

   void setModel(int model) {
      m_model = model;
   }

   int model() const {
      return m_model;
   }

   int context() const {
      return m_context;
   }

   int message() const {
      return m_message;
   }

   bool isValid() const {
      return m_context >= 0;
   }

   bool operator==(const MultiDataIndex &other) const {
      return m_model == other.m_model && m_context == other.m_context && m_message == other.m_message;
   }

   bool operator!=(const MultiDataIndex &other) const {
      return !(*this == other);
   }

 protected:
   int m_model;
   int m_context;
   int m_message;
};

class MultiDataModelIterator : public MultiDataIndex
{
 public:
   MultiDataModelIterator() = delete;
   MultiDataModelIterator(MultiDataModel *model, int modelNo, int contextNo = 0, int messageNo = 0);

   MessageItem *current() const;
   bool isValid() const;
   void operator++();

 private:
   MultiDataModel *m_dataModel;  // not owned
};

class MultiDataModel : public QObject
{
   CS_OBJECT(MultiDataModel)

 public:
   MultiDataModel(QObject *parent = nullptr);
   ~MultiDataModel();

   bool isWellMergeable(const DataModel *dm) const;
   void append(DataModel *dm, bool readWrite);

   bool save(int model, QWidget *parent) {
      return m_dataModels[model]->save(parent);
   }

   bool saveAs(int model, const QString &newFileName, QWidget *parent) {
      return m_dataModels[model]->saveAs(newFileName, parent);
   }

   bool release(int model, const QString &fileName, bool verbose, bool ignoreUnfinished,
               TranslatorMessage::SaveMode mode, QWidget *parent) {

      return m_dataModels[model]->release(fileName, verbose, ignoreUnfinished, mode, parent);
   }

   void close(int model);
   void closeAll();
   int isFileLoaded(const QString &name) const;

   // newPos is *before* removing at oldPos; note that this does not emit update signals
   void moveModel(int oldPos, int newPos);

   // Entire multi-model
   int modelCount() const {
      return m_dataModels.count();
   }

   int contextCount() const {
      return m_multiContextList.count();
   }

   int messageCount() const {
      return m_numMessages;
   }

   // Next two needed for progress indicator in main window
   int getNumFinished() const {
      return m_numFinished;
   }

   int getNumEditable() const {
      return m_numEditable;
   }

   bool isModified() const;
   QStringList srcFileNames(bool pretty = false) const;
   QString condensedSrcFileNames(bool pretty = false) const;

   // Per submodel
   QString srcFileName(int model, bool pretty = false) const {
      return m_dataModels[model]->srcFileName(pretty);
   }

   bool isModelWritable(int model) const {
      return m_dataModels[model]->isWritable();
   }

   bool isModified(int model) const {
      return m_dataModels[model]->isModified();
   }

   void setModified(int model, bool dirty) {
      m_dataModels[model]->setModified(dirty);
   }

   QLocale::Language language(int model) const {
      return m_dataModels[model]->language();
   }

   QLocale::Language sourceLanguage(int model) const {
      return m_dataModels[model]->sourceLanguage();
   }

   // Per message
   void setTranslation(const MultiDataIndex &index, const QString &translation);
   void setFinished(const MultiDataIndex &index, bool finished);
   void setDanger(const MultiDataIndex &index, bool danger);

   // Retrieve items
   DataModel *model(int i) {
      return m_dataModels[i];
   }

   MultiContextItem *multiContextItem(int ctxIdx) const {
      return const_cast<MultiContextItem *>(&m_multiContextList[ctxIdx]);
   }

   MultiMessageItem *multiMessageItem(const MultiDataIndex &index) const {
      return multiContextItem(index.context())->multiMessageItem(index.message());
   }

   MessageItem *getMessageItem(const MultiDataIndex &index, int model) const;

   MessageItem *getMessageItem(const MultiDataIndex &index) const {
      return getMessageItem(index, index.model());
   }

   static QString condenseFileNames(const QStringList &names);
   static QStringList prettifyFileNames(const QStringList &names);

   QBrush brushForModel(int model) const;

   CS_SIGNAL_1(Public, void modelAppended())
   CS_SIGNAL_2(modelAppended)

   CS_SIGNAL_1(Public, void modelDeleted(int model))
   CS_SIGNAL_2(modelDeleted,model)

   CS_SIGNAL_1(Public, void allModelsDeleted())
   CS_SIGNAL_2(allModelsDeleted)

   CS_SIGNAL_1(Public, void languageChanged(int model))
   CS_SIGNAL_2(languageChanged,model)

   CS_SIGNAL_1(Public, void statsChanged(int words,int characters,int cs,int words2,int characters2,int cs2))
   CS_SIGNAL_2(statsChanged,words,characters,cs,words2,characters2,cs2)

   CS_SIGNAL_1(Public, void modifiedChanged(bool changed))
   CS_SIGNAL_2(modifiedChanged, changed)

   CS_SIGNAL_1(Public, void multiContextDataChanged(const MultiDataIndex & index))
   CS_SIGNAL_2(multiContextDataChanged,index)

   CS_SIGNAL_1(Public, void contextDataChanged(const MultiDataIndex & index))
   CS_SIGNAL_2(contextDataChanged,index)

   CS_SIGNAL_1(Public, void messageDataChanged(const MultiDataIndex & index))
   CS_SIGNAL_2(messageDataChanged,index)

   CS_SIGNAL_1(Public, void translationChanged(const MultiDataIndex & index))
   CS_SIGNAL_2(translationChanged,index)  // Only the primary one

 private:
   CS_SLOT_1(Private, void onModifiedChanged())
   CS_SLOT_2(onModifiedChanged)

   CS_SLOT_1(Private, void onLanguageChanged())
   CS_SLOT_2(onLanguageChanged)

   ContextItem *getContextItem(const MultiDataIndex &index) const {
      return multiContextItem(index.context())->contextItem(index.model());
   }

   void updateCountsOnAdd(int model, bool writable);
   void updateCountsOnRemove(int model, bool writable);

   void incrementFinishedCount() {
      ++m_numFinished;
   }

   void decrementFinishedCount() {
      --m_numFinished;
   }

   void incrementEditableCount() {
      ++m_numEditable;
   }

   void decrementEditableCount() {
      --m_numEditable;
   }

   int findContextIndex(const QString &context) const;
   MultiContextItem *findContext(const QString &context) const;

   int m_numFinished;
   int m_numEditable;
   int m_numMessages;

   bool m_modified;

   QList<MultiContextItem> m_multiContextList;
   QList<DataModel *> m_dataModels;

   MessageModel *m_msgModel;

   QColor m_colors[7];
   QBitmap m_bitmap;

   friend class MultiDataModelIterator;
   friend class MessageModel;
};

class MessageModel : public QAbstractItemModel
{
   CS_OBJECT(MessageModel)

 public:
   enum { SortRole = Qt::UserRole };

   MessageModel(QObject *parent, MultiDataModel *data);

   // QAbstractItemModel
   QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
   QModelIndex parent(const QModelIndex &index) const override;
   int rowCount(const QModelIndex &parent = QModelIndex()) const override;
   int columnCount(const QModelIndex &parent = QModelIndex()) const override;
   QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

   // Convenience
   MultiDataIndex dataIndex(const QModelIndex &index, int model) const;

   MultiDataIndex dataIndex(const QModelIndex &index) const {
      return dataIndex(index, index.column() - 1 < m_data->modelCount() ? index.column() - 1 : -1);
   }

   QModelIndex modelIndex(const MultiDataIndex &index);

 private:
   MultiDataModel *m_data; // not owned

   CS_SLOT_1(Private, void multiContextItemChanged(const MultiDataIndex & index))
   CS_SLOT_2(multiContextItemChanged)

   CS_SLOT_1(Private, void contextItemChanged(const MultiDataIndex & index))
   CS_SLOT_2(contextItemChanged)

   CS_SLOT_1(Private, void messageItemChanged(const MultiDataIndex & index))
   CS_SLOT_2(messageItemChanged)

   friend class MultiDataModel;
};

#endif
