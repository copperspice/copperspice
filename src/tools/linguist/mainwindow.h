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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "phrase.h"
#include "ui_mainwindow.h"
#include "recentfiles.h"
#include "messagemodel.h"

#include <QtCore/QHash>
#include <QtCore/QLocale>

#include <QtGui/QMainWindow>

QT_BEGIN_NAMESPACE

class QPixmap;
class QAction;
class QDialog;
class QLabel;
class QMenu;
class QPrinter;
class QProcess;
class QIcon;
class QSortFilterProxyModel;
class QStackedWidget;
class QTableView;
class QTreeView;

class BatchTranslationDialog;
class ErrorsView;
class FindDialog;
class FocusWatcher;
class FormPreviewView;
class MessageEditor;
class PhraseView;
class SourceCodeView;
class Statistics;
class TranslateDialog;
class TranslationSettingsDialog;

class MainWindow : public QMainWindow
{
   Q_OBJECT
 public:
   enum {PhraseCloseMenu, PhraseEditMenu, PhrasePrintMenu};

   MainWindow();
   ~MainWindow();

   bool openFiles(const QStringList &names, bool readWrite = true);
   static RecentFiles &recentFiles();
   static const QString &resourcePrefix();
   static QString friendlyString(const QString &str);

 protected:
   void readConfig();
   void writeConfig();
   void closeEvent(QCloseEvent *);
   bool eventFilter(QObject *object, QEvent *event);

 private slots:
   void doneAndNext();
   void prev();
   void next();
   void recentFileActivated(QAction *action);
   void setupRecentFilesMenu();
   void open();
   void openAux();
   void saveAll();
   void save();
   void saveAs();
   void releaseAll();
   void release();
   void releaseAs();
   void print();
   void closeFile();
   bool closeAll();
   void findAgain();
   void showTranslateDialog();
   void showBatchTranslateDialog();
   void showTranslationSettings();
   void updateTranslateHit(bool &hit);
   void translate(int mode);
   void newPhraseBook();
   void openPhraseBook();
   void closePhraseBook(QAction *action);
   void editPhraseBook(QAction *action);
   void printPhraseBook(QAction *action);
   void addToPhraseBook();
   void manual();
   void resetSorting();
   void about();
   void aboutQt();

   void updateViewMenu();
   void fileAboutToShow();
   void editAboutToShow();

   void showContextDock();
   void showMessagesDock();
   void showPhrasesDock();
   void showSourceCodeDock();
   void showErrorDock();

   void setupPhrase();
   bool maybeSaveAll();
   bool maybeSave(int model);
   void updateProgress();
   void maybeUpdateStatistics(const MultiDataIndex &);
   void translationChanged(const MultiDataIndex &);
   void updateCaption();
   void updateLatestModel(const QModelIndex &index);
   void selectedContextChanged(const QModelIndex &sortedIndex, const QModelIndex &oldIndex);
   void selectedMessageChanged(const QModelIndex &sortedIndex, const QModelIndex &oldIndex);

   // To synchronize from the message editor to the model ...
   void updateTranslation(const QStringList &translations);
   void updateTranslatorComment(const QString &comment);

   void updateActiveModel(int);
   void refreshItemViews();
   void toggleFinished(const QModelIndex &index);
   void prevUnfinished();
   void nextUnfinished();
   void findNext(const QString &text, DataModel::FindLocation where, bool matchCase, bool ignoreAccelerators);
   void revalidate();
   void toggleStatistics();
   void onWhatsThis();
   void updatePhraseDicts();
   void updatePhraseDict(int model);

 private:
   QModelIndex nextContext(const QModelIndex &index) const;
   QModelIndex prevContext(const QModelIndex &index) const;
   QModelIndex nextMessage(const QModelIndex &currentIndex, bool checkUnfinished = false) const;
   QModelIndex prevMessage(const QModelIndex &currentIndex, bool checkUnfinished = false) const;
   bool next(bool checkUnfinished);
   bool prev(bool checkUnfinished);

   void updateStatistics();
   void initViewHeaders();
   void modelCountChanged();
   void setupMenuBar();
   void setupToolBars();
   void setCurrentMessage(const QModelIndex &index);
   void setCurrentMessage(const QModelIndex &index, int model);
   QModelIndex setMessageViewRoot(const QModelIndex &index);
   QModelIndex currentContextIndex() const;
   QModelIndex currentMessageIndex() const;
   PhraseBook *openPhraseBook(const QString &name);
   bool isPhraseBookOpen(const QString &name);
   bool savePhraseBook(QString *name, PhraseBook &pb);
   bool maybeSavePhraseBook(PhraseBook *phraseBook);
   bool maybeSavePhraseBooks();
   QStringList pickTranslationFiles();
   void showTranslationSettings(int model);
   void updateLatestModel(int model);
   void updatePhraseBookActions();
   void updatePhraseDictInternal(int model);
   void releaseInternal(int model);
   void saveInternal(int model);

   QPrinter *printer();

   // FIXME: move to DataModel
   void updateDanger(const MultiDataIndex &index, bool verbose);

   bool searchItem(const QString &searchWhat);

   QProcess *m_assistantProcess;
   QTreeView *m_contextView;
   QTreeView *m_messageView;
   MultiDataModel *m_dataModel;
   MessageModel *m_messageModel;
   QSortFilterProxyModel *m_sortedContextsModel;
   QSortFilterProxyModel *m_sortedMessagesModel;
   MessageEditor *m_messageEditor;
   PhraseView *m_phraseView;
   QStackedWidget *m_sourceAndFormView;
   SourceCodeView *m_sourceCodeView;
   FormPreviewView *m_formPreviewView;
   ErrorsView *m_errorsView;
   QLabel *m_progressLabel;
   QLabel *m_modifiedLabel;
   FocusWatcher *m_focusWatcher;
   QString m_phraseBookDir;
   // model : keyword -> list of appropriate phrases in the phrasebooks
   QList<QHash<QString, QList<Phrase *> > > m_phraseDict;
   QList<PhraseBook *> m_phraseBooks;
   QMap<QAction *, PhraseBook *> m_phraseBookMenu[3];
   QPrinter *m_printer;

   FindDialog *m_findDialog;
   QString m_findText;
   Qt::CaseSensitivity m_findMatchCase;
   bool m_findIgnoreAccelerators;
   DataModel::FindLocation m_findWhere;
   DataModel::FindLocation m_foundWhere;

   TranslateDialog *m_translateDialog;
   QString m_latestFindText;
   int m_latestCaseSensitivity;
   int m_remainingCount;
   int m_hitCount;

   BatchTranslationDialog *m_batchTranslateDialog;
   TranslationSettingsDialog *m_translationSettingsDialog;

   bool m_settingCurrentMessage;
   int m_fileActiveModel;
   int m_editActiveModel;
   MultiDataIndex m_currentIndex;

   QDockWidget *m_contextDock;
   QDockWidget *m_messagesDock;
   QDockWidget *m_phrasesDock;
   QDockWidget *m_sourceAndFormDock;
   QDockWidget *m_errorsDock;

   Ui::MainWindow m_ui;    // menus and actions
   Statistics *m_statistics;
};

QT_END_NAMESPACE

#endif
