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

#ifndef QTEXTDOCUMENT_P_H
#define QTEXTDOCUMENT_P_H

#include <qtextdocument.h>

#include <qglobal.h>
#include <qlist.h>
#include <qmap.h>
#include <qstring.h>
#include <qtextcursor.h>
#include <qtextlayout.h>
#include <qtextobject.h>
#include <qtextoption.h>
#include <qurl.h>
#include <qvariant.h>
#include <qvector.h>

#include <qcssparser_p.h>
#include <qfragmentmap_p.h>
#include <qtextformat_p.h>

class QAbstractTextDocumentLayout;
class QAbstractUndoItem;
class QTextBlockFormat;
class QTextCursorPrivate;
class QTextDocument;
class QTextFormat;
class QTextFormatCollection;
class QTextFrame;

#define QTextBeginningOfFrame QChar(0xfdd0)
#define QTextEndOfFrame       QChar(0xfdd1)

class QTextFragmentData : public QFragment<>
{
 public:
   inline void initialize() {}
   inline void invalidate() const {}
   inline void free() {}
   int stringPosition;
   int format;
};

class QTextBlockData : public QFragment<3>
{
 public:
   inline void initialize() {
      layout    = nullptr;
      userData  = nullptr;
      userState = -1;
      revision  = 0;
      hidden    = 0;
   }

   void invalidate() const;

   inline void free() {
      delete layout;
      layout = nullptr;

      delete userData;
      userData = nullptr;
   }

   mutable int format;

   // ##### probably store a QTextEngine * here
   mutable QTextLayout *layout;
   mutable QTextBlockUserData *userData;

   mutable int userState;
   mutable int revision : 31;
   mutable uint hidden : 1;
};

class QTextUndoCommand
{
 public:
   enum Command {
      Inserted = 0,
      Removed = 1,
      CharFormatChanged = 2,
      BlockFormatChanged = 3,
      BlockInserted = 4,
      BlockRemoved = 5,
      BlockAdded = 6,
      BlockDeleted = 7,
      GroupFormatChange = 8,
      CursorMoved = 9,
      Custom = 256
   };

   enum Operation {
      KeepCursor = 0,
      MoveCursor = 1
   };
   quint16 command;

   uint block_part : 1;    // all commands that are part of an undo block (including the first and the last one) have this set to 1
   uint block_end : 1;     // the last command in an undo block has this set to 1.
   uint block_padding : 6; // padding since block used to be a quint8
   quint8 operation;

   int format;
   quint32 strPos;
   quint32 pos;

   union {
      int blockFormat;
      quint32 length;
      QAbstractUndoItem *custom;
      int objectIndex;
   };
   quint32 revision;

   bool tryMerge(const QTextUndoCommand &other);
};

class QTextDocumentPrivate
{
   Q_DECLARE_PUBLIC(QTextDocument)

 public:
   using FragmentMap      = QFragmentMap<QTextFragmentData>;
   using FragmentIterator = FragmentMap::const_iterator;
   using BlockMap         = QFragmentMap<QTextBlockData>;

   QTextDocumentPrivate();
   virtual ~QTextDocumentPrivate();

   void init();
   void clear();

   void setLayout(QAbstractTextDocumentLayout *layout);

   void insert(int pos, const QString &text, int format);
   void insert(int pos, int strPos, int strLength, int format);

   int insertBlock(int pos, int blockFormat, int charFormat,
      QTextUndoCommand::Operation = QTextUndoCommand::MoveCursor);

   int insertBlock(QChar blockSeparator, int pos, int blockFormat, int charFormat,
      QTextUndoCommand::Operation op = QTextUndoCommand::MoveCursor);

   void move(int from, int to, int length, QTextUndoCommand::Operation = QTextUndoCommand::MoveCursor);
   void remove(int pos, int length, QTextUndoCommand::Operation = QTextUndoCommand::MoveCursor);

   void aboutToRemoveCell(int cursorFrom, int cursorEnd);

   QTextFrame *insertFrame(int start, int end, const QTextFrameFormat &format);
   void removeFrame(QTextFrame *frame);

   enum FormatChangeMode { MergeFormat, SetFormat, SetFormatAndPreserveObjectIndices };

   void setCharFormat(int pos, int length, const QTextCharFormat &newFormat, FormatChangeMode mode = SetFormat);
   void setBlockFormat(const QTextBlock &from, const QTextBlock &to,
      const QTextBlockFormat &newFormat, FormatChangeMode mode = SetFormat);

   void emitUndoAvailable(bool status);
   void emitRedoAvailable(bool status);

   int undoRedo(bool undo);

   inline void undo() {
      undoRedo(true);
   }

   inline void redo() {
      undoRedo(false);
   }

   void appendUndoItem(QAbstractUndoItem *);

   inline void beginEditBlock() {
      if (0 == editBlock++) {
         ++revision;
      }
   }

   void joinPreviousEditBlock();
   void endEditBlock();
   void finishEdit();
   inline bool isInEditBlock() const {
      return editBlock;
   }

   void enableUndoRedo(bool enable);
   inline bool isUndoRedoEnabled() const {
      return undoEnabled;
   }

   inline bool isUndoAvailable() const {
      return undoEnabled && undoState > 0;
   }

   inline bool isRedoAvailable() const {
      return undoEnabled && undoState < undoStack.size();
   }

   inline int availableUndoSteps() const {
      return undoEnabled ? undoState : 0;
   }

   inline int availableRedoSteps() const {
      return undoEnabled ? qMax(undoStack.size() - undoState - 1, 0) : 0;
   }

   inline QString buffer() const {
      return text;
   }

   QString plainText() const;

   inline int length() const {
      return fragments.length();
   }

   inline QTextFormatCollection *formatCollection() {
      return &formats;
   }

   inline const QTextFormatCollection *formatCollection() const {
      return &formats;
   }

   inline QAbstractTextDocumentLayout *layout() const {
      return lout;
   }

   inline FragmentIterator find(int pos) const {
      return fragments.find(pos);
   }

   inline FragmentIterator begin() const {
      return fragments.begin();
   }

   inline FragmentIterator end() const {
      return fragments.end();
   }

   inline QTextBlock blocksBegin() const {
      return QTextBlock(const_cast<QTextDocumentPrivate *>(this), blocks.firstNode());
   }

   inline QTextBlock blocksEnd() const {
      return QTextBlock(const_cast<QTextDocumentPrivate *>(this), 0);
   }

   inline QTextBlock blocksFind(int pos) const {
      return QTextBlock(const_cast<QTextDocumentPrivate *>(this), blocks.findNode(pos));
   }

   int blockCharFormatIndex(int node) const;

   inline int numBlocks() const {
      return blocks.numNodes();
   }

   const BlockMap &blockMap() const {
      return blocks;
   }

   const FragmentMap &fragmentMap() const {
      return fragments;
   }

   BlockMap &blockMap() {
      return blocks;
   }

   FragmentMap &fragmentMap() {
      return fragments;
   }

   static const QTextBlockData *block(const QTextBlock &it) {
      return it.p->blocks.fragment(it.n);
   }

   int nextCursorPosition(int position, QTextLayout::CursorMode mode) const;
   int previousCursorPosition(int position, QTextLayout::CursorMode mode) const;
   int leftCursorPosition(int position) const;
   int rightCursorPosition(int position) const;

   void changeObjectFormat(QTextObject *group, int format);

   void setModified(bool m);

   inline bool isModified() const {
      return modified;
   }

   inline QFont defaultFont() const {
      return formats.defaultFont();
   }

   inline void setDefaultFont(const QFont &f) {
      formats.setDefaultFont(f);
   }

   void clearUndoRedoStacks(QTextDocument::Stacks stacksToClear, bool emitSignals = false);

   void documentChange(int from, int length);

   inline void addCursor(QTextCursorPrivate *c) {
      cursors.append(c);
   }

   inline void removeCursor(QTextCursorPrivate *c) {
      cursors.removeAll(c);
   }

   QTextFrame *frameAt(int pos) const;
   QTextFrame *rootFrame() const;

   QTextObject *objectForIndex(int objectIndex) const;
   QTextObject *objectForFormat(int formatIndex) const;
   QTextObject *objectForFormat(const QTextFormat &f) const;

   QTextObject *createObject(const QTextFormat &newFormat, int objectIndex = -1);
   void deleteObject(QTextObject *object);

   QTextDocument *document() {
      return q_func();
   }

   const QTextDocument *document() const {
      return q_func();
   }

   bool ensureMaximumBlockCount();

   QTextOption defaultTextOption;
   Qt::CursorMoveStyle defaultCursorMoveStyle;

#ifndef QT_NO_CSSPARSER
   QCss::StyleSheet parsedDefaultStyleSheet;
#endif

   int maximumBlockCount;
   uint needsEnsureMaximumBlockCount : 1;
   uint inContentsChange : 1;
   uint blockCursorAdjustment : 1;
   QSizeF pageSize;
   QString title;
   QString url;
   qreal indentWidth;
   qreal documentMargin;
   QUrl baseUrl;

   void mergeCachedResources(const QTextDocumentPrivate *priv);

   friend class QTextHtmlExporter;
   friend class QTextCursor;

 protected:
   QTextDocument *q_ptr;

 private:
   bool split(int pos);
   bool unite(uint f);

   void insert_string(int pos, int strPos, uint length, int format, QTextUndoCommand::Operation op);
   int insert_block(int pos,   int strPos, int format, int blockformat, QTextUndoCommand::Operation op, int command);
   int remove_string(int pos,  uint length, QTextUndoCommand::Operation op);
   int remove_block(int pos,   int *blockformat, int command, QTextUndoCommand::Operation op);

   void insert_frame(QTextFrame *f);
   void scan_frames(int pos, int charsRemoved, int charsAdded);
   static void clearFrame(QTextFrame *f);

   void adjustDocumentChangesAndCursors(int from, int addedOrRemoved, QTextUndoCommand::Operation op);

   bool wasUndoAvailable;
   bool wasRedoAvailable;

   QTextDocumentPrivate(const QTextDocumentPrivate &m);
   QTextDocumentPrivate &operator= (const QTextDocumentPrivate &m);

   void appendUndoItem(const QTextUndoCommand &c);
   void contentsChanged();
   void compressPieceTable();

   QString text;
   uint unreachableCharacterCount;

   QVector<QTextUndoCommand> undoStack;
   bool undoEnabled;
   int undoState;
   int revision;

   // position in undo stack of the last setModified(false) call
   int modifiedState;
   bool modified;

   int editBlock;
   int editBlockCursorPosition;
   int docChangeFrom;
   int docChangeOldLength;
   int docChangeLength;
   bool framesDirty;

   QTextFormatCollection formats;
   mutable QTextFrame *rtFrame;
   QAbstractTextDocumentLayout *lout;

   FragmentMap fragments;
   BlockMap blocks;

   int initialBlockCharFormatIndex;

   QList<QTextCursorPrivate *> cursors;

   QMap<int, QTextObject *> objects;
   QMap<QUrl, QVariant> resources;
   QMap<QUrl, QVariant> cachedResources;

   QString defaultStyleSheet;

   int lastBlockCount;
};

class QTextTable;

class QTextHtmlExporter
{
 public:
   QTextHtmlExporter(const QTextDocument *_doc);

   enum ExportMode {
      ExportEntireDocument,
      ExportFragment
   };

   QString toHtml(const QString &encoding, ExportMode mode = ExportEntireDocument);

 private:
   enum StyleMode { EmitStyleTag, OmitStyleTag };
   enum FrameType { TextFrame, TableFrame, RootFrame };

   void emitFrame(QTextFrame::iterator frameIt);
   void emitTextFrame(const QTextFrame *frame);
   void emitBlock(const QTextBlock &block);
   void emitTable(const QTextTable *table);
   void emitFragment(const QTextFragment &fragment);

   void emitBlockAttributes(const QTextBlock &block);
   bool emitCharFormatStyle(const QTextCharFormat &format);
   void emitTextLength(const QString &attribute, const QTextLength &length);
   void emitAlignment(Qt::Alignment alignment);
   void emitFloatStyle(QTextFrameFormat::Position pos, StyleMode mode = EmitStyleTag);
   void emitMargins(const QString &top, const QString &bottom, const QString &left, const QString &right);
   void emitAttribute(const QString &attribute, const QString &value);
   void emitFrameStyle(const QTextFrameFormat &format, FrameType frameType);
   void emitBorderStyle(QTextFrameFormat::BorderStyle style);
   void emitPageBreakPolicy(QTextFormat::PageBreakFlags policy);

   void emitFontFamily(const QString &family);

   void emitBackgroundAttribute(const QTextFormat &format);
   QString findUrlForImage(const QTextDocument *doc, qint64 cacheKey, bool isPixmap);

   QString html;
   QTextCharFormat defaultCharFormat;
   const QTextDocument *doc;
   bool fragmentMarkers;
};

#endif
