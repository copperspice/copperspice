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

#ifndef QLABEL_P_H
#define QLABEL_P_H

#include <qlabel.h>
#include <qtextdocumentlayout_p.h>
#include <qtextcontrol_p.h>
#include <qtextdocumentfragment.h>
#include <qframe_p.h>
#include <qtextdocument.h>
#include <qmovie.h>
#include <qimage.h>
#include <qbitmap.h>
#include <qpicture.h>
#include <qmenu.h>

class QLabelPrivate : public QFramePrivate
{
   Q_DECLARE_PUBLIC(QLabel)

 public:
   QLabelPrivate();
   ~QLabelPrivate();

   void init();
   void clearContents();
   void updateLabel();
   QSize sizeForWidth(int w) const;

#ifndef QT_NO_MOVIE
   void _q_movieUpdated(const QRect &);
   void _q_movieResized(const QSize &);
#endif

#ifndef QT_NO_SHORTCUT
   void updateShortcut();
#endif

   inline bool needTextControl() const {
      return isTextLabel && (isRichText
            || (!isRichText && (textInteractionFlags & (Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard))));
   }

   void ensureTextPopulated() const;
   void ensureTextLayouted() const;
   void ensureTextControl() const;
   void sendControlEvent(QEvent *event);

   void _q_linkHovered(const QString &link);

   QRectF layoutRect() const;
   QRect documentRect() const;
   QPoint layoutPoint(const QPoint &p) const;
   Qt::LayoutDirection textDirection() const;

#ifndef QT_NO_CONTEXTMENU
   QMenu *createStandardContextMenu(const QPoint &pos);
#endif

#ifndef QT_NO_PICTURE
   QPicture *picture;
#endif

#ifndef QT_NO_MOVIE
   QPointer<QMovie> movie;
#endif

#ifndef QT_NO_CURSOR
   QCursor cursor;
#endif

#ifndef QT_NO_SHORTCUT
   QPointer<QWidget> buddy;
   int shortcutId;
#endif

   Qt::TextFormat textformat;
   Qt::TextInteractionFlags textInteractionFlags;
   mutable QSizePolicy sizePolicy;
   int margin;

   ushort align;
   short indent;
   mutable uint valid_hints : 1;
   uint scaledcontents : 1;
   mutable uint textLayoutDirty : 1;
   mutable uint textDirty : 1;
   mutable uint isRichText : 1;
   mutable uint isTextLabel : 1;
   mutable uint hasShortcut : 1;

   mutable QTextControl *control;
   mutable QTextCursor shortcutCursor;

   mutable QSize sh;
   mutable QSize msh;
   QString text;
   QPixmap  *pixmap;
   QPixmap *scaledpixmap;
   QImage *cachedimage;

#ifndef QT_NO_CURSOR
   uint validCursor : 1;
   uint onAnchor : 1;
#endif

   uint openExternalLinks : 1;

   friend class QMessageBoxPrivate;
};

#endif
