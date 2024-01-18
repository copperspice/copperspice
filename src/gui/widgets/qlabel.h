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

#ifndef QLABEL_H
#define QLABEL_H

#include <qframe.h>
#include <qpicture.h>

class QLabelPrivate;

class Q_GUI_EXPORT QLabel : public QFrame
{
   GUI_CS_OBJECT(QLabel)

   GUI_CS_PROPERTY_READ(text, text)
   GUI_CS_PROPERTY_WRITE(text, setText)

   GUI_CS_PROPERTY_READ(textFormat, textFormat)
   GUI_CS_PROPERTY_WRITE(textFormat, setTextFormat)

   GUI_CS_PROPERTY_READ(pixmap, pixmap)
   GUI_CS_PROPERTY_WRITE(pixmap, setPixmap)

   GUI_CS_PROPERTY_READ(scaledContents, hasScaledContents)
   GUI_CS_PROPERTY_WRITE(scaledContents, setScaledContents)

   GUI_CS_PROPERTY_READ(alignment, alignment)
   GUI_CS_PROPERTY_WRITE(alignment, setAlignment)

   GUI_CS_PROPERTY_READ(wordWrap, wordWrap)
   GUI_CS_PROPERTY_WRITE(wordWrap, setWordWrap)

   GUI_CS_PROPERTY_READ(margin, margin)
   GUI_CS_PROPERTY_WRITE(margin, setMargin)

   GUI_CS_PROPERTY_READ(indent, indent)
   GUI_CS_PROPERTY_WRITE(indent, setIndent)

   GUI_CS_PROPERTY_READ(openExternalLinks, openExternalLinks)
   GUI_CS_PROPERTY_WRITE(openExternalLinks, setOpenExternalLinks)

   GUI_CS_PROPERTY_READ(textInteractionFlags, textInteractionFlags)
   GUI_CS_PROPERTY_WRITE(textInteractionFlags, setTextInteractionFlags)

   GUI_CS_PROPERTY_READ(hasSelectedText, hasSelectedText)
   GUI_CS_PROPERTY_READ(selectedText, selectedText)

 public:
   explicit QLabel(QWidget *parent = nullptr, Qt::WindowFlags flags = Qt::EmptyFlag);
   explicit QLabel(const QString &text, QWidget *parent = nullptr, Qt::WindowFlags flags = Qt::EmptyFlag);

   QLabel(const QLabel &) = delete;
   QLabel &operator=(const QLabel &) = delete;

   ~QLabel();

   QString text() const;
   const QPixmap *pixmap() const;

#ifndef QT_NO_PICTURE
   const QPicture *picture() const;
#endif

#ifndef QT_NO_MOVIE
   QMovie *movie() const;
#endif

   Qt::TextFormat textFormat() const;
   void setTextFormat(Qt::TextFormat format);

   Qt::Alignment alignment() const;
   void setAlignment(Qt::Alignment alignment);

   void setWordWrap(bool enabled);
   bool wordWrap() const;

   int indent() const;
   void setIndent(int indent);

   int margin() const;
   void setMargin(int margin);

   bool hasScaledContents() const;
   void setScaledContents(bool enabled);
   QSize sizeHint() const override;
   QSize minimumSizeHint() const override;

#ifndef QT_NO_SHORTCUT
   void setBuddy(QWidget *buddy);
   QWidget *buddy() const;
#endif

   int heightForWidth(int width) const override;

   bool openExternalLinks() const;
   void setOpenExternalLinks(bool open);

   void setTextInteractionFlags(Qt::TextInteractionFlags flags);
   Qt::TextInteractionFlags textInteractionFlags() const;

   void setSelection(int start, int length);
   bool hasSelectedText() const;
   QString selectedText() const;
   int selectionStart() const;

   GUI_CS_SLOT_1(Public, void setText(const QString &text))
   GUI_CS_SLOT_2(setText)

   GUI_CS_SLOT_1(Public, void setPixmap(const QPixmap &pixmap))
   GUI_CS_SLOT_2(setPixmap)

#ifndef QT_NO_PICTURE
   GUI_CS_SLOT_1(Public, void setPicture(const QPicture &picture))
   GUI_CS_SLOT_2(setPicture)
#endif

#ifndef QT_NO_MOVIE
   GUI_CS_SLOT_1(Public, void setMovie(QMovie *movie))
   GUI_CS_SLOT_2(setMovie)
#endif

   GUI_CS_SLOT_1(Public, void setNum(int num))
   GUI_CS_SLOT_OVERLOAD(setNum, (int))

   GUI_CS_SLOT_1(Public, void setNum(double num))
   GUI_CS_SLOT_OVERLOAD(setNum, (double))

   GUI_CS_SLOT_1(Public, void clear())
   GUI_CS_SLOT_2(clear)

   GUI_CS_SIGNAL_1(Public, void linkActivated(const QString &link))
   GUI_CS_SIGNAL_2(linkActivated, link)

   GUI_CS_SIGNAL_1(Public, void linkHovered(const QString &link))
   GUI_CS_SIGNAL_2(linkHovered, link)

 protected:
   bool event(QEvent *event) override;
   void keyPressEvent(QKeyEvent *event) override;
   void paintEvent(QPaintEvent *event) override;
   void changeEvent(QEvent *event) override;
   void mousePressEvent(QMouseEvent *event) override;
   void mouseMoveEvent(QMouseEvent *event) override;
   void mouseReleaseEvent(QMouseEvent *event) override;
   void contextMenuEvent(QContextMenuEvent *event) override;
   void focusInEvent(QFocusEvent *event) override;
   void focusOutEvent(QFocusEvent *event) override;
   bool focusNextPrevChild(bool next) override;

 private:
   Q_DECLARE_PRIVATE(QLabel)

#ifndef QT_NO_MOVIE
   GUI_CS_SLOT_1(Private, void _q_movieUpdated(const QRect &rect))
   GUI_CS_SLOT_2(_q_movieUpdated)

   GUI_CS_SLOT_1(Private, void _q_movieResized(const QSize &size))
   GUI_CS_SLOT_2(_q_movieResized)
#endif

   GUI_CS_SLOT_1(Private, void _q_linkHovered(const QString &anchor))
   GUI_CS_SLOT_2(_q_linkHovered)

   friend class QTipLabel;
   friend class QMessageBoxPrivate;
   friend class QBalloonTip;
};

#endif
