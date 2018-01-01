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

QT_BEGIN_NAMESPACE

class QLabelPrivate : public QFramePrivate
{
   Q_DECLARE_PUBLIC(QLabel)

 public:
   QLabelPrivate() {}

   void init();
   void clearContents();
   void updateLabel();
   QSize sizeForWidth(int w) const;

   mutable QSize sh;
   mutable QSize msh;
   mutable bool valid_hints;
   mutable QSizePolicy sizePolicy;
   int margin;
   QString text;
   QPixmap  *pixmap;
   QPixmap *scaledpixmap;
   QImage *cachedimage;

#ifndef QT_NO_PICTURE
   QPicture *picture;
#endif

#ifndef QT_NO_MOVIE
   QPointer<QMovie> movie;
   void _q_movieUpdated(const QRect &);
   void _q_movieResized(const QSize &);
#endif

#ifndef QT_NO_SHORTCUT
   void updateShortcut();
#endif

#ifndef QT_NO_SHORTCUT
   QPointer<QWidget> buddy;
   int shortcutId;
#endif

   ushort align;
   short indent;
   uint scaledcontents : 1;
   mutable uint textLayoutDirty : 1;
   mutable uint textDirty : 1;
   mutable uint isRichText : 1;
   mutable uint isTextLabel : 1;
   mutable uint hasShortcut : 1;
   Qt::TextFormat textformat;
   mutable QTextControl *control;
   mutable QTextCursor shortcutCursor;
   Qt::TextInteractionFlags textInteractionFlags;

   inline bool needTextControl() const {
      return isTextLabel && (isRichText
                 || (!isRichText && (textInteractionFlags & (Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard))));
   }

   void ensureTextPopulated() const;
   void ensureTextLayouted() const;
   void ensureTextControl() const;
   void sendControlEvent(QEvent *e);

   void _q_linkHovered(const QString &link);

   QRectF layoutRect() const;
   QRect documentRect() const;
   QPoint layoutPoint(const QPoint &p) const;
   Qt::LayoutDirection textDirection() const;

#ifndef QT_NO_CONTEXTMENU
   QMenu *createStandardContextMenu(const QPoint &pos);
#endif

   bool openExternalLinks;

#ifndef QT_NO_CURSOR
   uint validCursor : 1;
   uint onAnchor : 1;
   QCursor cursor;
#endif

   friend class QMessageBoxPrivate;
};

QT_END_NAMESPACE

#endif // QLABEL_P_H
