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

#ifndef QCLIPBOARD_H
#define QCLIPBOARD_H

#include <qobject.h>
#include <qscopedpointer.h>

#ifndef QT_NO_CLIPBOARD

class QImage;
class QMimeData;
class QPixmap;

class Q_GUI_EXPORT QClipboard : public QObject
{
   GUI_CS_OBJECT(QClipboard)

 public:
   enum Mode {
      Clipboard,
      Selection,
      FindBuffer,
      LastMode = FindBuffer
   };

   QClipboard(const QClipboard &) = delete;
   QClipboard &operator=(const QClipboard &) = delete;

   void clear(Mode mode = Clipboard);

   bool supportsSelection() const;
   bool supportsFindBuffer() const;

   bool ownsSelection() const;
   bool ownsClipboard() const;
   bool ownsFindBuffer() const;

   QString text(Mode mode = Clipboard) const;
   QString text(QString &subtype, Mode mode = Clipboard) const;
   void setText(const QString &text, Mode mode = Clipboard);

   const QMimeData *mimeData(Mode mode = Clipboard ) const;
   void setMimeData(QMimeData *data, Mode mode = Clipboard);

   QImage image(Mode mode = Clipboard) const;
   QPixmap pixmap(Mode mode = Clipboard) const;
   void setImage(const QImage &image, Mode mode  = Clipboard);
   void setPixmap(const QPixmap &pixmap, Mode mode  = Clipboard);

   GUI_CS_SIGNAL_1(Public, void changed(QClipboard::Mode mode))
   GUI_CS_SIGNAL_2(changed, mode)

   GUI_CS_SIGNAL_1(Public, void selectionChanged())
   GUI_CS_SIGNAL_2(selectionChanged)

   GUI_CS_SIGNAL_1(Public, void findBufferChanged())
   GUI_CS_SIGNAL_2(findBufferChanged)

   GUI_CS_SIGNAL_1(Public, void dataChanged())
   GUI_CS_SIGNAL_2(dataChanged)

 protected:
   friend class QApplication;
   friend class QApplicationPrivate;
   friend class QBaseApplication;
   friend class QDragManager;
   friend class QPlatformClipboard;

 private:
   explicit QClipboard(QObject *parent);
   ~QClipboard();

   bool supportsMode(Mode mode) const;
   bool ownsMode(Mode mode) const;
   void emitChanged(Mode mode);
};

#endif // QT_NO_CLIPBOARD

#endif
