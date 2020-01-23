/***********************************************************************
*
* Copyright (c) 2012-2020 Barbara Geller
* Copyright (c) 2012-2020 Ansel Sermersheim
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

#ifndef QPRINTENGINE_WIN_P_H
#define QPRINTENGINE_WIN_P_H

#include <qglobal.h>
#ifndef QT_NO_PRINTER


#include <qpaintengine.h>
#include <qpagelayout.h>
#include <qprinter.h>
#include <qprintengine.h>
#include <qt_windows.h>

#include <qpaintengine_alpha_p.h>
#include <qprintdevice_p.h>

QT_BEGIN_NAMESPACE

class QWin32PrintEnginePrivate;
class QPrinterPrivate;
class QPainterState;

class Q_GUI_EXPORT QWin32PrintEngine : public QAlphaPaintEngine, public QPrintEngine
{
   Q_DECLARE_PRIVATE(QWin32PrintEngine)

 public:
   QWin32PrintEngine(QPrinter::PrinterMode mode);

   // override QWin32PaintEngine
   bool begin(QPaintDevice *dev) override;
   bool end() override;

   void updateState(const QPaintEngineState &state) override;

   void updateMatrix(const QTransform &matrix);
   void updateClipPath(const QPainterPath &clip, Qt::ClipOperation op);

   void drawPath(const QPainterPath &path) override;
   void drawPolygon(const QPointF *points, int pointCount, PolygonDrawMode mode) override;
   void drawTextItem(const QPointF &p, const QTextItem &textItem) override;

   void drawPixmap(const QRectF &r, const QPixmap &pm, const QRectF &sr) override;
   void drawTiledPixmap(const QRectF &r, const QPixmap &pm, const QPointF &p) override;
   void setProperty(PrintEnginePropertyKey key, const QVariant &value) override;
   QVariant property(PrintEnginePropertyKey key) const override;

   bool newPage() override;
   bool abort() override;
   int metric(QPaintDevice::PaintDeviceMetric) const override;

   QPrinter::PrinterState printerState() const override;

   QPaintEngine::Type type() const override {
      return Windows;
   }

   HDC getDC() const;
   void releaseDC(HDC) const;

   /* Used by print/page setup dialogs */
   void setGlobalDevMode(HGLOBAL globalDevNames, HGLOBAL globalDevMode);
   HGLOBAL *createGlobalDevNames();
   HGLOBAL globalDevMode();

 private:
   friend class QPrintDialog;
   friend class QPageSetupDialog;
};

class QWin32PrintEnginePrivate : public QAlphaPaintEnginePrivate
{
   Q_DECLARE_PUBLIC(QWin32PrintEngine)

 public:
   QWin32PrintEnginePrivate() :
      hPrinter(0),
      globalDevMode(0),
      devMode(0),
      pInfo(0),
      hMem(0),
      hdc(0),
      ownsDevMode(false),
      mode(QPrinter::ScreenResolution),
      state(QPrinter::Idle),
      resolution(0),
      m_pageLayout(QPageLayout(QPageSize(QPageSize::A4), QPageLayout::Portrait, QMarginsF(0, 0, 0, 0))),
      stretch_x(1), stretch_y(1), origin_x(0), origin_y(0),
      dpi_x(96), dpi_y(96), dpi_display(96),
      num_copies(1),
      printToFile(false),

      reinit(false),
      complex_xform(false), has_pen(false), has_brush(false), has_custom_paper_size(false),
      embed_fonts(true),
      txop(0 /* QTransform::TxNone */) {
   }

   ~QWin32PrintEnginePrivate();


   /* Initializes the printer data based on the current printer name. This
      function creates a DEVMODE struct, HDC and a printer handle. If these
      structures are already in use, they are freed using release
   */
   void initialize();

   /* Initializes data in the print engine whenever the HDC has been renewed
   */
   void initHDC();

   /* Releases all the handles the printer currently holds, HDC, DEVMODE,
      etc and resets the corresponding members to 0. */
   void release();

   /* Resets the DC with changes in devmode. If the printer is active
      this function only sets the reinit variable to true so it
      is handled in the next begin or newpage. */
   void doReinit();


   bool resetDC();

   void strokePath(const QPainterPath &path, const QColor &color);
   void fillPath(const QPainterPath &path, const QColor &color);

   void composeGdiPath(const QPainterPath &path);
   void fillPath_dev(const QPainterPath &path, const QColor &color);
   void strokePath_dev(const QPainterPath &path, const QColor &color, qreal width);

   void setPageSize(const QPageSize &pageSize);
   void updatePageLayout();

   void updateMetrics();
   void debugMetrics() const;

   // Windows GDI printer references.
   HANDLE hPrinter;

   HGLOBAL globalDevMode;
   DEVMODE *devMode;
   PRINTER_INFO_2 *pInfo;
   HGLOBAL hMem;

   HDC hdc;

   // True if devMode was allocated separately from pInfo.
   bool ownsDevMode;
   QPrinter::PrinterMode mode;

   // Printer info
   QPrintDevice m_printDevice;

   // Document info
   QString docName;
   QString m_creator;
   QString fileName;

   QPrinter::PrinterState state;
   int resolution;

   // Page Layout
   QPageLayout m_pageLayout;

   // Page metrics cache
   QRect m_paintRectPixels;
   QSize m_paintSizeMM;
   qreal stretch_x;
   qreal stretch_y;
   int origin_x;
   int origin_y;

   int dpi_x;
   int dpi_y;
   int dpi_display;
   int num_copies;

   uint printToFile : 1;

   uint reinit : 1;

   uint complex_xform : 1;
   uint has_pen : 1;
   uint has_brush : 1;
   uint has_custom_paper_size : 1;
   uint embed_fonts : 1;

   uint txop;

   QColor brush_color;
   QPen pen;
   QColor pen_color;
   QSizeF paper_size;

   QTransform painterMatrix;
   QTransform matrix;
};

#endif // QT_NO_PRINTER

#endif // QPRINTENGINE_WIN_P_H
