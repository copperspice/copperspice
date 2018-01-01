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

#ifndef QTEXTBROWSER_H
#define QTEXTBROWSER_H

#include <QtGui/qtextedit.h>
#include <QtCore/qurl.h>

QT_BEGIN_NAMESPACE

#ifndef QT_NO_TEXTBROWSER

class QTextBrowserPrivate;

class Q_GUI_EXPORT QTextBrowser : public QTextEdit
{
   GUI_CS_OBJECT(QTextBrowser)

   GUI_CS_PROPERTY_READ(source, source)
   GUI_CS_PROPERTY_WRITE(source, setSource)

   // overloaded properties
   GUI_CS_PROPERTY_READ(readOnly, isReadOnly)
   GUI_CS_PROPERTY_WRITE(readOnly, setReadOnly)
   GUI_CS_PROPERTY_DESIGNABLE(readOnly, false)
   GUI_CS_PROPERTY_SCRIPTABLE(readOnly, false)

   // overloaded properties
   GUI_CS_PROPERTY_READ(undoRedoEnabled, isUndoRedoEnabled)
   GUI_CS_PROPERTY_WRITE(undoRedoEnabled, setUndoRedoEnabled)
   GUI_CS_PROPERTY_DESIGNABLE(undoRedoEnabled, false)
   GUI_CS_PROPERTY_SCRIPTABLE(undoRedoEnabled, false)

   GUI_CS_PROPERTY_READ(searchPaths, searchPaths)
   GUI_CS_PROPERTY_WRITE(searchPaths, setSearchPaths)

   GUI_CS_PROPERTY_READ(openExternalLinks, openExternalLinks)
   GUI_CS_PROPERTY_WRITE(openExternalLinks, setOpenExternalLinks)

   GUI_CS_PROPERTY_READ(openLinks, openLinks)
   GUI_CS_PROPERTY_WRITE(openLinks, setOpenLinks)

 public:
   explicit QTextBrowser(QWidget *parent = nullptr);
   virtual ~QTextBrowser();

   QUrl source() const;

   QStringList searchPaths() const;
   void setSearchPaths(const QStringList &paths);

   QVariant loadResource(int type, const QUrl &name) override;

   bool isBackwardAvailable() const;
   bool isForwardAvailable() const;
   void clearHistory();
   QString historyTitle(int) const;
   QUrl historyUrl(int) const;
   int backwardHistoryCount() const;
   int forwardHistoryCount() const;

   bool openExternalLinks() const;
   void setOpenExternalLinks(bool open);

   bool openLinks() const;
   void setOpenLinks(bool open);

   GUI_CS_SLOT_1(Public, virtual void setSource(const QUrl &name))
   GUI_CS_SLOT_2(setSource)
   GUI_CS_SLOT_1(Public, virtual void backward())
   GUI_CS_SLOT_2(backward)
   GUI_CS_SLOT_1(Public, virtual void forward())
   GUI_CS_SLOT_2(forward)
   GUI_CS_SLOT_1(Public, virtual void home())
   GUI_CS_SLOT_2(home)
   GUI_CS_SLOT_1(Public, virtual void reload())
   GUI_CS_SLOT_2(reload)

   GUI_CS_SIGNAL_1(Public, void backwardAvailable(bool un_named_arg1))
   GUI_CS_SIGNAL_2(backwardAvailable, un_named_arg1)

   GUI_CS_SIGNAL_1(Public, void forwardAvailable(bool un_named_arg1))
   GUI_CS_SIGNAL_2(forwardAvailable, un_named_arg1)

   GUI_CS_SIGNAL_1(Public, void historyChanged())
   GUI_CS_SIGNAL_2(historyChanged)

   GUI_CS_SIGNAL_1(Public, void sourceChanged(const QUrl &un_named_arg1))
   GUI_CS_SIGNAL_2(sourceChanged, un_named_arg1)

   GUI_CS_SIGNAL_1(Public, void highlighted(const QUrl &un_named_arg1))
   GUI_CS_SIGNAL_OVERLOAD(highlighted, (const QUrl &), un_named_arg1)

   GUI_CS_SIGNAL_1(Public, void highlighted(const QString &un_named_arg1))
   GUI_CS_SIGNAL_OVERLOAD(highlighted, (const QString &), un_named_arg1)

   GUI_CS_SIGNAL_1(Public, void anchorClicked(const QUrl &un_named_arg1))
   GUI_CS_SIGNAL_2(anchorClicked, un_named_arg1)

 protected:
   bool event(QEvent *e) override;
   void keyPressEvent(QKeyEvent *ev) override;
   void mouseMoveEvent(QMouseEvent *ev) override;
   void mousePressEvent(QMouseEvent *ev) override;
   void mouseReleaseEvent(QMouseEvent *ev) override;
   void focusOutEvent(QFocusEvent *ev) override;
   bool focusNextPrevChild(bool next) override;
   void paintEvent(QPaintEvent *e) override;

 private:
   Q_DISABLE_COPY(QTextBrowser)
   Q_DECLARE_PRIVATE(QTextBrowser)

   GUI_CS_SLOT_1(Private, void _q_documentModified())
   GUI_CS_SLOT_2(_q_documentModified)

   GUI_CS_SLOT_1(Private, void _q_activateAnchor(const QString &un_named_arg1))
   GUI_CS_SLOT_2(_q_activateAnchor)

   GUI_CS_SLOT_1(Private, void _q_highlightLink(const QString &un_named_arg1))
   GUI_CS_SLOT_2(_q_highlightLink)
};

#endif // QT_NO_TEXTBROWSER

QT_END_NAMESPACE

#endif // QTEXTBROWSER_H
