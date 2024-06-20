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

#include <qcocoafiledialoghelper.h>

#ifndef QT_NO_FILEDIALOG

#include <qapplication.h>
#include <qcocoahelpers.h>
#include <qcocoamenubar.h>
#include <qcocoaeventdispatcher.h>
#include <qbuffer.h>
#include <qdebug.h>
#include <qstringlist.h>
#include <qtextcodec.h>
#include <qvarlengtharray.h>
#include <qabstracteventdispatcher.h>
#include <qdir.h>
#include <qplatform_nativeinterface.h>
#include <qregularexpression.h>

#include <qapplication_p.h>
#include <qt_mac_p.h>

#include <stdlib.h>

#import <AppKit/NSSavePanel.h>
#import <CoreFoundation/CFNumber.h>

class QFileInfo;
class QWindow;

@interface QNSOpenSavePanelDelegate
   : NSObject<NSOpenSavePanelDelegate>
{
 @public
   NSOpenPanel *mOpenPanel;
   NSSavePanel *mSavePanel;
   NSView *mAccessoryView;
   NSPopUpButton *mPopUpButton;
   NSTextField *mTextField;
   QCocoaFileDialogHelper *mHelper;
   NSString *mCurrentDir;

   int mReturnCode;

   QSharedPointer<QPlatformFileDialogOptions> mOptions;

   QString     mCurrentSelection;
   QStringList *mNameFilterDropDownList;
   QStringList *mSelectedNameFilter;
}

- (NSString *)strip: (const QString &)label;
- (BOOL)panel: (id)sender shouldShowFilename: (NSString *)filename;
- (void)filterChanged: (id)sender;
- (void)showModelessPanel;
- (BOOL)runApplicationModalPanel;
- (void)showWindowModalSheet: (QWindow *)docWidget;
- (void)updateProperties;
- (QStringList)acceptableExtensionsForSave;
- (QString)removeExtensions: (const QString &)filter;
- (void)createTextField;
- (void)createPopUpButton: (const QString &)selectedFilter hideDetails: (BOOL)hideDetails;
- (QStringList)findStrippedFilterWithVisualFilterName: (QString)name;
- (void)createAccessory;

@end


@implementation QNSOpenSavePanelDelegate

- (id)initWithAcceptMode:
   (const QString &)selectFile
                 options: (QSharedPointer<QPlatformFileDialogOptions>)options
                  helper: (QCocoaFileDialogHelper *)helper
{
   self = [super init];
   mOptions = options;

   if (mOptions->acceptMode() == QPlatformFileDialogOptions::AcceptOpen) {
      mOpenPanel = [NSOpenPanel openPanel];
      mSavePanel = mOpenPanel;
   } else {
      mSavePanel = [NSSavePanel savePanel];
      [mSavePanel setCanSelectHiddenExtension: YES];
      mOpenPanel = nullptr;
   }

   if ([mSavePanel respondsToSelector: @selector(setLevel:)]) {
      [mSavePanel setLevel: NSModalPanelWindowLevel];
   }

   mReturnCode = -1;
   mHelper = helper;
   mNameFilterDropDownList = new QStringList(mOptions->nameFilters());
   QString selectedVisualNameFilter = mOptions->initiallySelectedNameFilter();
   mSelectedNameFilter = new QStringList([self findStrippedFilterWithVisualFilterName: selectedVisualNameFilter]);

   QFileInfo sel(selectFile);
   if (sel.isDir() && ! sel.isBundle()) {
      mCurrentDir = [QCFString::toNSString(sel.absoluteFilePath()) retain];
      mCurrentSelection.clear();

   } else {
      mCurrentDir = [QCFString::toNSString(sel.absolutePath()) retain];
      mCurrentSelection = QString(sel.absoluteFilePath());
   }

   [mSavePanel setTitle: QCFString::toNSString(options->windowTitle())];
   [self createPopUpButton: selectedVisualNameFilter
               hideDetails: options->testOption(QFileDialog::FileDialogOption::HideNameFilterDetails)];

   [self createTextField];
   [self createAccessory];
   [mSavePanel setAccessoryView: mNameFilterDropDownList->size() > 1 ? mAccessoryView : nil];

   // -setAccessoryView: can result in -panel:directoryDidChange:
   // resetting our mCurrentDir, set the delegate
   // here to make sure it gets the correct value.
   [mSavePanel setDelegate: self];

   if (mOptions->isLabelExplicitlySet(QPlatformFileDialogOptions::Accept)) {
      [mSavePanel setPrompt: [self strip: options->labelText(QPlatformFileDialogOptions::Accept)]];
   }

   if (mOptions->isLabelExplicitlySet(QPlatformFileDialogOptions::FileName)) {
      [mSavePanel setNameFieldLabel: [self strip: options->labelText(QPlatformFileDialogOptions::FileName)]];
   }

   [self updateProperties];
   [mSavePanel retain];
   return self;
}

- (void)dealloc
{
   delete mNameFilterDropDownList;
   delete mSelectedNameFilter;

   if ([mSavePanel respondsToSelector: @selector(orderOut:)]) {
      [mSavePanel orderOut: mSavePanel];
   }

   [mSavePanel setAccessoryView: nil];
   [mPopUpButton release];
   [mTextField release];
   [mAccessoryView release];
   [mSavePanel setDelegate: nil];
   [mSavePanel release];
   [mCurrentDir release];
   [super dealloc];
}

static QString strippedText(QString s)
{
   s.remove( QString::fromLatin1("...") );
   return qt_mac_removeMnemonics(s).trimmed();
}

- (NSString *)strip: (const QString &)label
{
   return QCFString::toNSString(strippedText(label));
}

- (void)closePanel
{
   mCurrentSelection = QCFString::toQString([[mSavePanel URL] path]).normalized(QString::NormalizationForm_C);

   if ([mSavePanel respondsToSelector: @selector(close)]) {
      [mSavePanel close];
   }
   if ([mSavePanel isSheet]) {
      [NSApp endSheet: mSavePanel];
   }
}

- (void)showModelessPanel
{
   if (mOpenPanel) {
      QFileInfo info(mCurrentSelection);

      NSString *filepath = QCFString::toNSString(info.filePath());
      bool selectable = (mOptions->acceptMode() == QPlatformFileDialogOptions::AcceptSave)
         || [self panel: nil shouldShowFilename: filepath];

      [self updateProperties];

      QCocoaMenuBar::redirectKnownMenuItemsToFirstResponder();
      [mOpenPanel setAllowedFileTypes: nil];
      [mSavePanel setNameFieldStringValue: selectable ? QT_PREPEND_NAMESPACE(QCFString::toNSString)(info.fileName()) : @""];

      [mOpenPanel beginWithCompletionHandler: ^ (NSInteger result) {
                    mReturnCode = result;
                    if (mHelper) {
            mHelper->QNSOpenSavePanelDelegate_panelClosed(result == NSModalResponseOK);
         }
      }];
   }
}

- (BOOL)runApplicationModalPanel
{
   QFileInfo info(mCurrentSelection);

   NSString *filepath = QCFString::toNSString(info.filePath());
   bool selectable = (mOptions->acceptMode() == QPlatformFileDialogOptions::AcceptSave)
      || [self panel: nil shouldShowFilename: filepath];

   [mSavePanel setDirectoryURL: [NSURL fileURLWithPath: mCurrentDir]];
   [mSavePanel setNameFieldStringValue: selectable ? QCFString::toNSString(info.fileName()) : @""];

   // Call processEvents in case the event dispatcher has been interrupted, and needs to do
   // cleanup of modal sessions. Do this before showing the native dialog, otherwise it will
   // close down during the cleanup.
   qApp->processEvents(QEventLoop::ExcludeUserInputEvents | QEventLoop::ExcludeSocketNotifiers);

   // Make sure we don't interrupt the runModal call below.
   QCocoaEventDispatcher::clearCurrentThreadCocoaEventDispatcherInterruptFlag();

   QCocoaMenuBar::redirectKnownMenuItemsToFirstResponder();
   mReturnCode = [mSavePanel runModal];
   QCocoaMenuBar::resetKnownMenuItemsToQt();

   QAbstractEventDispatcher::instance()->interrupt();
   return (mReturnCode == NSModalResponseOK);
}

- (QPlatformDialogHelper::DialogCode)dialogResultCode
{
   return (mReturnCode == NSModalResponseOK) ? QPlatformDialogHelper::Accepted : QPlatformDialogHelper::Rejected;
}

- (void)showWindowModalSheet: (QWindow *)parent
{
   QFileInfo info(mCurrentSelection);

   NSString *filepath = QCFString::toNSString(info.filePath());
   bool selectable = (mOptions->acceptMode() == QPlatformFileDialogOptions::AcceptSave)
      || [self panel: nil shouldShowFilename: filepath];

   [self updateProperties];
   QCocoaMenuBar::redirectKnownMenuItemsToFirstResponder();
   [mSavePanel setDirectoryURL: [NSURL fileURLWithPath: mCurrentDir]];

   [mSavePanel setNameFieldStringValue: selectable ? QCFString::toNSString(info.fileName()) : @""];
   NSWindow *nsparent = static_cast<NSWindow *>(qGuiApp->platformNativeInterface()->nativeResourceForWindow("nswindow", parent));

   [mSavePanel beginSheetModalForWindow: nsparent completionHandler: ^ (NSInteger result) {
                 mReturnCode = result;

                 if (mHelper) {
         mHelper->QNSOpenSavePanelDelegate_panelClosed(result == NSModalResponseOK);
      }
   }];
}

- (BOOL)isHiddenFile: (NSString *)filename isDir: (BOOL)isDir
{
   CFURLRef url = CFURLCreateWithFileSystemPath(kCFAllocatorDefault, (CFStringRef)filename, kCFURLPOSIXPathStyle, isDir);
   CFBooleanRef isHidden;
   Boolean errorOrHidden = false;

   if (!CFURLCopyResourcePropertyForKey(url, kCFURLIsHiddenKey, &isHidden, nullptr)) {
      errorOrHidden = true;
   } else {
      if (CFBooleanGetValue(isHidden)) {
         errorOrHidden = true;
      }
      CFRelease(isHidden);
   }

   CFRelease(url);
   return errorOrHidden;
}

- (BOOL)panel: (id)sender shouldShowFilename: (NSString *)filename
{
   (void) sender;

   if ([filename length] == 0) {
      return NO;
   }

   // Always accept directories regardless of their names (unless it is a bundle):
   NSFileManager *fm = [NSFileManager defaultManager];
   NSDictionary *fileAttrs = [fm attributesOfItemAtPath: filename error: nil];
   if (!fileAttrs) {
      return NO;   // Error accessing the file means 'no'.
   }
   NSString *fileType = [fileAttrs fileType];
   bool isDir = [fileType isEqualToString: NSFileTypeDirectory];
   if (isDir) {
      if ([mSavePanel treatsFilePackagesAsDirectories] == NO) {
         if ([[NSWorkspace sharedWorkspace] isFilePackageAtPath: filename] == NO) {
            return YES;
         }
      }
   }

   QString qtFileName = QFileInfo(QCFString::toQString(filename)).fileName();
   // No filter means accept everything
   bool nameMatches = mSelectedNameFilter->isEmpty();
   // Check if the current file name filter accepts the file:
   for (int i = 0; !nameMatches && i < mSelectedNameFilter->size(); ++i) {
      if (QDir::match(mSelectedNameFilter->at(i), qtFileName)) {
         nameMatches = true;
      }
   }
   if (!nameMatches) {
      return NO;
   }

   QDir::Filters filter = mOptions->filter();
   if ((!(filter & (QDir::Dirs | QDir::AllDirs)) && isDir)
      || (!(filter & QDir::Files) && [fileType isEqualToString: NSFileTypeRegular])
      || ((filter & QDir::NoSymLinks) && [fileType isEqualToString: NSFileTypeSymbolicLink])) {
      return NO;
   }

   bool filterPermissions = ((filter & QDir::PermissionMask)
         && (filter & QDir::PermissionMask) != QDir::PermissionMask);
   if (filterPermissions) {
      if ((!(filter & QDir::Readable) && [fm isReadableFileAtPath: filename])
         || (!(filter & QDir::Writable) && [fm isWritableFileAtPath: filename])
         || (!(filter & QDir::Executable) && [fm isExecutableFileAtPath: filename])) {
         return NO;
      }
   }
   if (!(filter & QDir::Hidden)
      && (qtFileName.startsWith(QLatin1Char('.')) || [self isHiddenFile: filename isDir: isDir])) {
      return NO;
   }

   return YES;
}

- (NSString *)panel: (id)sender userEnteredFilename: (NSString *)filename confirmed: (BOOL)okFlag
{
   (void) sender;
   if (!okFlag) {
      return filename;
   }

   if (!mOptions->testOption(QFileDialog::FileDialogOption::DontConfirmOverwrite)) {
      return filename;
   }

   // User has clicked save, and no overwrite confirmation should occur.
   // To get the latter, we need to change the name we return (hence the prefix):
   return [@"___qt_very_unlikely_prefix_" stringByAppendingString: filename];
}

- (void)setNameFilters: (const QStringList &)filters hideDetails: (BOOL)hideDetails
{
   [mPopUpButton removeAllItems];
   *mNameFilterDropDownList = filters;
   if (filters.size() > 0) {
      for (int i = 0; i < filters.size(); ++i) {
         QString filter = hideDetails ? [self removeExtensions: filters.at(i)] : filters.at(i);
         [mPopUpButton addItemWithTitle: QCFString::toNSString(filter)];
      }
      [mPopUpButton selectItemAtIndex: 0];
      [mSavePanel setAccessoryView: mAccessoryView];
   } else {
      [mSavePanel setAccessoryView: nil];
   }

   [self filterChanged: self];
}

- (void)filterChanged: (id)sender
{
   // This mDelegate function is called when the _name_ filter changes.
   (void) sender;
   QString selection = mNameFilterDropDownList->value([mPopUpButton indexOfSelectedItem]);
   *mSelectedNameFilter = [self findStrippedFilterWithVisualFilterName: selection];
   if ([mSavePanel respondsToSelector: @selector(validateVisibleColumns:)]) {
      [mSavePanel validateVisibleColumns];
   }
   [self updateProperties];
   if (mHelper) {
      mHelper->QNSOpenSavePanelDelegate_filterSelected([mPopUpButton indexOfSelectedItem]);
   }
}

- (QString)currentNameFilter
{
   return mNameFilterDropDownList->value([mPopUpButton indexOfSelectedItem]);
}

- (QList<QUrl>)selectedFiles
{
   if (mOpenPanel) {
      QList<QUrl> result;
      NSArray *array = [mOpenPanel URLs];
      for (NSUInteger i = 0; i < [array count]; ++i) {
         QString path = QCFString::toQString([[array objectAtIndex: i] path]).normalized(QString::NormalizationForm_C);
         result << QUrl::fromLocalFile(path);
      }
      return result;
   } else {
      QList<QUrl> result;
      QString filename = QCFString::toQString([[mSavePanel URL] path]).normalized(QString::NormalizationForm_C);
      result << QUrl::fromLocalFile(filename.remove(QLatin1String("___qt_very_unlikely_prefix_")));
      return result;
   }
}

- (void)updateProperties
{
   // Call this functions if mFileMode, mFileOptions,
   // mNameFilterDropDownList or mQDirFilter changes.
   // The savepanel does not contain the neccessary functions for this.
   const QPlatformFileDialogOptions::FileMode fileMode = mOptions->fileMode();

   bool chooseFilesOnly = fileMode == QPlatformFileDialogOptions::ExistingFile
      || fileMode == QPlatformFileDialogOptions::ExistingFiles;

   bool chooseDirsOnly = fileMode == QPlatformFileDialogOptions::Directory
      || fileMode == QPlatformFileDialogOptions::DirectoryOnly
      || mOptions->testOption(QFileDialog::FileDialogOption::ShowDirsOnly);

   [mOpenPanel setCanChooseFiles: !chooseDirsOnly];
   [mOpenPanel setCanChooseDirectories: !chooseFilesOnly];
   [mSavePanel setCanCreateDirectories: !(mOptions->testOption(QFileDialog::FileDialogOption::ReadOnly))];
   [mOpenPanel setAllowsMultipleSelection: (fileMode == QPlatformFileDialogOptions::ExistingFiles)];
   [mOpenPanel setResolvesAliases: !(mOptions->testOption(QFileDialog::FileDialogOption::DontResolveSymlinks))];
   [mOpenPanel setTitle: QCFString::toNSString(mOptions->windowTitle())];
   [mSavePanel setTitle: QCFString::toNSString(mOptions->windowTitle())];
   [mPopUpButton setHidden: chooseDirsOnly];   // TODO hide the whole sunken pane instead?

   QStringList ext = [self acceptableExtensionsForSave];
   const QString defaultSuffix = mOptions->defaultSuffix();
   if (!ext.isEmpty() && !defaultSuffix.isEmpty()) {
      ext.prepend(defaultSuffix);
   }
   [mSavePanel setAllowedFileTypes: ext.isEmpty() ? nil : qt_mac_QStringListToNSMutableArray(ext)];

   if ([mSavePanel respondsToSelector: @selector(isVisible)] && [mSavePanel isVisible]) {
      if ([mSavePanel respondsToSelector: @selector(validateVisibleColumns)]) {
         [mSavePanel validateVisibleColumns];
      }
   }
}

- (void)panelSelectionDidChange: (id)sender
{
   if (mHelper && [mSavePanel isVisible]) {
      QString selection = QCFString::toQString([[mSavePanel URL] path]);

      if (selection != mCurrentSelection) {
         mCurrentSelection = selection;
         mHelper->QNSOpenSavePanelDelegate_selectionChanged(selection);
      }
   }
}

- (void)panel: (id)sender directoryDidChange: (NSString *)path
{
   if (! mHelper) {
      return;
   }

   if (! (path && path.length) || [path isEqualToString: mCurrentDir]) {
      return;
   }

   [mCurrentDir release];
   mCurrentDir = [path retain];
   mHelper->QNSOpenSavePanelDelegate_directoryEntered(QCFString::toQString(mCurrentDir));
}

/*
    Returns a list of extensions (e.g. "png", "jpg", "gif")
    for the current name filter. If a filter do not conform
    to the format *.xyz or * or *.*, an empty list
    is returned meaning accept everything.
*/
- (QStringList)acceptableExtensionsForSave
{
   QStringList result;

   for (int i = 0; i < mSelectedNameFilter->count(); ++i) {
      const QString &filter = mSelectedNameFilter->at(i);

      if (filter.startsWith("*.") && ! filter.contains('?') && filter.count('*') == 1) {
         result += filter.mid(2);
      } else {
         return QStringList(); // Accept everything
      }
   }

   return result;
}

- (QString)removeExtensions: (const QString &)filter
{
   QRegularExpression regExp(QPlatformFileDialogHelper::filterRegExp);
   QRegularExpressionMatch match = regExp.match(filter);

   if (match.hasMatch())  {
      return match.captured(1).trimmed();
   }

   return filter;
}

- (void)createTextField
{
   NSRect textRect = { { 0.0, 3.0 }, { 100.0, 25.0 } };
   mTextField = [[NSTextField alloc] initWithFrame: textRect];
   [[mTextField cell] setFont: [NSFont systemFontOfSize: [NSFont systemFontSizeForControlSize: NSControlSizeRegular]]];

   [mTextField setAlignment: NSTextAlignmentRight];
   [mTextField setEditable: false];
   [mTextField setSelectable: false];
   [mTextField setBordered: false];
   [mTextField setDrawsBackground: false];

   if (mOptions->isLabelExplicitlySet(QPlatformFileDialogOptions::FileType)) {
      [mTextField setStringValue: [self strip: mOptions->labelText(QPlatformFileDialogOptions::FileType)]];
   }
}

- (void)createPopUpButton: (const QString &)selectedFilter hideDetails: (BOOL)hideDetails
{
   NSRect popUpRect = { { 100.0, 5.0 }, { 250.0, 25.0 } };
   mPopUpButton = [[NSPopUpButton alloc] initWithFrame: popUpRect pullsDown: NO];
   [mPopUpButton setTarget: self];
   [mPopUpButton setAction: @selector(filterChanged:)];

   if (mNameFilterDropDownList->size() > 0) {
      int filterToUse = -1;
      for (int i = 0; i < mNameFilterDropDownList->size(); ++i) {
         QString currentFilter = mNameFilterDropDownList->at(i);
         if (selectedFilter == currentFilter ||
            (filterToUse == -1 && currentFilter.startsWith(selectedFilter))) {
            filterToUse = i;
         }
         QString filter = hideDetails ? [self removeExtensions: currentFilter] : currentFilter;
         [mPopUpButton addItemWithTitle: QCFString::toNSString(filter)];
      }
      if (filterToUse != -1) {
         [mPopUpButton selectItemAtIndex: filterToUse];
      }
   }
}

- (QStringList) findStrippedFilterWithVisualFilterName: (QString)name
{
   for (int i = 0; i < mNameFilterDropDownList->size(); ++i) {
      if (mNameFilterDropDownList->at(i).startsWith(name)) {
         return QPlatformFileDialogHelper::cleanFilterList(mNameFilterDropDownList->at(i));
      }
   }
   return QStringList();
}

- (void)createAccessory
{
   NSRect accessoryRect = { { 0.0, 0.0 }, { 450.0, 33.0 } };
   mAccessoryView = [[NSView alloc] initWithFrame: accessoryRect];
   [mAccessoryView addSubview: mTextField];
   [mAccessoryView addSubview: mPopUpButton];
}

@end

QCocoaFileDialogHelper::QCocoaFileDialogHelper()
   : mDelegate(nullptr)
{
}

QCocoaFileDialogHelper::~QCocoaFileDialogHelper()
{
   if (!mDelegate) {
      return;
   }
   QMacAutoReleasePool pool;
   [mDelegate release];
   mDelegate = nullptr;
}

void QCocoaFileDialogHelper::QNSOpenSavePanelDelegate_selectionChanged(const QString &newPath)
{
   emit currentChanged(QUrl::fromLocalFile(newPath));
}

void QCocoaFileDialogHelper::QNSOpenSavePanelDelegate_panelClosed(bool accepted)
{
   QCocoaMenuBar::resetKnownMenuItemsToQt();
   if (accepted) {
      emit accept();
   } else {
      emit reject();
   }
}

void QCocoaFileDialogHelper::QNSOpenSavePanelDelegate_directoryEntered(const QString &newDir)
{
   // ### fixme: priv->setLastVisitedDirectory(newDir);
   emit directoryEntered(QUrl::fromLocalFile(newDir));
}

void QCocoaFileDialogHelper::QNSOpenSavePanelDelegate_filterSelected(int menuIndex)
{
   const QStringList filters = options()->nameFilters();
   emit filterSelected(menuIndex >= 0 && menuIndex < filters.size() ? filters.at(menuIndex) : QString());
}

void QCocoaFileDialogHelper::setDirectory(const QUrl &directory)
{
   if (mDelegate) {
      [mDelegate->mSavePanel setDirectoryURL: [NSURL fileURLWithPath: QCFString::toNSString(directory.toLocalFile())]];
   } else {
      mDir = directory;
   }
}

QUrl QCocoaFileDialogHelper::directory() const
{
   if (mDelegate) {
      QString path = QCFString::toQString([[mDelegate->mSavePanel directoryURL] path]).normalized(QString::NormalizationForm_C);
      return QUrl::fromLocalFile(path);
   }

   return mDir;
}

void QCocoaFileDialogHelper::selectFile(const QUrl &filename)
{
   QString filePath = filename.toLocalFile();

   if (QDir::isRelativePath(filePath)) {
      filePath = QFileInfo(directory().toLocalFile(), filePath).filePath();
   }

   // There seems to no way to select a file once the dialog is running.
   // So do the next best thing, set the file's directory:

   QUrl dir = QUrl(QFileInfo(filePath).absolutePath());
   setDirectory(dir);
}

QList<QUrl> QCocoaFileDialogHelper::selectedFiles() const
{
   if (mDelegate) {
      return [mDelegate selectedFiles];
   }

   return QList<QUrl>();
}

void QCocoaFileDialogHelper::setFilter()
{
   if (! mDelegate) {
      return;
   }

   const QSharedPointer<QPlatformFileDialogOptions> &opts = options();
   [mDelegate->mSavePanel setTitle: QCFString::toNSString(opts->windowTitle())];

   if (opts->isLabelExplicitlySet(QPlatformFileDialogOptions::Accept)) {
      [mDelegate->mSavePanel setPrompt: [mDelegate strip: opts->labelText(QPlatformFileDialogOptions::Accept)]];
   }
   if (opts->isLabelExplicitlySet(QPlatformFileDialogOptions::FileName)) {
      [mDelegate->mSavePanel setNameFieldLabel: [mDelegate strip: opts->labelText(QPlatformFileDialogOptions::FileName)]];
   }

   [mDelegate updateProperties];
}

void QCocoaFileDialogHelper::selectNameFilter(const QString &filter)
{
   if (!options()) {
      return;
   }
   const int index = options()->nameFilters().indexOf(filter);
   if (index != -1) {
      if (!mDelegate) {
         options()->setInitiallySelectedNameFilter(filter);
         return;
      }
      [mDelegate->mPopUpButton selectItemAtIndex: index];
      [mDelegate filterChanged: nil];
   }
}

QString QCocoaFileDialogHelper::selectedNameFilter() const
{
   if (!mDelegate) {
      return options()->initiallySelectedNameFilter();
   }
   int index = [mDelegate->mPopUpButton indexOfSelectedItem];
   if (index >= options()->nameFilters().count()) {
      return QString();
   }
   return index != -1 ? options()->nameFilters().at(index) : QString();
}

void QCocoaFileDialogHelper::hide()
{
   hideCocoaFilePanel();
}

bool QCocoaFileDialogHelper::show(Qt::WindowFlags windowFlags, Qt::WindowModality windowModality, QWindow *parent)
{
   //    Q_Q(QFileDialog);
   if (windowFlags & Qt::WindowStaysOnTopHint) {
      // The native file dialog tries all it can to stay
      // on the NSModalPanel level. And it might also show
      // its own "create directory" dialog that we cannot control.
      // So we need to use the non-native version in this case...
      return false;
   }

   return showCocoaFilePanel(windowModality, parent);
}

void QCocoaFileDialogHelper::createNSOpenSavePanelDelegate()
{
   QMacAutoReleasePool pool;

   const QSharedPointer<QPlatformFileDialogOptions> &opts = options();
   const QList<QUrl> selectedFiles = opts->initiallySelectedFiles();
   const QUrl directory = mDir.isEmpty() ? opts->initialDirectory() : mDir;
   const bool selectDir = selectedFiles.isEmpty();

   QString selection(selectDir ? directory.toLocalFile() : selectedFiles.front().toLocalFile());

   QNSOpenSavePanelDelegate *delegate = [[QNSOpenSavePanelDelegate alloc]
         initWithAcceptMode: selection
                    options: opts
                     helper: this];

   [static_cast<QNSOpenSavePanelDelegate *>(mDelegate) release];
   mDelegate = delegate;
}

bool QCocoaFileDialogHelper::showCocoaFilePanel(Qt::WindowModality windowModality, QWindow *parent)
{
   createNSOpenSavePanelDelegate();
   if (!mDelegate) {
      return false;
   }

   if (windowModality == Qt::NonModal) {
      [mDelegate showModelessPanel];
   } else if (windowModality == Qt::WindowModal && parent) {
      [mDelegate showWindowModalSheet: parent];
   }

   // no need to show a Qt::ApplicationModal dialog here, since it will be done in _q_platformRunNativeAppModalPanel()
   return true;
}

bool QCocoaFileDialogHelper::hideCocoaFilePanel()
{
   if (!mDelegate) {
      // Nothing to do. We return false to leave the question
      // open regarding whether or not to go native:
      return false;
   } else {
      [mDelegate closePanel];
      // Even when we hide it, we are still using a
      // native dialog, so return true:
      return true;
   }
}

void QCocoaFileDialogHelper::exec()
{
   // Note: If NSApp is not running (which is the case if e.g a top-most
   // QEventLoop has been interrupted, and the second-most event loop has not
   // yet been reactivated (regardless if [NSApp run] is still on the stack)),
   // showing a native modal dialog will fail.
   QMacAutoReleasePool pool;
   if ([mDelegate runApplicationModalPanel]) {
      emit accept();
   } else {
      emit reject();
   }

}

bool QCocoaFileDialogHelper::defaultNameFilterDisables() const
{
   return true;
}

#endif // QT_NO_FILEDIALOG
