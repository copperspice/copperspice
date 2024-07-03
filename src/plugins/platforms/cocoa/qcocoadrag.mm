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

#include <qcocoadrag.h>
#include <qmacclipboard.h>
#include <qcocoahelpers.h>
#include <qwidget.h>

static constexpr const int dragImageMaxChars = 26;

QCocoaDrag::QCocoaDrag()
   : m_drag(nullptr)
{
   m_lastEvent = nullptr;
   m_lastView  = nullptr;
}

QCocoaDrag::~QCocoaDrag()
{
   [m_lastEvent release];
}

void QCocoaDrag::setLastMouseEvent(NSEvent *event, NSView *view)
{
   [m_lastEvent release];
   m_lastEvent = [event copy];
   m_lastView = view;
}

QMimeData *QCocoaDrag::platformDropData()
{
   if (m_drag) {
      return m_drag->mimeData();
   }

   return nullptr;
}

Qt::DropAction QCocoaDrag::defaultAction(Qt::DropActions possibleActions,
   Qt::KeyboardModifiers modifiers) const
{
   Qt::DropAction default_action = Qt::IgnoreAction;

   if (currentDrag()) {
      default_action = currentDrag()->defaultAction();
      possibleActions = currentDrag()->supportedActions();
   }

   if (default_action == Qt::IgnoreAction) {
      //This means that the drag was initiated by QDrag::start and we need to
      //preserve the old behavior
      default_action = Qt::CopyAction;
   }

   if (modifiers & Qt::ControlModifier && modifiers & Qt::AltModifier) {
      default_action = Qt::LinkAction;
   } else if (modifiers & Qt::AltModifier) {
      default_action = Qt::CopyAction;
   } else if (modifiers & Qt::ControlModifier) {
      default_action = Qt::MoveAction;
   }

   // Check if the action determined is allowed
   if (!(possibleActions & default_action)) {
      if (possibleActions & Qt::CopyAction) {
         default_action = Qt::CopyAction;
      } else if (possibleActions & Qt::MoveAction) {
         default_action = Qt::MoveAction;
      } else if (possibleActions & Qt::LinkAction) {
         default_action = Qt::LinkAction;
      } else {
         default_action = Qt::IgnoreAction;
      }
   }

   return default_action;
}


Qt::DropAction QCocoaDrag::drag(QDrag *o)
{
   m_drag = o;
   m_executed_drop_action = Qt::IgnoreAction;

   QPoint hotSpot = m_drag->hotSpot();
   QPixmap pm = dragPixmap(m_drag, hotSpot);
   QSize pmDeviceIndependentSize = pm.size() / pm.devicePixelRatio();
   NSImage *nsimage = qt_mac_create_nsimage(pm);
   [nsimage setSize: qt_mac_toNSSize(pmDeviceIndependentSize)];

   QMacPasteboard dragBoard((CFStringRef) NSDragPboard, QMacInternalPasteboardMime::MIME_DND);
   m_drag->mimeData()->setData(QLatin1String("application/x-qt-mime-type-name"), QByteArray("dummy"));
   dragBoard.setMimeData(m_drag->mimeData(), QMacPasteboard::LazyRequest);

   NSPoint event_location = [m_lastEvent locationInWindow];
   NSWindow *theWindow = [m_lastEvent window];
   Q_ASSERT(theWindow != nil);
   event_location.x -= hotSpot.x();
   CGFloat flippedY = pmDeviceIndependentSize.height() - hotSpot.y();
   event_location.y -= flippedY;
   NSSize mouseOffset_unused = NSMakeSize(0.0, 0.0);
   NSPasteboard *pboard = [NSPasteboard pasteboardWithName: NSDragPboard];

   [theWindow dragImage: nsimage
                     at: event_location
                 offset: mouseOffset_unused
                  event: m_lastEvent
             pasteboard: pboard
                 source: m_lastView
              slideBack: YES];

   [nsimage release];

   m_drag = nullptr;

   return m_executed_drop_action;
}

void QCocoaDrag::setAcceptedAction(Qt::DropAction act)
{
   m_executed_drop_action = act;
}

QPixmap QCocoaDrag::dragPixmap(QDrag *drag, QPoint &hotSpot) const
{
   const QMimeData *data = drag->mimeData();
   QPixmap pm = drag->pixmap();

   if (pm.isNull()) {
      QFont f(qApp->font());
      f.setPointSize(12);
      QFontMetrics fm(f);

      if (data->hasImage()) {
         const QImage img = data->imageData().value<QImage>();
         if (!img.isNull()) {
            pm = QPixmap::fromImage(img).scaledToWidth(dragImageMaxChars * fm.averageCharWidth());
         }
      }

      if (pm.isNull() && (data->hasText() || data->hasUrls()) ) {
         QString s = data->hasText() ? data->text() : data->urls().first().toString();

         if (s.length() > dragImageMaxChars) {
            s = s.left(dragImageMaxChars - 3) + QChar(0x2026);
         }

         if (!s.isEmpty()) {
            const int width = fm.width(s);
            const int height = fm.height();

            if (width > 0 && height > 0) {
               qreal dpr = 1.0;

               if (const QWindow *sourceWindow = qobject_cast<QWindow *>(drag->source())) {
                  dpr = sourceWindow->devicePixelRatio();

               } else if (const QWidget *sourceWidget = qobject_cast<QWidget *>(drag->source())) {
                  if (const QWindow *sourceWindow = sourceWidget->window()->windowHandle()) {
                     dpr = sourceWindow->devicePixelRatio();
                  }
               }

               pm = QPixmap(width * dpr, height * dpr);
               pm.setDevicePixelRatio(dpr);

               QPainter p(&pm);
               p.fillRect(0, 0, pm.width(), pm.height(), Qt::color0);
               p.setPen(Qt::color1);
               p.setFont(f);
               p.drawText(0, fm.ascent(), s);
               p.end();
               hotSpot = QPoint(pm.width() / 2, pm.height() / 2);
            }
         }
      }
   }

   if (pm.isNull()) {
      pm = defaultPixmap();
   }

   return pm;
}

QCocoaDropData::QCocoaDropData(NSPasteboard *pasteboard)
{
   dropPasteboard = reinterpret_cast<CFStringRef>(const_cast<const NSString *>([pasteboard name]));
   CFRetain(dropPasteboard);
}

QCocoaDropData::~QCocoaDropData()
{
   CFRelease(dropPasteboard);
}

QStringList QCocoaDropData::formats_sys() const
{
   QStringList formats;
   PasteboardRef board;

   if (PasteboardCreate(dropPasteboard, &board) != noErr) {

#if defined(CS_SHOW_DEBUG_PLATFORM_PASTEBOARD)
      qDebug("QCocoaDropData::formats_sys() Unable to create a PasteBoard");
#endif

      return formats;
   }

   formats = QMacPasteboard(board, QMacInternalPasteboardMime::MIME_DND).formats();
   return formats;
}

QVariant QCocoaDropData::retrieveData_sys(const QString &mimeType, QVariant::Type type) const
{
   QVariant data;
   PasteboardRef board;

   if (PasteboardCreate(dropPasteboard, &board) != noErr) {

#if defined(CS_SHOW_DEBUG_PLATFORM_PASTEBOARD)
      qDebug("QCocoaDropData::retrieveData_sys() Unable to create a PasteBoard");
#endif

      return data;
   }

   data = QMacPasteboard(board, QMacInternalPasteboardMime::MIME_DND).retrieveData(mimeType, type);
   CFRelease(board);
   return data;
}

bool QCocoaDropData::hasFormat_sys(const QString &mimeType) const
{
   bool has = false;
   PasteboardRef board;

   if (PasteboardCreate(dropPasteboard, &board) != noErr) {

#if defined(CS_SHOW_DEBUG_PLATFORM_PASTEBOARD)
      qDebug("QCocoaDropData::hasFormat_sys() Unable to create a PasteBoard");
#endif

      return has;
   }

   has = QMacPasteboard(board, QMacInternalPasteboardMime::MIME_DND).hasFormat(mimeType);
   CFRelease(board);

   return has;
}
