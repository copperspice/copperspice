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

#ifndef QTEXTBROWSER_H
#define QTEXTBROWSER_H

#include <qtextedit.h>
#include <qurl.h>

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

   QTextBrowser(const QTextBrowser &) = delete;
   QTextBrowser &operator=(const QTextBrowser &) = delete;

   virtual ~QTextBrowser();

   QUrl source() const;

   QStringList searchPaths() const;
   void setSearchPaths(const QStringList &paths);

   QVariant loadResource(int type, const QUrl &name) override;

   bool isBackwardAvailable() const;
   bool isForwardAvailable() const;
   void clearHistory();
   QString historyTitle(int index) const;
   QUrl historyUrl(int index) const;
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

   GUI_CS_SIGNAL_1(Public, void backwardAvailable(bool available))
   GUI_CS_SIGNAL_2(backwardAvailable, available)

   GUI_CS_SIGNAL_1(Public, void forwardAvailable(bool available))
   GUI_CS_SIGNAL_2(forwardAvailable, available)

   GUI_CS_SIGNAL_1(Public, void historyChanged())
   GUI_CS_SIGNAL_2(historyChanged)

   GUI_CS_SIGNAL_1(Public, void sourceChanged(const QUrl &url))
   GUI_CS_SIGNAL_2(sourceChanged, url)

   GUI_CS_SIGNAL_1(Public, void highlighted(const QUrl &url))
   GUI_CS_SIGNAL_OVERLOAD(highlighted, (const QUrl &), url)

   GUI_CS_SIGNAL_1(Public, void highlighted(const QString &text))
   GUI_CS_SIGNAL_OVERLOAD(highlighted, (const QString &), text)

   GUI_CS_SIGNAL_1(Public, void anchorClicked(const QUrl &url))
   GUI_CS_SIGNAL_2(anchorClicked, url)

 protected:
   bool event(QEvent *event) override;
   void keyPressEvent(QKeyEvent *event) override;
   void mouseMoveEvent(QMouseEvent *event) override;
   void mousePressEvent(QMouseEvent *event) override;
   void mouseReleaseEvent(QMouseEvent *event) override;
   void focusOutEvent(QFocusEvent *event) override;
   bool focusNextPrevChild(bool next) override;
   void paintEvent(QPaintEvent *event) override;

 private:
   Q_DECLARE_PRIVATE(QTextBrowser)

   GUI_CS_SLOT_1(Private, void _q_documentModified())
   GUI_CS_SLOT_2(_q_documentModified)

   GUI_CS_SLOT_1(Private, void _q_activateAnchor(const QString &anchor))
   GUI_CS_SLOT_2(_q_activateAnchor)

   GUI_CS_SLOT_1(Private, void _q_highlightLink(const QString &anchor))
   GUI_CS_SLOT_2(_q_highlightLink)
};

#endif // QT_NO_TEXTBROWSER


#endif
