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

#ifndef QTEXTDOCUMENT_H
#define QTEXTDOCUMENT_H

#include <QtCore/qobject.h>
#include <QtCore/qsize.h>
#include <QtCore/qrect.h>
#include <QtGui/qfont.h>
#include <qtextcursor.h>
#include <QScopedPointer>

QT_BEGIN_NAMESPACE

class QTextFormatCollection;
class QTextListFormat;
class QRect;
class QPainter;
class QPrinter;
class QAbstractTextDocumentLayout;
class QPoint;
class QTextObject;
class QTextFormat;
class QTextFrame;
class QTextBlock;
class QTextCodec;
class QUrl;
class QVariant;
class QRectF;
class QTextOption;
class QTextDocumentPrivate;

template<typename T> class QVector;

class Q_GUI_EXPORT QAbstractUndoItem
{

 public:
   virtual ~QAbstractUndoItem() = 0;
   virtual void undo() = 0;
   virtual void redo() = 0;
};

inline QAbstractUndoItem::~QAbstractUndoItem()
{
}

class Q_GUI_EXPORT QTextDocument : public QObject
{
   GUI_CS_OBJECT(QTextDocument)

   GUI_CS_PROPERTY_READ(undoRedoEnabled, isUndoRedoEnabled)
   GUI_CS_PROPERTY_WRITE(undoRedoEnabled, setUndoRedoEnabled)
   GUI_CS_PROPERTY_READ(modified, isModified)
   GUI_CS_PROPERTY_WRITE(modified, setModified)
   GUI_CS_PROPERTY_DESIGNABLE(modified, false)
   GUI_CS_PROPERTY_READ(pageSize, pageSize)
   GUI_CS_PROPERTY_WRITE(pageSize, setPageSize)
   GUI_CS_PROPERTY_READ(defaultFont, defaultFont)
   GUI_CS_PROPERTY_WRITE(defaultFont, setDefaultFont)
   GUI_CS_PROPERTY_READ(useDesignMetrics, useDesignMetrics)
   GUI_CS_PROPERTY_WRITE(useDesignMetrics, setUseDesignMetrics)
   GUI_CS_PROPERTY_READ(size, size)
   GUI_CS_PROPERTY_READ(textWidth, textWidth)
   GUI_CS_PROPERTY_WRITE(textWidth, setTextWidth)
   GUI_CS_PROPERTY_READ(blockCount, blockCount)
   GUI_CS_PROPERTY_READ(indentWidth, indentWidth)
   GUI_CS_PROPERTY_WRITE(indentWidth, setIndentWidth)

#ifndef QT_NO_CSSPARSER
   GUI_CS_PROPERTY_READ(defaultStyleSheet, defaultStyleSheet)
   GUI_CS_PROPERTY_WRITE(defaultStyleSheet, setDefaultStyleSheet)
#endif

   GUI_CS_PROPERTY_READ(maximumBlockCount, maximumBlockCount)
   GUI_CS_PROPERTY_WRITE(maximumBlockCount, setMaximumBlockCount)
   GUI_CS_PROPERTY_READ(documentMargin, documentMargin)
   GUI_CS_PROPERTY_WRITE(documentMargin, setDocumentMargin)

   // following 2 were qdoc_property 1/5/2014
   GUI_CS_PROPERTY_READ(defaultTextOption, defaultTextOption)
   GUI_CS_PROPERTY_WRITE(defaultTextOption, setDefaultTextOption)

 public:
   explicit QTextDocument(QObject *parent = nullptr);
   explicit QTextDocument(const QString &text, QObject *parent = nullptr);
   ~QTextDocument();

   QTextDocument *clone(QObject *parent = nullptr) const;

   bool isEmpty() const;
   virtual void clear();

   void setUndoRedoEnabled(bool enable);
   bool isUndoRedoEnabled() const;

   bool isUndoAvailable() const;
   bool isRedoAvailable() const;

   int availableUndoSteps() const;
   int availableRedoSteps() const;

   int revision() const;

   void setDocumentLayout(QAbstractTextDocumentLayout *layout);
   QAbstractTextDocumentLayout *documentLayout() const;

   enum MetaInformation {
      DocumentTitle,
      DocumentUrl
   };
   void setMetaInformation(MetaInformation info, const QString &);
   QString metaInformation(MetaInformation info) const;

#ifndef QT_NO_TEXTHTMLPARSER
   QString toHtml(const QByteArray &encoding = QByteArray()) const;
   void setHtml(const QString &html);
#endif

   QString toPlainText() const;
   void setPlainText(const QString &text);

   QChar characterAt(int pos) const;

   enum FindFlag {
      FindBackward        = 0x00001,
      FindCaseSensitively = 0x00002,
      FindWholeWords      = 0x00004
   };
   using FindFlags = QFlags<FindFlag>;

   QTextCursor find(const QString &subString, int from = 0, FindFlags options = 0) const;
   QTextCursor find(const QString &subString, const QTextCursor &from, FindFlags options = 0) const;

   QTextCursor find(const QRegExp &expr, int from = 0, FindFlags options = 0) const;
   QTextCursor find(const QRegExp &expr, const QTextCursor &from, FindFlags options = 0) const;

   QTextFrame *frameAt(int pos) const;
   QTextFrame *rootFrame() const;

   QTextObject *object(int objectIndex) const;
   QTextObject *objectForFormat(const QTextFormat &) const;

   QTextBlock findBlock(int pos) const;
   QTextBlock findBlockByNumber(int blockNumber) const;
   QTextBlock findBlockByLineNumber(int blockNumber) const;
   QTextBlock begin() const;
   QTextBlock end() const;

   QTextBlock firstBlock() const;
   QTextBlock lastBlock() const;

   void setPageSize(const QSizeF &size);
   QSizeF pageSize() const;

   void setDefaultFont(const QFont &font);
   QFont defaultFont() const;

   int pageCount() const;

   bool isModified() const;

#ifndef QT_NO_PRINTER
   void print(QPrinter *printer) const;
#endif

   enum ResourceType {
      HtmlResource  = 1,
      ImageResource = 2,
      StyleSheetResource = 3,

      UserResource  = 100
   };

   QVariant resource(int type, const QUrl &name) const;
   void addResource(int type, const QUrl &name, const QVariant &resource);

   QVector<QTextFormat> allFormats() const;

   void markContentsDirty(int from, int length);

   void setUseDesignMetrics(bool b);
   bool useDesignMetrics() const;

   void drawContents(QPainter *painter, const QRectF &rect = QRectF());

   void setTextWidth(qreal width);
   qreal textWidth() const;

   qreal idealWidth() const;

   qreal indentWidth() const;
   void setIndentWidth(qreal width);

   qreal documentMargin() const;
   void setDocumentMargin(qreal margin);

   void adjustSize();
   QSizeF size() const;

   int blockCount() const;
   int lineCount() const;
   int characterCount() const;

#ifndef QT_NO_CSSPARSER
   void setDefaultStyleSheet(const QString &sheet);
   QString defaultStyleSheet() const;
#endif

   void undo(QTextCursor *cursor);
   void redo(QTextCursor *cursor);

   enum Stacks {
      UndoStack = 0x01,
      RedoStack = 0x02,
      UndoAndRedoStacks = UndoStack | RedoStack
   };
   void clearUndoRedoStacks(Stacks historyToClear = UndoAndRedoStacks);

   int maximumBlockCount() const;
   void setMaximumBlockCount(int maximum);

   QTextOption defaultTextOption() const;
   void setDefaultTextOption(const QTextOption &option);

   Qt::CursorMoveStyle defaultCursorMoveStyle() const;
   void setDefaultCursorMoveStyle(Qt::CursorMoveStyle style);

   GUI_CS_SIGNAL_1(Public, void contentsChange(int from, int charsRemoved, int charsAdded))
   GUI_CS_SIGNAL_2(contentsChange, from, charsRemoved, charsAdded)

   GUI_CS_SIGNAL_1(Public, void contentsChanged())
   GUI_CS_SIGNAL_2(contentsChanged)

   GUI_CS_SIGNAL_1(Public, void undoAvailable(bool un_named_arg1))
   GUI_CS_SIGNAL_2(undoAvailable, un_named_arg1)

   GUI_CS_SIGNAL_1(Public, void redoAvailable(bool un_named_arg1))
   GUI_CS_SIGNAL_2(redoAvailable, un_named_arg1)

   GUI_CS_SIGNAL_1(Public, void undoCommandAdded())
   GUI_CS_SIGNAL_2(undoCommandAdded)

   GUI_CS_SIGNAL_1(Public, void modificationChanged(bool m))
   GUI_CS_SIGNAL_2(modificationChanged, m)

   GUI_CS_SIGNAL_1(Public, void cursorPositionChanged(const QTextCursor &cursor))
   GUI_CS_SIGNAL_2(cursorPositionChanged, cursor)

   GUI_CS_SIGNAL_1(Public, void blockCountChanged(int newBlockCount))
   GUI_CS_SIGNAL_2(blockCountChanged, newBlockCount)

   GUI_CS_SIGNAL_1(Public, void documentLayoutChanged())
   GUI_CS_SIGNAL_2(documentLayoutChanged)

   GUI_CS_SLOT_1(Public, void undo())
   GUI_CS_SLOT_OVERLOAD(undo, ())

   GUI_CS_SLOT_1(Public, void redo())
   GUI_CS_SLOT_OVERLOAD(redo, ())

   GUI_CS_SLOT_1(Public, void appendUndoItem(QAbstractUndoItem *un_named_arg1))
   GUI_CS_SLOT_2(appendUndoItem)

   GUI_CS_SLOT_1(Public, void setModified(bool m = true))
   GUI_CS_SLOT_2(setModified)

   QTextDocumentPrivate *docHandle() const;

 protected:
   virtual QTextObject *createObject(const QTextFormat &f);
   virtual QVariant loadResource(int type, const QUrl &name);

   QTextDocument(QTextDocumentPrivate &dd, QObject *parent);
   QScopedPointer<QTextDocumentPrivate> d_ptr;

 private:
   Q_DISABLE_COPY(QTextDocument)
   Q_DECLARE_PRIVATE(QTextDocument)
   friend class QTextObjectPrivate;

};

Q_DECLARE_OPERATORS_FOR_FLAGS(QTextDocument::FindFlags)

QT_END_NAMESPACE

#endif // QTEXTDOCUMENT_H
