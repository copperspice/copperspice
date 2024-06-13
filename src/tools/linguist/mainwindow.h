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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <ui_mainwindow.h>

#include <messagemodel.h>
#include <phrase.h>
#include <recentfiles.h>

#include <qhash.h>
#include <qlocale.h>
#include <qmainwindow.h>

#include <optional>

class QAction;
class QDialog;
class QIcon;
class QLabel;
class QMenu;
class QPixmap;
class QPrinter;
class QProcess;
class QSortFilterProxyModel;
class QStackedWidget;
class QTableView;
class QTreeView;

class BatchTranslationDialog;
class ErrorsView;
class FindDialog;
class FocusWatcher;
class MessageEditor;
class PhraseView;
class SettingsDialog;
class SourceCodeView;
class Statistics;
class TranslateDialog;

// class FormPreviewView;

class MainWindow : public QMainWindow
{
   CS_OBJECT(MainWindow)

 public:
   enum { PhraseCloseMenu, PhraseEditMenu, PhrasePrintMenu};

   MainWindow();
   ~MainWindow();

   bool openFiles(const QStringList &names, bool readWrite = true);
   static RecentFiles &recentFiles();
   static const QString &resourcePrefix();
   static QString friendlyString(const QString &str);

 protected:
   void readConfig();
   void writeConfig();
   void closeEvent(QCloseEvent *) override;
   bool eventFilter(QObject *object, QEvent *event) override;

 private:
   // file
   CS_SLOT_1(Private, void open())
   CS_SLOT_2(open)

   CS_SLOT_1(Private, void openAux())
   CS_SLOT_2(openAux)

   CS_SLOT_1(Private, void recentFileActivated(QAction * action))
   CS_SLOT_2(recentFileActivated)

   CS_SLOT_1(Private, void saveAll())
   CS_SLOT_2(saveAll)

   CS_SLOT_1(Private, void save())
   CS_SLOT_2(save)

   CS_SLOT_1(Private, void saveAs())
   CS_SLOT_2(saveAs)

   CS_SLOT_1(Private, void releaseAll())
   CS_SLOT_2(releaseAll)

   CS_SLOT_1(Private, void release())
   CS_SLOT_2(release)

   CS_SLOT_1(Private, void releaseAs())
   CS_SLOT_2(releaseAs)

   CS_SLOT_1(Private, void print())
   CS_SLOT_2(print)

   CS_SLOT_1(Private, void closeFile())
   CS_SLOT_2(closeFile)

   CS_SLOT_1(Private, bool closeAll())
   CS_SLOT_2(closeAll)

   // edit
   CS_SLOT_1(Private, void findAgain())
   CS_SLOT_2(findAgain)

   CS_SLOT_1(Private, void batchTranslateDialog())
   CS_SLOT_2(batchTranslateDialog)

   CS_SLOT_1(Private, void searchTranslateDialog())
   CS_SLOT_2(searchTranslateDialog)

   // translations
   CS_SLOT_1(Private, bool previous(bool checkUnfinished = false))
   CS_SLOT_2(previous)

   CS_SLOT_1(Private, bool next(bool checkUnfinished = false))
   CS_SLOT_2(next)

   CS_SLOT_1(Private, void doneAndNext())
   CS_SLOT_2(doneAndNext)

   CS_SLOT_1(Private, void setupRecentFilesMenu())
   CS_SLOT_2(setupRecentFilesMenu)

   CS_SLOT_1(Private, void updateTranslateHit(bool & hit))
   CS_SLOT_2(updateTranslateHit)

   CS_SLOT_1(Private, void translate(int mode))
   CS_SLOT_2(translate)

   // phrases
   CS_SLOT_1(Private, void newPhraseBook())
   CS_SLOT_2(newPhraseBook)

   CS_SLOT_1(Private, void selectPhraseBook())
   CS_SLOT_2(selectPhraseBook)

   CS_SLOT_1(Private, void closePhraseBook(QAction * action))
   CS_SLOT_2(closePhraseBook)

   CS_SLOT_1(Private, void editPhraseBook(QAction * action))
   CS_SLOT_2(editPhraseBook)

   CS_SLOT_1(Private, void printPhraseBook(QAction * action))
   CS_SLOT_2(printPhraseBook)

   CS_SLOT_1(Private, void addToPhraseBook())
   CS_SLOT_2(addToPhraseBook)

   // view
   CS_SLOT_1(Private, void resetSorting())
   CS_SLOT_2(resetSorting)

   CS_SLOT_1(Private, void settingsDialog(std::optional<int> model = std::optional<int>()))
   CS_SLOT_2(settingsDialog)

   // help
   CS_SLOT_1(Private, void manual())
   CS_SLOT_2(manual)

   CS_SLOT_1(Private, void about())
   CS_SLOT_2(about)

   //
   CS_SLOT_1(Private, void updateViewMenu())
   CS_SLOT_2(updateViewMenu)

   CS_SLOT_1(Private, void fileAboutToShow())
   CS_SLOT_2(fileAboutToShow)

   CS_SLOT_1(Private, void editAboutToShow())
   CS_SLOT_2(editAboutToShow)

   CS_SLOT_1(Private, void showContextDock())
   CS_SLOT_2(showContextDock)

   CS_SLOT_1(Private, void showMessagesDock())
   CS_SLOT_2(showMessagesDock)

   CS_SLOT_1(Private, void showPhrasesDock())
   CS_SLOT_2(showPhrasesDock)

   CS_SLOT_1(Private, void showSourceCodeDock())
   CS_SLOT_2(showSourceCodeDock)

   CS_SLOT_1(Private, void showErrorDock())
   CS_SLOT_2(showErrorDock)

   CS_SLOT_1(Private, void setupPhrase())
   CS_SLOT_2(setupPhrase)

   CS_SLOT_1(Private, bool maybeSaveAll())
   CS_SLOT_2(maybeSaveAll)

   CS_SLOT_1(Private, bool maybeSave(int model))
   CS_SLOT_2(maybeSave)

   CS_SLOT_1(Private, void updateProgress())
   CS_SLOT_2(updateProgress)

   CS_SLOT_1(Private, void maybeUpdateStatistics(const MultiDataIndex &index))
   CS_SLOT_2(maybeUpdateStatistics)

   CS_SLOT_1(Private, void translationChanged(const MultiDataIndex &index))
   CS_SLOT_2(translationChanged)

   CS_SLOT_1(Private, void updateCaption())
   CS_SLOT_2(updateCaption)

   CS_SLOT_1(Private, void updateModelIndex(const QModelIndex &index))
   CS_SLOT_2(updateModelIndex)

   CS_SLOT_1(Private, void selectedContextChanged(const QModelIndex &sortedIndex,const QModelIndex &oldIndex))
   CS_SLOT_2(selectedContextChanged)

   CS_SLOT_1(Private, void selectedMessageChanged(const QModelIndex &sortedIndex,const QModelIndex &oldIndex))
   CS_SLOT_2(selectedMessageChanged)

   // To synchronize from the message editor to the model
   CS_SLOT_1(Private, void updateTranslation(const QStringList & translations))
   CS_SLOT_2(updateTranslation)

   CS_SLOT_1(Private, void updateTranslatorComment(const QString & comment))
   CS_SLOT_2(updateTranslatorComment)

   CS_SLOT_1(Private, void updateActiveModel(int model))
   CS_SLOT_2(updateActiveModel)

   CS_SLOT_1(Private, void refreshItemViews())
   CS_SLOT_2(refreshItemViews)

   CS_SLOT_1(Private, void toggleFinished(const QModelIndex &index))
   CS_SLOT_2(toggleFinished)

   CS_SLOT_1(Private, void prevUnfinished())
   CS_SLOT_2(prevUnfinished)

   CS_SLOT_1(Private, void nextUnfinished())
   CS_SLOT_2(nextUnfinished)

   CS_SLOT_1(Private, void findNext(const QString &text, DataModel::FindLocation where, bool matchCase, bool ignoreAccelerators, bool skipObsolete))
   CS_SLOT_2(findNext)

   CS_SLOT_1(Private, void revalidate())
   CS_SLOT_2(revalidate)

   CS_SLOT_1(Private, void toggleStatistics())
   CS_SLOT_2(toggleStatistics)

   CS_SLOT_1(Private, void toggleVisualizeWhitespace())
   CS_SLOT_2(toggleVisualizeWhitespace)

   CS_SLOT_1(Private, void updatePhraseDicts())
   CS_SLOT_2(updatePhraseDicts)

   CS_SLOT_1(Private, void updatePhraseDict(int model))
   CS_SLOT_2(updatePhraseDict)

   QModelIndex nextContext(const QModelIndex &index) const;
   QModelIndex prevContext(const QModelIndex &index) const;

   QModelIndex nextMessage(const QModelIndex &currentIndex, bool checkUnfinished = false) const;
   QModelIndex prevMessage(const QModelIndex &currentIndex, bool checkUnfinished = false) const;

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

   void updateLatestModel(int model);
   void updateSourceView(int model, MessageItem *item);
   void updatePhraseBookActions();
   void updatePhraseDictInternal(int model);
   void releaseInternal(int model);
   void saveInternal(int model);

   QPrinter *printer();

   // may want to move to DataModel
   void updateDanger(const MultiDataIndex &index, bool verbose);

   bool searchItem(DataModel::FindLocation where, const QString &searchWhat);

   QProcess  *m_assistantProcess;
   QTreeView *m_contextView;
   QTreeView *m_messageView;
   QPrinter  *m_printer;

   MultiDataModel *m_dataModel;
   MessageModel   *m_messageModel;
   MessageEditor  *m_messageEditor;
   PhraseView     *m_phraseView;
   SourceCodeView *m_sourceCodeView;
   FocusWatcher   *m_focusWatcher;

   QDockWidget *m_contextDock;
   QDockWidget *m_messagesDock;
   QDockWidget *m_phrasesDock;
   QDockWidget *m_sourceAndFormDock;
   QDockWidget *m_errorsDock;

   QStackedWidget *m_sourceAndFormView;

   QSortFilterProxyModel *m_sortedContextsModel;
   QSortFilterProxyModel *m_sortedMessagesModel;

   // FormPreviewView *m_formPreviewView;
   ErrorsView *m_errorsView;
   QLabel *m_progressLabel;
   QLabel *m_modifiedLabel;

   QString m_phraseBookDir;

   // model : keyword -> list of appropriate phrases in the phrasebooks
   QList<QHash<QString, QList<Phrase *> > > m_phraseDict;
   QList<PhraseBook *> m_phraseBooks;
   QMap<QAction *, PhraseBook *> m_phraseBookMenu[3];

   FindDialog *m_findDialog;
   QString m_findText;
   Qt::CaseSensitivity m_findMatchCase;
   bool m_findIgnoreAccelerators;
   bool m_findSkipObsolete;

   DataModel::FindLocation m_findWhere;

   TranslateDialog *m_translateDialog;
   QString m_latestFindText;
   int m_latestCaseSensitivity;
   int m_remainingCount;
   int m_hitCount;

   BatchTranslationDialog *m_batchTranslateDialog;
   SettingsDialog *m_settingsDialog;

   bool m_settingCurrentMessage;
   int m_fileActiveModel;
   int m_editActiveModel;
   MultiDataIndex m_currentIndex;

   Ui::MainWindow m_ui;         // menus and actions
   Statistics *m_statistics;
};

#endif
