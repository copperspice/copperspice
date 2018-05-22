/***********************************************************************
*
* Copyright (c) 2012-2018 Barbara Geller
* Copyright (c) 2012-2018 Ansel Sermersheim
* Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
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
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#include <qfiledialog.h>

#ifndef QT_NO_FILEDIALOG

#include <qfiledialog_p.h>
#include <qfiledialog_win_p.h>

#include <qapplication.h>
#include <qapplication_p.h>
#include <qt_windows.h>
#include <qglobal.h>
#include <qregularexpression.h>
#include <qbuffer.h>
#include <qdir.h>
#include <qstringlist.h>
#include <qsystemlibrary_p.h>

// declare them here because they are not present for all SDK/compilers
static const IID   QT_IID_IFileOpenDialog  = {0xd57c7288, 0xd4ad, 0x4768, {0xbe, 0x02, 0x9d, 0x96, 0x95, 0x32, 0xd9, 0x60} };
static const IID   QT_IID_IFileSaveDialog  = {0x84bccd23, 0x5fde, 0x4cdb, {0xae, 0xa4, 0xaf, 0x64, 0xb8, 0x3d, 0x78, 0xab} };
static const IID   QT_IID_IShellItem       = {0x43826d1e, 0xe718, 0x42ee, {0xbc, 0x55, 0xa1, 0xe2, 0x61, 0xc3, 0x7b, 0xfe} };

static const CLSID QT_CLSID_FileOpenDialog = {0xdc1c5a9c, 0xe88a, 0x4dde, {0xa5, 0xa1, 0x60, 0xf8, 0x2a, 0x20, 0xae, 0xf7} };
static const CLSID QT_CLSID_FileSaveDialog = {0xc0b4e2f3, 0xba21, 0x4773, {0x8d, 0xba, 0x33, 0x5e, 0xc9, 0x46, 0xeb, 0x8b} };

typedef qt_LPITEMIDLIST (WINAPI *PtrSHBrowseForFolder)(qt_BROWSEINFO *);
static PtrSHBrowseForFolder ptrSHBrowseForFolder = 0;

typedef BOOL (WINAPI *PtrSHGetPathFromIDList)(qt_LPITEMIDLIST, LPWSTR);
static PtrSHGetPathFromIDList ptrSHGetPathFromIDList = 0;

typedef HRESULT (WINAPI *PtrSHGetMalloc)(LPMALLOC *);
static PtrSHGetMalloc ptrSHGetMalloc = 0;

// forward declarations
static QStringList qt_win_CID_get_open_file_names(const QFileDialogArgs &args, QString *initialDirectory,
      const QStringList &filterList, QString *selectedFilter, int selectedFilterIndex);

static QString qt_win_CID_get_save_file_name(const QFileDialogArgs &args, QString *initialDirectory,
      const QStringList &filterList, QString *selectedFilter, int selectedFilterIndex);

QString qt_win_CID_get_existing_directory(const QFileDialogArgs &args);

static int __stdcall winGetExistDirCallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData);

static QString tCaption;

static const QString qt_file_dialog_filter_reg_exp = "^(.*)\\(([a-zA-Z0-9_.*? +;#\\-\\[\\]@\\{\\}/!<>\\$%&=^~:\\|]*)\\)$";

QStringList qt_make_filter_list(const QString &filter);

const int maxNameLen  = 1023;
const int maxMultiLen = 65535;

struct CS_OpenFile : public OPENFILENAME
{
   CS_OpenFile() = default;
   CS_OpenFile(const CS_OpenFile & data) = delete;

   std::wstring m_init_selection;
   std::wstring m_Filters;
   std::wstring m_InitDir;
   std::wstring m_Title;
};

static void qt_win_resolve_libs()
{
   static bool triedResolve = false;

   if (! triedResolve) {
      QSystemLibrary lib(QLatin1String("shell32"));
      ptrSHBrowseForFolder = (PtrSHBrowseForFolder)lib.resolve("SHBrowseForFolderW");
      ptrSHGetPathFromIDList = (PtrSHGetPathFromIDList)lib.resolve("SHGetPathFromIDListW");
      ptrSHGetMalloc = (PtrSHGetMalloc)lib.resolve("SHGetMalloc");

      triedResolve = true;
   }
}

// Returns the wildcard part of a filter.
static QString qt_win_extract_filter(const QString &rawFilter)
{
   QString result = rawFilter;

   static QRegularExpression regExp(qt_file_dialog_filter_reg_exp);
   QRegularExpressionMatch match = regExp.match(result);

   if (match.hasMatch()) {
      result = match.captured(2);
   }

   QStringList list = result.split(' ');

   for (auto &it : list) {
      if (it == "*") {
         it = "*.*";
         break;
      }
   }

   return list.join(";");
}

static QStringList qt_win_make_filters_list(const QString &filter)
{
   QString f(filter);

   if (f.isEmpty()) {
      f = QFileDialog::tr("All Files (*.*)");
   }

   return qt_make_filter_list(f);
}

// Makes a NUL-oriented Windows filter from a Qt filter.
static QString qt_win_filter(const QString &filter, bool hideFiltersDetails)
{
   QStringList filterLst    = qt_win_make_filters_list(filter);

   QString winfilters;

   static QRegularExpression regExp(qt_file_dialog_filter_reg_exp);
   QRegularExpressionMatch match;

   for (const auto &subfilter : filterLst) {

      if (! subfilter.isEmpty()) {

         if (hideFiltersDetails) {
            match = regExp.match(subfilter);

            if (match.hasMatch()) {
               winfilters += match.captured(1);
            }

         } else {
            winfilters += subfilter;
         }

         winfilters += QChar();
         winfilters += qt_win_extract_filter(subfilter);
         winfilters += QChar();
      }
   }

   winfilters += QChar();
   return winfilters;
}

static QString qt_win_selected_filter(const QString &filter, DWORD idx)
{
   return qt_win_make_filters_list(filter).at((int)idx - 1);
}

static std::unique_ptr<CS_OpenFile> qt_win_make_OFN(QWidget *parent, const QString &initialSelection,
                  const QString &initialDirectory, const QString &title, const QString &filters,
                  QFileDialog::FileMode mode, QFileDialog::Options options)
{
   if (parent) {
      parent = parent->window();

   } else {
      parent = QApplication::activeWindow();
   }

   QString initSel = QDir::toNativeSeparators(initialSelection);

   if (! initSel.isEmpty()) {
      initSel.remove('<');
      initSel.remove('>');
      initSel.remove('\"');
      initSel.remove('|');
   }

   int maxLen = mode == QFileDialog::ExistingFiles ? maxMultiLen : maxNameLen;

   std::unique_ptr<CS_OpenFile> ofn = std::make_unique<CS_OpenFile>();
   memset(ofn.get(), 0, sizeof(OPENFILENAME));

   ofn->lStructSize      = sizeof(OPENFILENAME);
   ofn->hwndOwner        = parent ? parent->winId() : 0;
   ofn->nMaxFile         = maxLen;

   ofn->m_Filters        = filters.toStdWString();
   ofn->lpstrFilter      = ofn->m_Filters.c_str();

   ofn->m_init_selection = initSel.toStdWString();
   ofn->m_init_selection.resize(maxLen);

   ofn->lpstrFile        = &ofn->m_init_selection[0];

   ofn->m_InitDir        = QDir::toNativeSeparators(initialDirectory).toStdWString();
   ofn->lpstrInitialDir  = ofn->m_InitDir.c_str();

   ofn->m_Title          = title.toStdWString();;
   ofn->lpstrTitle       = ofn->m_Title.c_str();

   ofn->Flags = (OFN_NOCHANGEDIR | OFN_HIDEREADONLY | OFN_EXPLORER | OFN_PATHMUSTEXIST);

   if (mode == QFileDialog::ExistingFile || mode == QFileDialog::ExistingFiles) {
      ofn->Flags |= (OFN_FILEMUSTEXIST);
   }

   if (mode == QFileDialog::ExistingFiles) {
      ofn->Flags |= (OFN_ALLOWMULTISELECT);
   }

   if (! (options & QFileDialog::DontConfirmOverwrite)) {
      ofn->Flags |= OFN_OVERWRITEPROMPT;
   }

   return ofn;
}

extern void qt_win_eatMouseMove();

QString qt_win_get_open_file_name(const QFileDialogArgs &args, QString *initialDirectory, QString *selectedFilter)
{
   QString result;

   QString isel = args.selection;
   if (initialDirectory && initialDirectory->left(5) == QLatin1String("file:")) {
      initialDirectory->remove(0, 5);
   }

   QFileInfo fi(*initialDirectory);

   if (initialDirectory && ! fi.isDir()) {
      *initialDirectory = fi.absolutePath();

      if (isel.isEmpty()) {
         isel = fi.fileName();
      }
   }

   if (! fi.exists()) {
      *initialDirectory = QDir::homePath();
   }

   DWORD selFilIdx = 0;

   int idx = 0;
   if (selectedFilter) {
      QStringList filterLst = qt_win_make_filters_list(args.filter);
      idx = filterLst.indexOf(*selectedFilter);
   }

   QDialog modal_widget;
   modal_widget.setAttribute(Qt::WA_NoChildEventsForParent, true);
   modal_widget.setParent(args.parent, Qt::Window);
   QApplicationPrivate::enterModal(&modal_widget);

   bool hideFiltersDetails = args.options & QFileDialog::HideNameFilterDetails;
   std::unique_ptr<CS_OpenFile> ofn = qt_win_make_OFN(args.parent, args.selection, args.directory, args.caption,
                  qt_win_filter(args.filter, hideFiltersDetails), QFileDialog::ExistingFile, args.options);

   if (idx) {
      ofn->nFilterIndex = idx + 1;
   }

   if (GetOpenFileName(ofn.get())) {
      std::wstring tmp(ofn->lpstrFile);
      result = QString::fromStdWString(tmp);

      selFilIdx = ofn->nFilterIndex;
   }

   QApplicationPrivate::leaveModal(&modal_widget);

   qt_win_eatMouseMove();

   if (result.isEmpty()) {
      return result;
   }

   fi = result;
   *initialDirectory = fi.path();

   if (selectedFilter) {
      *selectedFilter = qt_win_selected_filter(args.filter, selFilIdx);
   }

   return fi.absoluteFilePath();
}

QStringList qt_win_get_open_file_names(const QFileDialogArgs &args, QString *initialDirectory, QString *selectedFilter)
{
   QDir dir;

   if (initialDirectory && initialDirectory->left(5) == QLatin1String("file:")) {
      initialDirectory->remove(0, 5);
   }

   QFileInfo fi = QFileInfo(*initialDirectory);

   if (initialDirectory && !fi.isDir()) {
      *initialDirectory = fi.absolutePath();
   }

   if (! fi.exists()) {
      *initialDirectory = QDir::homePath();
   }

   DWORD selFilIdx = 0;

   QStringList filterLst = qt_win_make_filters_list(args.filter);

   int idx = 0;
   if (selectedFilter) {
      idx = filterLst.indexOf(*selectedFilter);
   }

   // Windows Vista & above allows users to search from file dialogs. If user selects
   // multiple files belonging to different folders from these search results, the
   // GetOpenFileName() will return only one folder name for all the files. To retrieve
   // the correct path for all selected files, we have to use Common Item Dialog interfaces.

   if (QSysInfo::WindowsVersion >= QSysInfo::WV_VISTA && (QSysInfo::WindowsVersion & QSysInfo::WV_NT_based))  {
      return qt_win_CID_get_open_file_names(args, initialDirectory, filterLst, selectedFilter, idx);
   }

   QStringList result;

   QDialog modal_widget;
   modal_widget.setAttribute(Qt::WA_NoChildEventsForParent, true);
   modal_widget.setParent(args.parent, Qt::Window);
   QApplicationPrivate::enterModal(&modal_widget);

   bool hideFiltersDetails = args.options & QFileDialog::HideNameFilterDetails;
   std::unique_ptr<CS_OpenFile> ofn = qt_win_make_OFN(args.parent, args.selection, args.directory, args.caption,
                  qt_win_filter(args.filter, hideFiltersDetails), QFileDialog::ExistingFiles, args.options);

   if (idx) {
      ofn->nFilterIndex = idx + 1;
   }

   if (GetOpenFileName(ofn.get())) {
      std::wstring tmp(ofn->lpstrFile);
      QString fileOrDir = QString::fromStdWString(tmp);

      selFilIdx  = ofn->nFilterIndex;
      int offset = fileOrDir.length() + 1;

      if (ofn->lpstrFile[offset] == 0) {
         // Only one file selected; has full path
         fi.setFile(fileOrDir);
         QString res = fi.absoluteFilePath();

         if (! res.isEmpty()) {
            result.append(res);
         }

      } else {
         // Several files selected; first string is path
         dir.setPath(fileOrDir);

         QString f;
         std::wstring tmp;

         while (true) {
            tmp = ofn->lpstrFile + offset;
            f   = QString::fromStdWString(tmp);

            if (f.isEmpty())  {
               break;
            }

            fi.setFile(dir, f);
            QString res = fi.absoluteFilePath();

            if (! res.isEmpty()) {
               result.append(res);
            }
            offset += f.length() + 1;
         }
      }
   }

   QApplicationPrivate::leaveModal(&modal_widget);

   qt_win_eatMouseMove();

   if (!result.isEmpty()) {
      *initialDirectory = fi.path();    // only save the path if there is a result

      if (selectedFilter) {
         *selectedFilter = qt_win_selected_filter(args.filter, selFilIdx);
      }
   }
   return result;
}

QString qt_win_get_save_file_name(const QFileDialogArgs &args, QString *initialDirectory, QString *selectedFilter)
{
   QString result;

   QString isel = args.selection;
   if (initialDirectory && initialDirectory->left(5) == "file:") {
      initialDirectory->remove(0, 5);
   }

   QFileInfo fi(*initialDirectory);

   if (initialDirectory && ! fi.isDir()) {
      *initialDirectory = fi.absolutePath();

      if (isel.isEmpty()) {
         isel = fi.fileName();
      }
   }

   if (! fi.exists()) {
      *initialDirectory = QDir::homePath();
   }

   DWORD selFilIdx = 0;

   int idx = 0;
   QStringList filterLst = qt_win_make_filters_list(args.filter);

   if (selectedFilter) {
      idx = filterLst.indexOf(*selectedFilter);
   }

   // CopperSpice change
   if ((args.options & QFileDialog::Option::ForceInitialDir_Win7) &&
         (QSysInfo::windowsVersion() >= QSysInfo::WV_WINDOWS7))   {

      return qt_win_CID_get_save_file_name(args, initialDirectory, filterLst, selectedFilter, idx);
   }

   QDialog modal_widget;
   modal_widget.setAttribute(Qt::WA_NoChildEventsForParent, true);
   modal_widget.setParent(args.parent, Qt::Window);
   QApplicationPrivate::enterModal(&modal_widget);

   bool hideFiltersDetails = args.options & QFileDialog::HideNameFilterDetails;

   // This block is used below for the lpstrDefExt member.
   // Note that the current MSDN docs document this member wrong.
   // It should rather be documented as "the default extension if no extension was given and if the
   // current filter does not have a extension (e.g (*)). If the current filter have an extension, use
   // the extension of the current filter"

   QString defaultSaveExt;

   if (selectedFilter && !selectedFilter->isEmpty()) {
      defaultSaveExt = qt_win_extract_filter(*selectedFilter);

      // make sure we only have the extension
      int firstDot = defaultSaveExt.indexOf('.');

      if (firstDot != -1) {
         defaultSaveExt.remove(0, firstDot + 1);
      } else {
         defaultSaveExt.clear();
      }
   }

   std::unique_ptr<CS_OpenFile> ofn = qt_win_make_OFN(args.parent, args.selection, args.directory, args.caption,
                  qt_win_filter(args.filter, hideFiltersDetails), QFileDialog::AnyFile, args.options);

   std::wstring tmp(defaultSaveExt.toStdWString());
   ofn->lpstrDefExt = &tmp[0];

   if (idx) {
      ofn->nFilterIndex = idx + 1;
   }

   if (GetSaveFileName(ofn.get())) {
      std::wstring tmp(ofn->lpstrFile);
      result = QString::fromStdWString(tmp);

      selFilIdx = ofn->nFilterIndex;
   }

   QApplicationPrivate::leaveModal(&modal_widget);

   qt_win_eatMouseMove();

   if (result.isEmpty()) {
      return result;
   }

   fi = result;
   *initialDirectory = fi.path();

   if (selectedFilter) {
      *selectedFilter = qt_win_selected_filter(args.filter, selFilIdx);
   }

   return fi.absoluteFilePath();
}

QString qt_win_get_existing_directory(const QFileDialogArgs &args)
{
   if (QSysInfo::WindowsVersion >= QSysInfo::WV_VISTA && (QSysInfo::WindowsVersion & QSysInfo::WV_NT_based)) {
      return qt_win_CID_get_existing_directory(args);
   }

   QString result;
   QWidget *parent = args.parent;

   if (parent) {
      parent = parent->window();
   } else {
      parent = QApplication::activeWindow();
   }

   if (parent) {
      parent->createWinId();
   }

   QDialog modal_widget;
   modal_widget.setAttribute(Qt::WA_NoChildEventsForParent, true);
   modal_widget.setParent(parent, Qt::Window);
   QApplicationPrivate::enterModal(&modal_widget);

   QString initDir = QDir::toNativeSeparators(args.directory);
   wchar_t path[MAX_PATH];
   wchar_t initPath[MAX_PATH];

   initPath[0] = 0;
   path[0]     = 0;

   tCaption = args.caption;

   qt_BROWSEINFO bi;

   Q_ASSERT(! parent || parent->testAttribute(Qt::WA_WState_Created));
   bi.hwndOwner = (parent ? parent->winId() : 0);
   bi.pidlRoot  = NULL;

   // area above the tree view is hard set to empty
   std::wstring title;
   bi.lpszTitle = title.c_str();

   bi.pszDisplayName = initPath;
   bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_STATUSTEXT | BIF_NEWDIALOGSTYLE;

   bi.lpfn    = winGetExistDirCallbackProc;
   bi.lParam  = LPARAM(&initDir);

   qt_win_resolve_libs();

   if (ptrSHBrowseForFolder) {
      qt_LPITEMIDLIST pItemIDList = ptrSHBrowseForFolder(&bi);

      if (pItemIDList) {
         ptrSHGetPathFromIDList(pItemIDList, path);

         IMalloc *pMalloc;
         if (ptrSHGetMalloc(&pMalloc) == NOERROR) {
            pMalloc->Free(pItemIDList);
            pMalloc->Release();

            std::wstring tmp(path);
            result = QString::fromStdWString(tmp);
         }
      }
   }

   QApplicationPrivate::leaveModal(&modal_widget);

   qt_win_eatMouseMove();

   if (! result.isEmpty()) {
      result.replace(QLatin1Char('\\'), QLatin1Char('/'));
   }

   return result;
}

static int __stdcall winGetExistDirCallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{
   if (uMsg == BFFM_INITIALIZED && lpData != 0) {

      // set window caption
      SetWindowTextW(hwnd, tCaption.toStdWString().c_str());

      // set starting folder
      QString *initDir = (QString *)(lpData);

      if (! initDir->isEmpty()) {
         std::wstring tmp(initDir->toStdWString());
         SendMessage(hwnd, BFFM_SETSELECTION, TRUE, LPARAM(tmp.c_str()));
      }

   } else if (uMsg == BFFM_SELCHANGED) {
      qt_win_resolve_libs();

      if (ptrSHGetPathFromIDList) {
         wchar_t path[MAX_PATH];
         ptrSHGetPathFromIDList(qt_LPITEMIDLIST(lParam), path);

         std::wstring tmp(path);
         QString tmpStr = QString::fromStdWString(tmp);

         if (! tmpStr.isEmpty()) {
            SendMessage(hwnd, BFFM_ENABLEOK, 1, 1);
         } else {
            SendMessage(hwnd, BFFM_ENABLEOK, 0, 0);
         }

         SendMessage(hwnd, BFFM_SETSTATUSTEXT, 1, LPARAM(path));
      }
   }

   return 0;
}

//  *****
typedef HRESULT (WINAPI *PtrSHCreateItemFromParsingName)(PCWSTR pszPath, IBindCtx *pbc, REFIID riid, void **ppv);
static PtrSHCreateItemFromParsingName pSHCreateItemFromParsingName = 0;

static bool qt_win_set_IFileDialogOptions(IFileDialog *pfd, const QString &initialSelection,
      const QString &initialDirectory, const QString &title, const QStringList &filterLst,
      QFileDialog::FileMode mode, QFileDialog::Options options)
{
   if (! pSHCreateItemFromParsingName) {
      // This function is available only in Vista & above.
      QSystemLibrary shellLib("Shell32");

      pSHCreateItemFromParsingName = (PtrSHCreateItemFromParsingName) shellLib.resolve("SHCreateItemFromParsingName");

      if (!pSHCreateItemFromParsingName) {
         return false;
      }
   }

   HRESULT hr;
   QString winfilters;
   int numFilters = 0;
   quint32 currentOffset = 0;

   QList<quint32> offsets;

   // Create the native filter string and save offset to each entry.
   for (auto subfilter : filterLst) {

      if (! subfilter.isEmpty()) {
         offsets << currentOffset;

         // Here the COMMON_ITEM_DIALOG API always add the details for the filter (e.g. *.txt)
         // so we don't need to handle the flag HideNameFilterDetails.

         winfilters += subfilter; // The name of the filter.
         winfilters += QChar();
         currentOffset += subfilter.size() + 1;
         offsets << currentOffset;

         QString spec = qt_win_extract_filter(subfilter);
         winfilters += spec; // The actual filter spec.
         winfilters += QChar();
         currentOffset += spec.size() + 1;
         numFilters++;
      }
   }

   // Add the filters to the file dialog
   if (numFilters) {
      std::wstring szData(winfilters.toStdWString());
      qt_COMDLG_FILTERSPEC *filterSpec = new qt_COMDLG_FILTERSPEC[numFilters];

      for (int i = 0; i < numFilters; i++) {
         filterSpec[i].pszName = szData.c_str() + offsets[i * 2];
         filterSpec[i].pszSpec = szData.c_str() + offsets[(i * 2) + 1];
      }

      hr = pfd->SetFileTypes(numFilters, filterSpec);
      delete []filterSpec;
   }

   // Set the starting folder.
   std::wstring initDir = QDir::toNativeSeparators(initialDirectory).toStdWString();

   if (! initDir.empty()) {
      IShellItem *psiDefaultFolder;
      hr = pSHCreateItemFromParsingName(initDir.c_str(), NULL, QT_IID_IShellItem, reinterpret_cast<void **>(&psiDefaultFolder));

      if (SUCCEEDED(hr)) {
         hr = pfd->SetFolder(psiDefaultFolder);
         psiDefaultFolder->Release();
      }
   }

   // Set the currently selected file.
   QString initSel = QDir::toNativeSeparators(initialSelection);

   if (! initSel.isEmpty()) {
      initSel.remove(QLatin1Char('<'));
      initSel.remove(QLatin1Char('>'));
      initSel.remove(QLatin1Char('\"'));
      initSel.remove(QLatin1Char('|'));
   }

   if (! initSel.isEmpty()) {
      std::wstring tmp(initSel.toStdWString());
      hr = pfd->SetFileName(tmp.c_str());
   }

   // Set the title for the file dialog.
   if (! title.isEmpty()) {
      std::wstring tmp(title.toStdWString());
      hr = pfd->SetTitle(tmp.c_str());
   }

   // Set other flags for the dialog.
   DWORD newOptions;
   hr = pfd->GetOptions(&newOptions);

   if (SUCCEEDED(hr)) {
      newOptions |= FOS_NOCHANGEDIR;

      if (mode == QFileDialog::ExistingFile || mode == QFileDialog::ExistingFiles) {
         newOptions |= (FOS_FILEMUSTEXIST | FOS_PATHMUSTEXIST);
      }

      if (mode == QFileDialog::ExistingFiles) {
         newOptions |= FOS_ALLOWMULTISELECT;
      }

      if (! (options & QFileDialog::DontConfirmOverwrite)) {
         newOptions |= FOS_OVERWRITEPROMPT;
      }

      hr = pfd->SetOptions(newOptions);
   }

   return SUCCEEDED(hr);
}

static QStringList qt_win_CID_get_open_file_names(const QFileDialogArgs &args,
                  QString *initialDirectory, const QStringList &filterList,
                  QString *selectedFilter, int selectedFilterIndex)
{
   QStringList result;

   QDialog modal_widget;
   modal_widget.setAttribute(Qt::WA_NoChildEventsForParent, true);
   modal_widget.setParent(args.parent, Qt::Window);
   QApplicationPrivate::enterModal(&modal_widget);

   // Multiple selection is allowed only in IFileOpenDialog.
   IFileOpenDialog *pfd = 0;
   HRESULT hr = CoCreateInstance(QT_CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, QT_IID_IFileOpenDialog,
                                 reinterpret_cast<void **>(&pfd));

   if (SUCCEEDED(hr)) {
      qt_win_set_IFileDialogOptions(pfd, args.selection, args.directory, args.caption,
                  filterList, QFileDialog::ExistingFiles, args.options);

      // Set the currently selected filter (one-based index).
      hr = pfd->SetFileTypeIndex(selectedFilterIndex + 1);
      QWidget *parentWindow = args.parent;

      if (parentWindow) {
         parentWindow = parentWindow->window();
      } else {
         parentWindow = QApplication::activeWindow();
      }

      // Show the file dialog.
      hr = pfd->Show(parentWindow ? parentWindow->winId() : 0);
      if (SUCCEEDED(hr)) {
         // Retrieve the results.
         IShellItemArray *psiaResults;
         hr = pfd->GetResults(&psiaResults);

         if (SUCCEEDED(hr)) {
            DWORD numItems = 0;
            psiaResults->GetCount(&numItems);

            for (DWORD i = 0; i < numItems; i++) {
               IShellItem *psi = 0;
               hr = psiaResults->GetItemAt(i, &psi);

               if (SUCCEEDED(hr)) {
                  // Retrieve the file name from shell item

                  wchar_t *pszPath;
                  hr = psi->GetDisplayName(SIGDN_FILESYSPATH, &pszPath);

                  if (SUCCEEDED(hr)) {
                     std::wstring tmp(pszPath);
                     QString fileName = QString::fromStdWString(tmp);

                     result.append(fileName);
                     CoTaskMemFree(pszPath);
                  }

                  psi->Release(); // Free the current item.
               }
            }
            psiaResults->Release(); // Free the array of items.
         }
      }
   }
   QApplicationPrivate::leaveModal(&modal_widget);

   qt_win_eatMouseMove();

   if (!result.isEmpty()) {
      // Retrieve the current folder name.
      IShellItem *psi = 0;
      hr = pfd->GetFolder(&psi);

      if (SUCCEEDED(hr)) {
         wchar_t *pszPath;
         hr = psi->GetDisplayName(SIGDN_FILESYSPATH, &pszPath);

         if (SUCCEEDED(hr)) {
            std::wstring tmp(pszPath);
            *initialDirectory = QString::fromStdWString(tmp);

            CoTaskMemFree(pszPath);
         }
         psi->Release();
      }

      // Retrieve the currently selected filter.
      if (selectedFilter) {
         quint32 filetype = 0;
         hr = pfd->GetFileTypeIndex(&filetype);

         if (SUCCEEDED(hr) && filetype && filetype <= (quint32)filterList.length()) {
            // This is a one-based index, not zero-based.
            *selectedFilter = filterList[filetype - 1];
         }
      }
   }

   if (pfd) {
      pfd->Release();
   }

   return result;
}

static QString qt_win_CID_get_save_file_name(const QFileDialogArgs &args, QString *initialDirectory,
                  const QStringList &filterList, QString *selectedFilter, int selectedFilterIndex)
{
   QString result;

   QDialog modal_widget;
   modal_widget.setAttribute(Qt::WA_NoChildEventsForParent, true);
   modal_widget.setParent(args.parent, Qt::Window);
   QApplicationPrivate::enterModal(&modal_widget);

   IFileSaveDialog *pfd = 0;
   HRESULT hr = CoCreateInstance(QT_CLSID_FileSaveDialog, NULL, CLSCTX_INPROC_SERVER, QT_IID_IFileSaveDialog,
                                 reinterpret_cast<void **>(&pfd));

   if (SUCCEEDED(hr)) {
      qt_win_set_IFileDialogOptions(pfd, args.selection, args.directory, args.caption,
                  filterList, QFileDialog::AnyFile, args.options);

      // Set the currently selected filter (one-based index).
      hr = pfd->SetFileTypeIndex(selectedFilterIndex + 1);
      QWidget *parentWindow = args.parent;

      if (parentWindow) {
         parentWindow = parentWindow->window();
      } else {
         parentWindow = QApplication::activeWindow();
      }

      // Show the file dialog
      hr = pfd->Show(parentWindow ? parentWindow->winId() : 0);

      if (SUCCEEDED(hr)) {
         // Retrieve the results
         IShellItem *psi;
         hr = pfd->GetResult(&psi);

         if (SUCCEEDED(hr)) {
            // Retrieve the file name from shell item

            wchar_t *pszPath;
            hr = psi->GetDisplayName(SIGDN_FILESYSPATH, &pszPath);

            if (SUCCEEDED(hr)) {
               std::wstring tmp(pszPath);
               QString fileName = QString::fromStdWString(tmp);

               result.append(fileName);
               CoTaskMemFree(pszPath);
            }

            psi->Release(); // Free the current item
         }
      }
   }
   QApplicationPrivate::leaveModal(&modal_widget);

   qt_win_eatMouseMove();

   if (result.isEmpty()) {
      //

   } else {
      // Retrieve the currently selected folder name
      IShellItem *psi = 0;
      hr = pfd->GetFolder(&psi);

      if (SUCCEEDED(hr)) {
         wchar_t *pszPath;
         hr = psi->GetDisplayName(SIGDN_FILESYSPATH, &pszPath);

         if (SUCCEEDED(hr)) {
            std::wstring tmp(pszPath);
            *initialDirectory = QString::fromStdWString(tmp);

            CoTaskMemFree(pszPath);
         }
         psi->Release();
      }

      // Retrieve the currently selected filter
      if (selectedFilter) {
         quint32 filetype = 0;
         hr = pfd->GetFileTypeIndex(&filetype);

         if (SUCCEEDED(hr) && filetype && filetype <= (quint32)filterList.length()) {
            // This is a one-based index, not zero-based
            *selectedFilter = filterList[filetype - 1];
         }
      }
   }

   if (pfd) {
      pfd->Release();
   }

   return result;
}

QString qt_win_CID_get_existing_directory(const QFileDialogArgs &args)
{
   QString result;

   QDialog modal_widget;
   modal_widget.setAttribute(Qt::WA_NoChildEventsForParent, true);
   modal_widget.setParent(args.parent, Qt::Window);
   QApplicationPrivate::enterModal(&modal_widget);

   IFileOpenDialog *pfd = 0;
   HRESULT hr = CoCreateInstance(QT_CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER,
                                 QT_IID_IFileOpenDialog, reinterpret_cast<void **>(&pfd));

   if (SUCCEEDED(hr)) {
      qt_win_set_IFileDialogOptions(pfd, args.selection, args.directory, args.caption,
                  QStringList(), QFileDialog::ExistingFile, args.options);

      // Set the FOS_PICKFOLDERS flag
      DWORD newOptions;
      hr = pfd->GetOptions(&newOptions);
      newOptions |= (FOS_PICKFOLDERS | FOS_FORCEFILESYSTEM);

      if (SUCCEEDED(hr) && SUCCEEDED((hr = pfd->SetOptions(newOptions)))) {
         QWidget *parentWindow = args.parent;
         if (parentWindow) {
            parentWindow = parentWindow->window();
         } else {
            parentWindow = QApplication::activeWindow();
         }

         // Show the file dialog.
         hr = pfd->Show(parentWindow ? parentWindow->winId() : 0);

         if (SUCCEEDED(hr)) {
            // Retrieve the result

            IShellItem *psi = 0;
            hr = pfd->GetResult(&psi);
            if (SUCCEEDED(hr)) {
               // Retrieve the file name from shell item.
               wchar_t *pszPath;
               hr = psi->GetDisplayName(SIGDN_FILESYSPATH, &pszPath);

               if (SUCCEEDED(hr)) {
                  std::wstring tmp(pszPath);
                  result = QString::fromStdWString(tmp);

                  CoTaskMemFree(pszPath);
               }
               psi->Release(); // Free the current item.
            }
         }
      }
   }

   QApplicationPrivate::leaveModal(&modal_widget);

   qt_win_eatMouseMove();

   if (pfd) {
      pfd->Release();
   }
   return result;
}

#endif
