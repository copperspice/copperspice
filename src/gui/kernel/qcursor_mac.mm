/***********************************************************************
*
* Copyright (c) 2012-2019 Barbara Geller
* Copyright (c) 2012-2019 Ansel Sermersheim
*
* Copyright (C) 2015 The Qt Company Ltd.
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

#include <qcursor_p.h>
#include <qpixmap_mac_p.h>
#include <qapplication.h>
#include <qbitmap.h>
#include <qcursor.h>
#include <qevent.h>
#include <string.h>
#include <unistd.h>
#include <AppKit/NSCursor.h>
#include <qpainter.h>
#include <qt_cocoa_helpers_mac_p.h>
#include <qapplication_p.h>

QT_BEGIN_NAMESPACE

static QCursorData *currentCursor = 0;
static QPointer<QWidget> lastWidgetUnderMouse = 0;
static QPointer<QWidget> lastMouseCursorWidget = 0;
static bool qt_button_down_on_prev_call = false;
static QCursor *grabCursor = 0;
static int nextCursorId = Qt::BitmapCursor;

extern QCursorData *qt_cursorTable[Qt::LastCursor + 1];
extern OSWindowRef qt_mac_window_for(const QWidget *);     // qwidget_mac.cpp
extern GrafPtr qt_mac_qd_context(const QPaintDevice *);    // qpaintdevice_mac.cpp
extern bool qt_sendSpontaneousEvent(QObject *, QEvent *);  // qapplication_mac.cpp
extern QWidget *qt_button_down;                            // qapplication_mac.cpp

inline void *qt_mac_nsCursorForQCursor(const QCursor &c)
{
   c.d->update();
   return [[static_cast<NSCursor *>(c.d->curs.cp.nscursor) retain] autorelease];
}

void qt_mac_set_cursor(const QCursor *c)
{
   QMacCocoaAutoReleasePool pool;
   [static_cast<NSCursor *>(qt_mac_nsCursorForQCursor(*c)) set];
}

void qt_mac_updateCursorWithWidgetUnderMouse(QWidget *widgetUnderMouse)
{
   QCursor cursor(Qt::ArrowCursor);
   if (qt_button_down) {
      // The widget that is currently pressed grabs the mouse cursor
      widgetUnderMouse = qt_button_down;
      qt_button_down_on_prev_call = true;

   } else if (qt_button_down_on_prev_call) {
      // Grab has been released, so do a full check
      qt_button_down_on_prev_call = false;
      lastWidgetUnderMouse = 0;
      lastMouseCursorWidget = 0;
   }

   if (QApplication::overrideCursor()) {
      cursor = *QApplication::overrideCursor();

   } else if (grabCursor) {
      cursor = *grabCursor;

   } else if (widgetUnderMouse) {
      if (widgetUnderMouse == lastWidgetUnderMouse) {
         // Optimization that should hit when the widget under the mouse does not change as the mouse moves:
         if (lastMouseCursorWidget) {
            cursor = lastMouseCursorWidget->cursor();
         }

      } else {
         QWidget *w = widgetUnderMouse;

         for (; w; w = w->parentWidget()) {
            if (w->testAttribute(Qt::WA_SetCursor)) {
               cursor = w->cursor();
               break;
            }
            if (w->isWindow()) {
               break;
            }
         }

         // One final check in case we ran out of parents in the loop:
         if (w && !w->testAttribute(Qt::WA_SetCursor)) {
            w = 0;
         }

         lastWidgetUnderMouse = widgetUnderMouse;
         lastMouseCursorWidget = w;
      }
   }

   cursor.d->update();
   NSCursor *nsCursor = static_cast<NSCursor *>(cursor.d->curs.cp.nscursor);

   if ([NSCursor currentCursor] != nsCursor) {
      QMacCocoaAutoReleasePool pool;
      [nsCursor set];
   }
}

void qt_mac_update_cursor()
{
   // This function is similar to qt_mac_updateCursorWithWidgetUnderMouse
   // except that is clears the optimization cache, and finds the widget
   // under mouse itself. Clearing the cache is useful in cases where the
   // application has been deactivated/activated

   // NB: since we dont have any true native widget, the call to
   // qt_mac_getTargetForMouseEvent will fail when the mouse is over QMacNativeWidgets.

   lastWidgetUnderMouse = 0;
   lastMouseCursorWidget = 0;
   QWidget *widgetUnderMouse = 0;

   if (qt_button_down) {
      widgetUnderMouse = qt_button_down;

   } else {
      QPoint localPoint;
      QPoint globalPoint;
      qt_mac_getTargetForMouseEvent(0, QEvent::None, localPoint, globalPoint, 0, &widgetUnderMouse);
   }
   qt_mac_updateCursorWithWidgetUnderMouse(widgetUnderMouse);

}

void qt_mac_setMouseGrabCursor(bool set, QCursor *const cursor = 0)
{
   if (grabCursor) {
      delete grabCursor;
      grabCursor = 0;
   }
   if (set) {
      if (cursor) {
         grabCursor = new QCursor(*cursor);
      } else if (lastMouseCursorWidget) {
         grabCursor = new QCursor(lastMouseCursorWidget->cursor());
      } else {
         grabCursor = new QCursor(Qt::ArrowCursor);
      }
   }
   qt_mac_update_cursor();
}

QCursorData::QCursorData(Qt::CursorShape s)
   : cshape(s), bm(0), bmm(0), hx(-1), hy(-1), mId(s), type(TYPE_None)
{
   ref = 1;
   memset(&curs, '\0', sizeof(curs));
}

QCursorData::~QCursorData()
{
   if (type == TYPE_ImageCursor) {

      if (curs.cp.my_cursor) {
         QMacCocoaAutoReleasePool pool;
         [static_cast<NSCursor *>(curs.cp.nscursor) release];
      }
   }

   type = TYPE_None;

   delete bm;
   delete bmm;

   if (currentCursor == this) {
      currentCursor = 0;
   }
}

QCursorData *QCursorData::setBitmap(const QBitmap &bitmap, const QBitmap &mask, int hotX, int hotY)
{
   if (!QCursorData::initialized) {
      QCursorData::initialize();
   }

   if (bitmap.depth() != 1 || mask.depth() != 1 || bitmap.size() != mask.size()) {
      qWarning("Qt: QCursor: Cannot create bitmap cursor; invalid bitmap(s)");
      QCursorData *c = qt_cursorTable[0];
      c->ref.ref();
      return c;
   }

   // This is silly, but this is apparently called outside the constructor, so we have to be ready

   QCursorData *x = new QCursorData;
   x->ref = 1;
   x->mId = ++nextCursorId;
   x->bm  = new QBitmap(bitmap);
   x->bmm = new QBitmap(mask);
   x->cshape = Qt::BitmapCursor;
   x->hx = hotX >= 0 ? hotX : bitmap.width() / 2;
   x->hy = hotY >= 0 ? hotY : bitmap.height() / 2;

   return x;
}

Qt::HANDLE QCursor::handle() const
{
   if (!QCursorData::initialized) {
      QCursorData::initialize();
   }

   if (d->type == QCursorData::TYPE_None) {
      d->update();
   }

   return (Qt::HANDLE)d->mId;
}

QPoint QCursor::pos()
{
   return flipPoint([NSEvent mouseLocation]).toPoint();
}

void QCursor::setPos(int x, int y)
{
   CGPoint pos;
   pos.x = x;
   pos.y = y;

   CGEventRef e = CGEventCreateMouseEvent(0, kCGEventMouseMoved, pos, kCGMouseButtonLeft);
   CGEventPost(kCGHIDEventTap, e);
   CFRelease(e);
}

void QCursorData::initCursorFromBitmap()
{
   NSImage *nsimage;
   QImage finalCursor(bm->size(), QImage::Format_ARGB32);
   QImage bmi = bm->toImage().convertToFormat(QImage::Format_RGB32);
   QImage bmmi = bmm->toImage().convertToFormat(QImage::Format_RGB32);

   for (int row = 0; row < finalCursor.height(); ++row) {
      QRgb *bmData = reinterpret_cast<QRgb *>(bmi.scanLine(row));
      QRgb *bmmData = reinterpret_cast<QRgb *>(bmmi.scanLine(row));
      QRgb *finalData = reinterpret_cast<QRgb *>(finalCursor.scanLine(row));

      for (int col = 0; col < finalCursor.width(); ++col) {
         if (bmmData[col] == 0xff000000 && bmData[col] == 0xffffffff) {
            finalData[col] = 0xffffffff;
         } else if (bmmData[col] == 0xff000000 && bmData[col] == 0xffffffff) {
            finalData[col] = 0x7f000000;
         } else if (bmmData[col] == 0xffffffff && bmData[col] == 0xffffffff) {
            finalData[col] = 0x00000000;
         } else {
            finalData[col] = 0xff000000;
         }
      }
   }

   type = QCursorData::TYPE_ImageCursor;
   curs.cp.my_cursor = true;

   QPixmap bmCopy = QPixmap::fromImage(finalCursor);
   NSPoint hotSpot = { static_cast<CGFloat>(hx), static_cast<CGFloat>(hy) };
   nsimage = static_cast<NSImage *>(qt_mac_create_nsimage(bmCopy));
   curs.cp.nscursor = [[NSCursor alloc] initWithImage: nsimage hotSpot: hotSpot];
   [nsimage release];
}

void QCursorData::initCursorFromPixmap()
{
   type = QCursorData::TYPE_ImageCursor;
   curs.cp.my_cursor = true;
   NSPoint hotSpot = { static_cast<CGFloat>(hx), static_cast<CGFloat>(hy) };

   NSImage *nsimage;
   nsimage = static_cast<NSImage *>(qt_mac_create_nsimage(pixmap));

   curs.cp.nscursor = [[NSCursor alloc] initWithImage: nsimage hotSpot: hotSpot];
   [nsimage release];
}

void QCursorData::update()
{
   if (!QCursorData::initialized) {
      QCursorData::initialize();
   }

   if (type != QCursorData::TYPE_None) {
      return;
   }

   /* Note to self... ***
    * mask x data
    * 0xFF x 0x00 == fully opaque white
    * 0x00 x 0xFF == xor'd black
    * 0xFF x 0xFF == fully opaque black
    * 0x00 x 0x00 == fully transparent
    */

   if (hx < 0) {
      hx = 0;
   }
   if (hy < 0) {
      hy = 0;
   }

#define QT_USE_APPROXIMATE_CURSORS
#ifdef QT_USE_APPROXIMATE_CURSORS
   static const uchar cur_ver_bits[] = {
      0x00, 0x00, 0x00, 0x00, 0x01, 0x80, 0x03, 0xc0, 0x07, 0xe0, 0x0f, 0xf0,
      0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x0f, 0xf0,
      0x07, 0xe0, 0x03, 0xc0, 0x01, 0x80, 0x00, 0x00
   };
   static const uchar mcur_ver_bits[] = {
      0x00, 0x00, 0x03, 0x80, 0x07, 0xc0, 0x0f, 0xe0, 0x1f, 0xf0, 0x3f, 0xf8,
      0x7f, 0xfc, 0x07, 0xc0, 0x07, 0xc0, 0x07, 0xc0, 0x7f, 0xfc, 0x3f, 0xf8,
      0x1f, 0xf0, 0x0f, 0xe0, 0x07, 0xc0, 0x03, 0x80
   };

   static const uchar cur_hor_bits[] = {
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x20, 0x18, 0x30,
      0x38, 0x38, 0x7f, 0xfc, 0x7f, 0xfc, 0x38, 0x38, 0x18, 0x30, 0x08, 0x20,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
   };
   static const uchar mcur_hor_bits[] = {
      0x00, 0x00, 0x00, 0x00, 0x04, 0x40, 0x0c, 0x60, 0x1c, 0x70, 0x3c, 0x78,
      0x7f, 0xfc, 0xff, 0xfe, 0xff, 0xfe, 0xff, 0xfe, 0x7f, 0xfc, 0x3c, 0x78,
      0x1c, 0x70, 0x0c, 0x60, 0x04, 0x40, 0x00, 0x00
   };

   static const uchar cur_fdiag_bits[] = {
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xf8, 0x00, 0xf8, 0x00, 0x78,
      0x00, 0xf8, 0x01, 0xd8, 0x23, 0x88, 0x37, 0x00, 0x3e, 0x00, 0x3c, 0x00,
      0x3e, 0x00, 0x3f, 0x00, 0x00, 0x00, 0x00, 0x00
   };
   static const uchar mcur_fdiag_bits[] = {
      0x00, 0x00, 0x00, 0x00, 0x07, 0xfc, 0x03, 0xfc, 0x01, 0xfc, 0x00, 0xfc,
      0x41, 0xfc, 0x63, 0xfc, 0x77, 0xdc, 0x7f, 0x8c, 0x7f, 0x04, 0x7e, 0x00,
      0x7f, 0x00, 0x7f, 0x80, 0x7f, 0xc0, 0x00, 0x00
   };

   static const uchar cur_bdiag_bits[] = {
      0x00, 0x00, 0x00, 0x00, 0x3f, 0x00, 0x3e, 0x00, 0x3c, 0x00, 0x3e, 0x00,
      0x37, 0x00, 0x23, 0x88, 0x01, 0xd8, 0x00, 0xf8, 0x00, 0x78, 0x00, 0xf8,
      0x01, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
   };
   static const uchar mcur_bdiag_bits[] = {
      0x00, 0x00, 0x7f, 0xc0, 0x7f, 0x80, 0x7f, 0x00, 0x7e, 0x00, 0x7f, 0x04,
      0x7f, 0x8c, 0x77, 0xdc, 0x63, 0xfc, 0x41, 0xfc, 0x00, 0xfc, 0x01, 0xfc,
      0x03, 0xfc, 0x07, 0xfc, 0x00, 0x00, 0x00, 0x00
   };

   static const unsigned char cur_up_arrow_bits[] = {
      0x00, 0x80, 0x01, 0x40, 0x01, 0x40, 0x02, 0x20, 0x02, 0x20, 0x04, 0x10,
      0x04, 0x10, 0x08, 0x08, 0x0f, 0x78, 0x01, 0x40, 0x01, 0x40, 0x01, 0x40,
      0x01, 0x40, 0x01, 0x40, 0x01, 0x40, 0x01, 0xc0
   };
   static const unsigned char mcur_up_arrow_bits[] = {
      0x00, 0x80, 0x01, 0xc0, 0x01, 0xc0, 0x03, 0xe0, 0x03, 0xe0, 0x07, 0xf0,
      0x07, 0xf0, 0x0f, 0xf8, 0x0f, 0xf8, 0x01, 0xc0, 0x01, 0xc0, 0x01, 0xc0,
      0x01, 0xc0, 0x01, 0xc0, 0x01, 0xc0, 0x01, 0xc0
   };
#endif
   const uchar *cursorData = 0;
   const uchar *cursorMaskData = 0;

   switch (cshape) {                        // map Q cursor to MAC cursor
      case Qt::BitmapCursor: {
         if (pixmap.isNull()) {
            initCursorFromBitmap();
         } else {
            initCursorFromPixmap();
         }
         break;
      }
      case Qt::BlankCursor: {
         pixmap = QPixmap(16, 16);
         pixmap.fill(Qt::transparent);
         initCursorFromPixmap();
         break;
      }
      case Qt::ArrowCursor: {
         type = QCursorData::TYPE_ThemeCursor;
         curs.cp.nscursor = [NSCursor arrowCursor];
         break;
      }
      case Qt::CrossCursor: {
         type = QCursorData::TYPE_ThemeCursor;
         curs.cp.nscursor = [NSCursor crosshairCursor];
         break;
      }
      case Qt::WaitCursor: {
         pixmap = QPixmap(QLatin1String(":/copperspice/mac/cursors/images/spincursor.png"));
         initCursorFromPixmap();
         break;
      }
      case Qt::IBeamCursor: {
         type = QCursorData::TYPE_ThemeCursor;
         curs.cp.nscursor = [NSCursor IBeamCursor];
         break;
      }
      case Qt::SizeAllCursor: {
         pixmap = QPixmap(QLatin1String(":/copperspice/mac/cursors/images/pluscursor.png"));
         initCursorFromPixmap();
         break;
      }
      case Qt::WhatsThisCursor: { //for now just use the pointing hand
         case Qt::PointingHandCursor:
            type = QCursorData::TYPE_ThemeCursor;
            curs.cp.nscursor = [NSCursor pointingHandCursor];
            break;
         }
      case Qt::BusyCursor: {
         pixmap = QPixmap(QLatin1String(":/copperspice/mac/cursors/images/waitcursor.png"));
         initCursorFromPixmap();
         break;
      }
      case Qt::SplitVCursor: {
         type = QCursorData::TYPE_ThemeCursor;
         curs.cp.nscursor = [NSCursor resizeUpDownCursor];
         break;
      }
      case Qt::SplitHCursor: {
         type = QCursorData::TYPE_ThemeCursor;
         curs.cp.nscursor = [NSCursor resizeLeftRightCursor];
         break;
      }
      case Qt::ForbiddenCursor: {
         pixmap = QPixmap(QLatin1String(":/copperspice/mac/cursors/images/forbiddencursor.png"));
         initCursorFromPixmap();
         break;
      }
      case Qt::OpenHandCursor:
         type = QCursorData::TYPE_ThemeCursor;
         curs.cp.nscursor = [NSCursor openHandCursor];
         break;
      case Qt::ClosedHandCursor:
         type = QCursorData::TYPE_ThemeCursor;
         curs.cp.nscursor = [NSCursor closedHandCursor];
         break;
      case Qt::DragCopyCursor:
         type = QCursorData::TYPE_ThemeCursor;
         if ([NSCursor respondsToSelector: @selector(dragCopyCursor)]) {
            curs.cp.nscursor = [NSCursor performSelector: @selector(dragCopyCursor)];
         }
         break;
      case Qt::DragMoveCursor:
         type = QCursorData::TYPE_ThemeCursor;
         curs.cp.nscursor = [NSCursor arrowCursor];
         break;
      case Qt::DragLinkCursor:
         type = QCursorData::TYPE_ThemeCursor;
         if ([NSCursor respondsToSelector: @selector(dragLinkCursor)]) {
            curs.cp.nscursor = [NSCursor performSelector: @selector(dragLinkCursor)];
         }
         break;

#define QT_USE_APPROXIMATE_CURSORS
#ifdef QT_USE_APPROXIMATE_CURSORS
      case Qt::SizeVerCursor:
         cursorData = cur_ver_bits;
         cursorMaskData = mcur_ver_bits;
         hx = hy = 8;
         break;
      case Qt::SizeHorCursor:
         cursorData = cur_hor_bits;
         cursorMaskData = mcur_hor_bits;
         hx = hy = 8;
         break;
      case Qt::SizeBDiagCursor:
         cursorData = cur_fdiag_bits;
         cursorMaskData = mcur_fdiag_bits;
         hx = hy = 8;
         break;
      case Qt::SizeFDiagCursor:
         cursorData = cur_bdiag_bits;
         cursorMaskData = mcur_bdiag_bits;
         hx = hy = 8;
         break;
      case Qt::UpArrowCursor:
         cursorData = cur_up_arrow_bits;
         cursorMaskData = mcur_up_arrow_bits;
         hx = 8;
         break;
#endif

      default:
         qWarning("QCursor::update() Invalid cursor shape %d", cshape);
         return;
   }

   if (cursorData) {
      bm  = new QBitmap(QBitmap::fromData(QSize(16, 16), cursorData, QImage::Format_Mono));
      bmm = new QBitmap(QBitmap::fromData(QSize(16, 16), cursorMaskData, QImage::Format_Mono));
      initCursorFromBitmap();
   }
}

QT_END_NAMESPACE
