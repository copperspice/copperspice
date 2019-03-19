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

#include <qcolordialog_p.h>

#ifndef QT_NO_COLORDIALOG

#include <qapplication.h>
#include <qdesktopwidget.h>
#include <qdrawutil.h>
#include <qevent.h>
#include <qimage.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qmenu.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qpushbutton.h>
#include <qsettings.h>
#include <qstyle.h>
#include <qstyleoption.h>
#include <qvalidator.h>
#include <qmime.h>
#include <qspinbox.h>
#include <qdialogbuttonbox.h>
#include <qguiplatformplugin_p.h>

QT_BEGIN_NAMESPACE

struct QWellArrayData;

class QWellArray : public QWidget
{
   GUI_CS_OBJECT(QWellArray)

   GUI_CS_PROPERTY_READ(selectedColumn, selectedColumn)
   GUI_CS_PROPERTY_READ(selectedRow, selectedRow)

 public:
   QWellArray(int rows, int cols, QWidget *parent = nullptr);
   ~QWellArray() {}

   QString cellContent(int row, int col) const;

   inline int selectedColumn() const;
   inline int selectedRow() const;

   virtual void setCurrent(int row, int col);
   virtual void setSelected(int row, int col);

   QSize sizeHint() const override;

   virtual void setCellBrush(int row, int col, const QBrush &);
   QBrush cellBrush(int row, int col);

   inline int cellWidth() const {
      return cellw;
   }

   inline int cellHeight() const {
      return cellh;
   }

   inline int rowAt(int y) const {
      return y / cellh;
   }

   inline int columnAt(int x) const {
      if (isRightToLeft()) {
         return ncols - (x / cellw) - 1;
      }
      return x / cellw;
   }

   inline int rowY(int row) const {
      return cellh * row;
   }

   inline int columnX(int column) const {
      if (isRightToLeft()) {
         return cellw * (ncols - column - 1);
      }
      return cellw * column;
   }

   inline int numRows() const {
      return nrows;
   }

   inline int numCols() const {
      return ncols;
   }

   inline QRect cellRect() const {
      return QRect(0, 0, cellw, cellh);
   }

   inline QSize gridSize() const {
      return QSize(ncols * cellw, nrows * cellh);
   }

   inline QRect cellGeometry(int row, int column) {
      QRect r;
      if (row >= 0 && row < nrows && column >= 0 && column < ncols) {
         r.setRect(columnX(column), rowY(row), cellw, cellh);
      }
      return r;
   }

   inline void updateCell(int row, int column) {
      update(cellGeometry(row, column));
   }

   GUI_CS_SIGNAL_1(Public, void selected(int row, int col))
   GUI_CS_SIGNAL_2(selected, row, col)

 protected:
   virtual void paintCell(QPainter *, int row, int col, const QRect &);
   virtual void paintCellContents(QPainter *, int row, int col, const QRect &);

   void mousePressEvent(QMouseEvent *) override;
   void mouseReleaseEvent(QMouseEvent *) override;
   void keyPressEvent(QKeyEvent *) override;
   void focusInEvent(QFocusEvent *) override;
   void focusOutEvent(QFocusEvent *) override;
   void paintEvent(QPaintEvent *) override;

 private:
   Q_DISABLE_COPY(QWellArray)

   int nrows;
   int ncols;
   int cellw;
   int cellh;
   int curRow;
   int curCol;
   int selRow;
   int selCol;
   QWellArrayData *d;
};

int QWellArray::selectedColumn() const
{
   return selCol;
}

int QWellArray::selectedRow() const
{
   return selRow;
}

void QWellArray::paintEvent(QPaintEvent *e)
{
   QRect r = e->rect();
   int cx = r.x();
   int cy = r.y();
   int ch = r.height();
   int cw = r.width();
   int colfirst = columnAt(cx);
   int collast = columnAt(cx + cw);
   int rowfirst = rowAt(cy);
   int rowlast = rowAt(cy + ch);

   if (isRightToLeft()) {
      int t = colfirst;
      colfirst = collast;
      collast = t;
   }

   QPainter painter(this);
   QPainter *p = &painter;
   QRect rect(0, 0, cellWidth(), cellHeight());


   if (collast < 0 || collast >= ncols) {
      collast = ncols - 1;
   }
   if (rowlast < 0 || rowlast >= nrows) {
      rowlast = nrows - 1;
   }

   // Go through the rows
   for (int r = rowfirst; r <= rowlast; ++r) {
      // get row position and height
      int rowp = rowY(r);

      // Go through the columns in the row r
      // if we know from where to where, go through [colfirst, collast],
      // else go through all of them
      for (int c = colfirst; c <= collast; ++c) {
         // get position and width of column c
         int colp = columnX(c);
         // Translate painter and draw the cell
         rect.translate(colp, rowp);
         paintCell(p, r, c, rect);
         rect.translate(-colp, -rowp);
      }
   }
}

struct QWellArrayData {
   QBrush *brush;
};

QWellArray::QWellArray(int rows, int cols, QWidget *parent)
   : QWidget(parent), nrows(rows), ncols(cols)
{
   d = 0;
   setFocusPolicy(Qt::StrongFocus);
   cellw = 28;
   cellh = 24;
   curCol = 0;
   curRow = 0;
   selCol = -1;
   selRow = -1;
}

QSize QWellArray::sizeHint() const
{
   ensurePolished();
   return gridSize().boundedTo(QSize(640, 480));
}

void QWellArray::paintCell(QPainter *p, int row, int col, const QRect &rect)
{
   int b = 3; //margin

   const QPalette &g = palette();
   QStyleOptionFrame opt;
   int dfw = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
   opt.lineWidth = dfw;
   opt.midLineWidth = 1;
   opt.rect = rect.adjusted(b, b, -b, -b);
   opt.palette = g;
   opt.state = QStyle::State_Enabled | QStyle::State_Sunken;
   style()->drawPrimitive(QStyle::PE_Frame, &opt, p, this);
   b += dfw;

   if ((row == curRow) && (col == curCol)) {
      if (hasFocus()) {
         QStyleOptionFocusRect opt;
         opt.palette = g;
         opt.rect = rect;
         opt.state = QStyle::State_None | QStyle::State_KeyboardFocusChange;
         style()->drawPrimitive(QStyle::PE_FrameFocusRect, &opt, p, this);
      }
   }
   paintCellContents(p, row, col, opt.rect.adjusted(dfw, dfw, -dfw, -dfw));
}


//  Reimplement this function to change the contents of the well array.
void QWellArray::paintCellContents(QPainter *p, int row, int col, const QRect &r)
{
   if (d) {
      p->fillRect(r, d->brush[row * numCols() + col]);
   } else {
      p->fillRect(r, Qt::white);
      p->setPen(Qt::black);
      p->drawLine(r.topLeft(), r.bottomRight());
      p->drawLine(r.topRight(), r.bottomLeft());
   }
}

void QWellArray::mousePressEvent(QMouseEvent *e)
{
   // The current cell marker is set to the cell the mouse is pressed in
   QPoint pos = e->pos();
   setCurrent(rowAt(pos.y()), columnAt(pos.x()));
}

void QWellArray::mouseReleaseEvent(QMouseEvent * /* event */)
{
   // The current cell marker is set to the cell the mouse is clicked in
   setSelected(curRow, curCol);
}


/*
  Sets the cell currently having the focus. This is not necessarily
  the same as the currently selected cell.
*/

void QWellArray::setCurrent(int row, int col)
{
   if ((curRow == row) && (curCol == col)) {
      return;
   }

   if (row < 0 || col < 0) {
      row = col = -1;
   }

   int oldRow = curRow;
   int oldCol = curCol;

   curRow = row;
   curCol = col;

   updateCell(oldRow, oldCol);
   updateCell(curRow, curCol);
}

/*
  Sets the currently selected cell to \a row, \a column. If \a row or
  \a column are less than zero, the current cell is unselected.

  Does not set the position of the focus indicator.
*/
void QWellArray::setSelected(int row, int col)
{
   int oldRow = selRow;
   int oldCol = selCol;

   if (row < 0 || col < 0) {
      row = col = -1;
   }

   selCol = col;
   selRow = row;

   updateCell(oldRow, oldCol);
   updateCell(selRow, selCol);
   if (row >= 0) {
      emit selected(row, col);
   }

#ifndef QT_NO_MENU
   if (isVisible() && qobject_cast<QMenu *>(parentWidget())) {
      parentWidget()->close();
   }
#endif
}

void QWellArray::focusInEvent(QFocusEvent *)
{
   updateCell(curRow, curCol);
}

void QWellArray::setCellBrush(int row, int col, const QBrush &b)
{
   if (!d) {
      d = new QWellArrayData;
      int i = numRows() * numCols();
      d->brush = new QBrush[i];
   }
   if (row >= 0 && row < numRows() && col >= 0 && col < numCols()) {
      d->brush[row * numCols() + col] = b;
   }
}

/*
  Returns the brush set for the cell at \a row, \a column. If no brush is
  set, Qt::NoBrush is returned.
*/

QBrush QWellArray::cellBrush(int row, int col)
{
   if (d && row >= 0 && row < numRows() && col >= 0 && col < numCols()) {
      return d->brush[row * numCols() + col];
   }
   return Qt::NoBrush;
}



/*!\reimp
*/

void QWellArray::focusOutEvent(QFocusEvent *)
{
   updateCell(curRow, curCol);
}

/*\reimp
*/
void QWellArray::keyPressEvent(QKeyEvent *e)
{
   switch (e->key()) {                       // Look at the key code
      case Qt::Key_Left:                                // If 'left arrow'-key,
         if (curCol > 0) {                     // and cr't not in leftmost col
            setCurrent(curRow, curCol - 1);   // set cr't to next left column
         }
         break;
      case Qt::Key_Right:                                // Correspondingly...
         if (curCol < numCols() - 1) {
            setCurrent(curRow, curCol + 1);
         }
         break;
      case Qt::Key_Up:
         if (curRow > 0) {
            setCurrent(curRow - 1, curCol);
         }
         break;
      case Qt::Key_Down:
         if (curRow < numRows() - 1) {
            setCurrent(curRow + 1, curCol);
         }
         break;
      case Qt::Key_Space:
         setSelected(curRow, curCol);
         break;
      default:                                // If not an interesting key,
         e->ignore();                        // we don't accept the event
         return;
   }

}

static bool initrgb = false;
static QRgb s_standardRGB[6 * 8];
static QRgb s_customRGB[2 * 8];
static bool customSet = false;


static void initRGB()
{
   if (initrgb) {
      return;
   }

   initrgb = true;
   int i = 0;

   for (int g = 0; g < 4; g++)
      for (int r = 0;  r < 4; r++)
         for (int b = 0; b < 3; b++) {
            s_standardRGB[i++] = qRgb(r * 255 / 3, g * 255 / 3, b * 255 / 2);
         }

   for (i = 0; i < 2 * 8; i++) {
      s_customRGB[i] = 0xffffffff;
   }
}

/*!
    Returns the number of custom colors supported by QColorDialog. All
    color dialogs share the same custom colors.
*/
int QColorDialog::customCount()
{
   return 2 * 8;
}

QColor QColorDialog::customColor(int index)
{
   if (uint(index) >= uint(customCount())) {
      return qRgb(255, 255, 255);
   }

   initRGB();
   return s_customRGB[index];
}

void QColorDialog::setCustomColor(int index, QColor color)
{
   if (uint(index) >= uint(customCount())) {
      return;
   }

   initRGB();
   customSet = true;
   s_customRGB[index] = color.rgb();
}

void QColorDialog::setStandardColor(int index, QColor color)
{
   if (uint(index) >= uint(6 * 8)) {
      return;
   }

   initRGB();
   s_standardRGB[index] = color.rgb();
}

static inline void rgb2hsv(QRgb rgb, int &h, int &s, int &v)
{
   QColor c;
   c.setRgb(rgb);
   c.getHsv(&h, &s, &v);
}

class QColorWell : public QWellArray
{
 public:
   QColorWell(QWidget *parent, int r, int c, QRgb *vals)
      : QWellArray(r, c, parent), values(vals), mousePressed(false), oldCurrent(-1, -1) {
      setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum));
   }

 protected:
   void paintCellContents(QPainter *, int row, int col, const QRect &) override;
   void mousePressEvent(QMouseEvent *e) override;
   void mouseMoveEvent(QMouseEvent *e)override;
   void mouseReleaseEvent(QMouseEvent *e) override;

#ifndef QT_NO_DRAGANDDROP
   void dragEnterEvent(QDragEnterEvent *e) override;
   void dragLeaveEvent(QDragLeaveEvent *e) override;
   void dragMoveEvent(QDragMoveEvent *e) override;
   void dropEvent(QDropEvent *e)override;
#endif

 private:
   QRgb *values;
   bool mousePressed;
   QPoint pressPos;
   QPoint oldCurrent;
};

void QColorWell::paintCellContents(QPainter *p, int row, int col, const QRect &r)
{
   int i = row + col * numRows();
   p->fillRect(r, QColor(values[i]));
}

void QColorWell::mousePressEvent(QMouseEvent *e)
{
   oldCurrent = QPoint(selectedRow(), selectedColumn());
   QWellArray::mousePressEvent(e);
   mousePressed = true;
   pressPos = e->pos();
}

void QColorWell::mouseMoveEvent(QMouseEvent *e)
{
   QWellArray::mouseMoveEvent(e);
#ifndef QT_NO_DRAGANDDROP
   if (!mousePressed) {
      return;
   }
   if ((pressPos - e->pos()).manhattanLength() > QApplication::startDragDistance()) {
      setCurrent(oldCurrent.x(), oldCurrent.y());
      int i = rowAt(pressPos.y()) + columnAt(pressPos.x()) * numRows();
      QColor col(values[i]);
      QMimeData *mime = new QMimeData;
      mime->setColorData(col);
      QPixmap pix(cellWidth(), cellHeight());
      pix.fill(col);
      QPainter p(&pix);
      p.drawRect(0, 0, pix.width() - 1, pix.height() - 1);
      p.end();
      QDrag *drg = new QDrag(this);
      drg->setMimeData(mime);
      drg->setPixmap(pix);
      mousePressed = false;
      drg->start();
   }
#endif
}

#ifndef QT_NO_DRAGANDDROP
void QColorWell::dragEnterEvent(QDragEnterEvent *e)
{
   if (qvariant_cast<QColor>(e->mimeData()->colorData()).isValid()) {
      e->accept();
   } else {
      e->ignore();
   }
}

void QColorWell::dragLeaveEvent(QDragLeaveEvent *)
{
   if (hasFocus()) {
      parentWidget()->setFocus();
   }
}

void QColorWell::dragMoveEvent(QDragMoveEvent *e)
{
   if (qvariant_cast<QColor>(e->mimeData()->colorData()).isValid()) {
      setCurrent(rowAt(e->pos().y()), columnAt(e->pos().x()));
      e->accept();
   } else {
      e->ignore();
   }
}

void QColorWell::dropEvent(QDropEvent *e)
{
   QColor col = qvariant_cast<QColor>(e->mimeData()->colorData());
   if (col.isValid()) {
      int i = rowAt(e->pos().y()) + columnAt(e->pos().x()) * numRows();
      values[i] = col.rgb();
      update();
      e->accept();
   } else {
      e->ignore();
   }
}

#endif // QT_NO_DRAGANDDROP

void QColorWell::mouseReleaseEvent(QMouseEvent *e)
{
   if (!mousePressed) {
      return;
   }
   QWellArray::mouseReleaseEvent(e);
   mousePressed = false;
}

class QColorPicker : public QFrame
{
   GUI_CS_OBJECT(QColorPicker)

 public:
   QColorPicker(QWidget *parent);
   ~QColorPicker();

   GUI_CS_SIGNAL_1(Public, void newCol(int h, int s))
   GUI_CS_SIGNAL_2(newCol, h, s)

   GUI_CS_SLOT_1(Public, void setCol(int h, int s))
   GUI_CS_SLOT_OVERLOAD(setCol, (int, int))

 protected:
   QSize sizeHint() const override;
   void paintEvent(QPaintEvent *) override;
   void mouseMoveEvent(QMouseEvent *) override;
   void mousePressEvent(QMouseEvent *) override;
   void resizeEvent(QResizeEvent *) override;

 private:
   int hue;
   int sat;

   QPoint colPt();
   int huePt(const QPoint &pt);
   int satPt(const QPoint &pt);
   void setCol(const QPoint &pt);

   QPixmap pix;
};

static int pWidth = 220;
static int pHeight = 200;

class QColorLuminancePicker : public QWidget
{
   GUI_CS_OBJECT(QColorLuminancePicker)

 public:
   QColorLuminancePicker(QWidget *parent = nullptr);
   ~QColorLuminancePicker();

   GUI_CS_SIGNAL_1(Public, void newHsv(int h, int s, int v))
   GUI_CS_SIGNAL_2(newHsv, h, s, v)

   GUI_CS_SLOT_1(Public, void setCol(int h, int s, int v))
   GUI_CS_SLOT_OVERLOAD(setCol, (int, int, int))

   GUI_CS_SLOT_1(Public, void setCol(int h, int s))
   GUI_CS_SLOT_OVERLOAD(setCol, (int, int))

 protected:
   void paintEvent(QPaintEvent *) override;
   void mouseMoveEvent(QMouseEvent *) override;
   void mousePressEvent(QMouseEvent *) override;

 private:
   enum { foff = 3, coff = 4 }; //frame and contents offset
   int val;
   int hue;
   int sat;

   int y2val(int y);
   int val2y(int val);
   void setVal(int v);

   QPixmap *pix;
};


int QColorLuminancePicker::y2val(int y)
{
   int d = height() - 2 * coff - 1;
   return 255 - (y - coff) * 255 / d;
}

int QColorLuminancePicker::val2y(int v)
{
   int d = height() - 2 * coff - 1;
   return coff + (255 - v) * d / 255;
}

QColorLuminancePicker::QColorLuminancePicker(QWidget *parent)
   : QWidget(parent)
{
   hue = 100;
   val = 100;
   sat = 100;
   pix = 0;
   //    setAttribute(WA_NoErase, true);
}

QColorLuminancePicker::~QColorLuminancePicker()
{
   delete pix;
}

void QColorLuminancePicker::mouseMoveEvent(QMouseEvent *m)
{
   setVal(y2val(m->y()));
}
void QColorLuminancePicker::mousePressEvent(QMouseEvent *m)
{
   setVal(y2val(m->y()));
}

void QColorLuminancePicker::setVal(int v)
{
   if (val == v) {
      return;
   }
   val = qMax(0, qMin(v, 255));
   delete pix;
   pix = 0;
   repaint();
   emit newHsv(hue, sat, val);
}

//receives from a hue,sat chooser and relays.
void QColorLuminancePicker::setCol(int h, int s)
{
   setCol(h, s, val);
   emit newHsv(h, s, val);
}

void QColorLuminancePicker::paintEvent(QPaintEvent *)
{
   int w = width() - 5;

   QRect r(0, foff, w, height() - 2 * foff);
   int wi = r.width() - 2;
   int hi = r.height() - 2;
   if (!pix || pix->height() != hi || pix->width() != wi) {
      delete pix;
      QImage img(wi, hi, QImage::Format_RGB32);
      int y;
      uint *pixel = (uint *) img.scanLine(0);
      for (y = 0; y < hi; y++) {
         const uint *end = pixel + wi;
         while (pixel < end) {
            QColor c;
            c.setHsv(hue, sat, y2val(y + coff));
            *pixel = c.rgb();
            ++pixel;
         }
      }
      pix = new QPixmap(QPixmap::fromImage(img));
   }
   QPainter p(this);
   p.drawPixmap(1, coff, *pix);
   const QPalette &g = palette();
   qDrawShadePanel(&p, r, g, true);
   p.setPen(g.foreground().color());
   p.setBrush(g.foreground());
   QPolygon a;
   int y = val2y(val);
   a.setPoints(3, w, y, w + 5, y + 5, w + 5, y - 5);
   p.eraseRect(w, 0, 5, height());
   p.drawPolygon(a);
}

void QColorLuminancePicker::setCol(int h, int s , int v)
{
   val = v;
   hue = h;
   sat = s;
   delete pix;
   pix = 0;
   repaint();
}

QPoint QColorPicker::colPt()
{
   QRect r = contentsRect();
   return QPoint((360 - hue) * (r.width() - 1) / 360, (255 - sat) * (r.height() - 1) / 255);
}

int QColorPicker::huePt(const QPoint &pt)
{
   QRect r = contentsRect();
   return 360 - pt.x() * 360 / (r.width() - 1);
}

int QColorPicker::satPt(const QPoint &pt)
{
   QRect r = contentsRect();
   return 255 - pt.y() * 255 / (r.height() - 1);
}

void QColorPicker::setCol(const QPoint &pt)
{
   setCol(huePt(pt), satPt(pt));
}

QColorPicker::QColorPicker(QWidget *parent)
   : QFrame(parent)
{
   hue = 0;
   sat = 0;
   setCol(150, 255);

   setAttribute(Qt::WA_NoSystemBackground);
   setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed) );
}

QColorPicker::~QColorPicker()
{
}

QSize QColorPicker::sizeHint() const
{
   return QSize(pWidth + 2 * frameWidth(), pHeight + 2 * frameWidth());
}

void QColorPicker::setCol(int h, int s)
{
   int nhue = qMin(qMax(0, h), 359);
   int nsat = qMin(qMax(0, s), 255);
   if (nhue == hue && nsat == sat) {
      return;
   }

   QRect r(colPt(), QSize(20, 20));
   hue = nhue;
   sat = nsat;
   r = r.united(QRect(colPt(), QSize(20, 20)));
   r.translate(contentsRect().x() - 9, contentsRect().y() - 9);
   //    update(r);
   repaint(r);
}

void QColorPicker::mouseMoveEvent(QMouseEvent *m)
{
   QPoint p = m->pos() - contentsRect().topLeft();
   setCol(p);
   emit newCol(hue, sat);
}

void QColorPicker::mousePressEvent(QMouseEvent *m)
{
   QPoint p = m->pos() - contentsRect().topLeft();
   setCol(p);
   emit newCol(hue, sat);
}

void QColorPicker::paintEvent(QPaintEvent *)
{
   QPainter p(this);
   drawFrame(&p);
   QRect r = contentsRect();

   p.drawPixmap(r.topLeft(), pix);
   QPoint pt = colPt() + r.topLeft();
   p.setPen(Qt::black);

   p.fillRect(pt.x() - 9, pt.y(), 20, 2, Qt::black);
   p.fillRect(pt.x(), pt.y() - 9, 2, 20, Qt::black);

}

void QColorPicker::resizeEvent(QResizeEvent *ev)
{
   QFrame::resizeEvent(ev);

   int w = width() - frameWidth() * 2;
   int h = height() - frameWidth() * 2;
   QImage img(w, h, QImage::Format_RGB32);
   int x, y;
   uint *pixel = (uint *) img.scanLine(0);
   for (y = 0; y < h; y++) {
      const uint *end = pixel + w;
      x = 0;
      while (pixel < end) {
         QPoint p(x, y);
         QColor c;
         c.setHsv(huePt(p), satPt(p), 200);
         *pixel = c.rgb();
         ++pixel;
         ++x;
      }
   }
   pix = QPixmap::fromImage(img);
}


class QColSpinBox : public QSpinBox
{

 public:
   QColSpinBox(QWidget *parent)
      : QSpinBox(parent) {
      setRange(0, 255);
   }
   void setValue(int i) {
      bool block = signalsBlocked();
      blockSignals(true);
      QSpinBox::setValue(i);
      blockSignals(block);
   }
};

class QColorShowLabel;

class QColorShower : public QWidget
{
   GUI_CS_OBJECT(QColorShower)

 public:
   QColorShower(QColorDialog *parent);

   // things that don't emit signals
   void setHsv(int h, int s, int v);

   int currentAlpha() const {
      return (colorDialog->options() & QColorDialog::ShowAlphaChannel) ? alphaEd->value() : 255;
   }

   void setCurrentAlpha(int a) {
      alphaEd->setValue(a);
      rgbEd();
   }

   void showAlpha(bool b);
   bool isAlphaVisible() const;

   QRgb currentColor() const {
      return curCol;
   }

   QColor currentQColor() const {
      return curQColor;
   }
   void retranslateStrings();
   void updateQColor();

   GUI_CS_SIGNAL_1(Public, void newCol(const QRgb &rgb))
   GUI_CS_SIGNAL_2(newCol, rgb)

   GUI_CS_SIGNAL_1(Public, void currentColorChanged(const QColor &color))
   GUI_CS_SIGNAL_2(currentColorChanged, color)

   GUI_CS_SLOT_1(Public, void setRgb(const QRgb &rgb))
   GUI_CS_SLOT_2(setRgb)

 private:
   GUI_CS_SLOT_1(Private, void rgbEd())
   GUI_CS_SLOT_2(rgbEd)

   GUI_CS_SLOT_1(Private, void hsvEd())
   GUI_CS_SLOT_2(hsvEd)

   void showCurrentColor();
   int hue, sat, val;
   QRgb curCol;
   QColor curQColor;
   QLabel *lblHue;
   QLabel *lblSat;
   QLabel *lblVal;
   QLabel *lblRed;
   QLabel *lblGreen;
   QLabel *lblBlue;
   QColSpinBox *hEd;
   QColSpinBox *sEd;
   QColSpinBox *vEd;
   QColSpinBox *rEd;
   QColSpinBox *gEd;
   QColSpinBox *bEd;
   QColSpinBox *alphaEd;
   QLabel *alphaLab;
   QColorShowLabel *lab;
   bool rgbOriginal;

   QColorDialog *colorDialog;

   friend class QColorDialog;
   friend class QColorDialogPrivate;
};

class QColorShowLabel : public QFrame
{
   GUI_CS_OBJECT(QColorShowLabel)

 public:
   QColorShowLabel(QWidget *parent) : QFrame(parent) {
      setFrameStyle(QFrame::Panel | QFrame::Sunken);
      setAcceptDrops(true);
      mousePressed = false;
   }
   void setColor(QColor c) {
      col = c;
   }

   GUI_CS_SIGNAL_1(Public, void colorDropped(const QRgb &un_named_arg1))
   GUI_CS_SIGNAL_2(colorDropped, un_named_arg1)

 protected:
   void paintEvent(QPaintEvent *) override;
   void mousePressEvent(QMouseEvent *e) override;
   void mouseMoveEvent(QMouseEvent *e) override;
   void mouseReleaseEvent(QMouseEvent *e) override;

#ifndef QT_NO_DRAGANDDROP
   void dragEnterEvent(QDragEnterEvent *e) override;
   void dragLeaveEvent(QDragLeaveEvent *e) override;
   void dropEvent(QDropEvent *e) override;
#endif

 private:
   QColor col;
   bool mousePressed;
   QPoint pressPos;
};

void QColorShowLabel::paintEvent(QPaintEvent *e)
{
   QPainter p(this);
   drawFrame(&p);
   p.fillRect(contentsRect()&e->rect(), col);
}

void QColorShower::showAlpha(bool b)
{
   alphaLab->setVisible(b);
   alphaEd->setVisible(b);
}

inline bool QColorShower::isAlphaVisible() const
{
   return alphaLab->isVisible();
}

void QColorShowLabel::mousePressEvent(QMouseEvent *e)
{
   mousePressed = true;
   pressPos = e->pos();
}

void QColorShowLabel::mouseMoveEvent(QMouseEvent *e)
{
#ifdef QT_NO_DRAGANDDROP
   Q_UNUSED(e);
#else
   if (!mousePressed) {
      return;
   }
   if ((pressPos - e->pos()).manhattanLength() > QApplication::startDragDistance()) {
      QMimeData *mime = new QMimeData;
      mime->setColorData(col);
      QPixmap pix(30, 20);
      pix.fill(col);
      QPainter p(&pix);
      p.drawRect(0, 0, pix.width() - 1, pix.height() - 1);
      p.end();
      QDrag *drg = new QDrag(this);
      drg->setMimeData(mime);
      drg->setPixmap(pix);
      mousePressed = false;
      drg->start();
   }
#endif
}

#ifndef QT_NO_DRAGANDDROP
void QColorShowLabel::dragEnterEvent(QDragEnterEvent *e)
{
   if (qvariant_cast<QColor>(e->mimeData()->colorData()).isValid()) {
      e->accept();
   } else {
      e->ignore();
   }
}

void QColorShowLabel::dragLeaveEvent(QDragLeaveEvent *)
{
}

void QColorShowLabel::dropEvent(QDropEvent *e)
{
   QColor color = qvariant_cast<QColor>(e->mimeData()->colorData());
   if (color.isValid()) {
      col = color;
      repaint();
      emit colorDropped(col.rgb());
      e->accept();
   } else {
      e->ignore();
   }
}
#endif // QT_NO_DRAGANDDROP

void QColorShowLabel::mouseReleaseEvent(QMouseEvent *)
{
   if (!mousePressed) {
      return;
   }
   mousePressed = false;
}

QColorShower::QColorShower(QColorDialog *parent)
   : QWidget(parent)
{
   colorDialog = parent;

   curCol = qRgb(255, 255, 255);
   curQColor = Qt::white;

   QGridLayout *gl = new QGridLayout(this);
   gl->setMargin(gl->spacing());
   lab = new QColorShowLabel(this);

#ifdef QT_SMALL_COLORDIALOG
   lab->setMinimumHeight(60);
#endif
   lab->setMinimumWidth(60);


   // In S60, due to small screen and different screen layouts need to re-arrange the widgets.
   // For QVGA screens only the comboboxes and color label are visible.
   // For nHD screens only color and luminence pickers and color label are visible.
#if !defined(QT_SMALL_COLORDIALOG)
   gl->addWidget(lab, 0, 0, -1, 1);
#else
   if (nonTouchUI) {
      gl->addWidget(lab, 0, 0, 1, -1);
   } else {
      gl->addWidget(lab, 0, 0, -1, 1);
   }
#endif

   connect(lab, SIGNAL(colorDropped(const QRgb &)), this, SLOT(newCol(const QRgb &)));
   connect(lab, SIGNAL(colorDropped(const QRgb &)), this, SLOT(setRgb(const QRgb &)));

   hEd = new QColSpinBox(this);
   hEd->setRange(0, 359);
   lblHue = new QLabel(this);

#ifndef QT_NO_SHORTCUT
   lblHue->setBuddy(hEd);
#endif

   lblHue->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
#if !defined(QT_SMALL_COLORDIALOG)
   gl->addWidget(lblHue, 0, 1);
   gl->addWidget(hEd, 0, 2);
#else
   if (nonTouchUI) {
      gl->addWidget(lblHue, 1, 0);
      gl->addWidget(hEd, 2, 0);
   } else {
      lblHue->hide();
      hEd->hide();
   }
#endif

   sEd = new QColSpinBox(this);
   lblSat = new QLabel(this);
#ifndef QT_NO_SHORTCUT
   lblSat->setBuddy(sEd);
#endif
   lblSat->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
#if !defined(QT_SMALL_COLORDIALOG)
   gl->addWidget(lblSat, 1, 1);
   gl->addWidget(sEd, 1, 2);
#else
   if (nonTouchUI) {
      gl->addWidget(lblSat, 1, 1);
      gl->addWidget(sEd, 2, 1);
   } else {
      lblSat->hide();
      sEd->hide();
   }
#endif

   vEd = new QColSpinBox(this);
   lblVal = new QLabel(this);
#ifndef QT_NO_SHORTCUT
   lblVal->setBuddy(vEd);
#endif
   lblVal->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
#if !defined(QT_SMALL_COLORDIALOG)
   gl->addWidget(lblVal, 2, 1);
   gl->addWidget(vEd, 2, 2);
#else
   if (nonTouchUI) {
      gl->addWidget(lblVal, 1, 2);
      gl->addWidget(vEd, 2, 2);
   } else {
      lblVal->hide();
      vEd->hide();
   }
#endif

   rEd = new QColSpinBox(this);
   lblRed = new QLabel(this);
#ifndef QT_NO_SHORTCUT
   lblRed->setBuddy(rEd);
#endif
   lblRed->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
#if !defined(QT_SMALL_COLORDIALOG)
   gl->addWidget(lblRed, 0, 3);
   gl->addWidget(rEd, 0, 4);
#else
   if (nonTouchUI) {
      gl->addWidget(lblRed, 3, 0);
      gl->addWidget(rEd, 4, 0);
   } else {
      lblRed->hide();
      rEd->hide();
   }
#endif

   gEd = new QColSpinBox(this);
   lblGreen = new QLabel(this);
#ifndef QT_NO_SHORTCUT
   lblGreen->setBuddy(gEd);
#endif
   lblGreen->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
#if !defined(QT_SMALL_COLORDIALOG)
   gl->addWidget(lblGreen, 1, 3);
   gl->addWidget(gEd, 1, 4);
#else
   if (nonTouchUI) {
      gl->addWidget(lblGreen, 3, 1);
      gl->addWidget(gEd, 4, 1);
   } else {
      lblGreen->hide();
      gEd->hide();
   }
#endif

   bEd = new QColSpinBox(this);
   lblBlue = new QLabel(this);
#ifndef QT_NO_SHORTCUT
   lblBlue->setBuddy(bEd);
#endif
   lblBlue->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
#if !defined(QT_SMALL_COLORDIALOG)
   gl->addWidget(lblBlue, 2, 3);
   gl->addWidget(bEd, 2, 4);
#else
   if (nonTouchUI) {
      gl->addWidget(lblBlue, 3, 2);
      gl->addWidget(bEd, 4, 2);
   } else {
      lblBlue->hide();
      bEd->hide();
   }
#endif

   alphaEd = new QColSpinBox(this);
   alphaLab = new QLabel(this);
#ifndef QT_NO_SHORTCUT
   alphaLab->setBuddy(alphaEd);
#endif
   alphaLab->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
#if !defined(QT_SMALL_COLORDIALOG)
   gl->addWidget(alphaLab, 3, 1, 1, 3);
   gl->addWidget(alphaEd, 3, 4);
#else
   if (nonTouchUI) {
      gl->addWidget(alphaLab, 1, 3, 3, 1);
      gl->addWidget(alphaEd, 4, 3);
   } else {
      alphaLab->hide();
      alphaEd->hide();
   }
#endif
   alphaEd->hide();
   alphaLab->hide();

   connect(hEd, SIGNAL(valueChanged(int)), this, SLOT(hsvEd()));
   connect(sEd, SIGNAL(valueChanged(int)), this, SLOT(hsvEd()));
   connect(vEd, SIGNAL(valueChanged(int)), this, SLOT(hsvEd()));

   connect(rEd, SIGNAL(valueChanged(int)), this, SLOT(rgbEd()));
   connect(gEd, SIGNAL(valueChanged(int)), this, SLOT(rgbEd()));
   connect(bEd, SIGNAL(valueChanged(int)), this, SLOT(rgbEd()));
   connect(alphaEd, SIGNAL(valueChanged(int)), this, SLOT(rgbEd()));

   retranslateStrings();
}

inline QRgb QColorDialogPrivate::currentColor() const
{
   return cs->currentColor();
}
inline int QColorDialogPrivate::currentAlpha() const
{
   return cs->currentAlpha();
}
inline void QColorDialogPrivate::setCurrentAlpha(int a)
{
   cs->setCurrentAlpha(a);
}
inline void QColorDialogPrivate::showAlpha(bool b)
{
   cs->showAlpha(b);
}
inline bool QColorDialogPrivate::isAlphaVisible() const
{
   return cs->isAlphaVisible();
}

QColor QColorDialogPrivate::currentQColor() const
{
   return cs->currentQColor();
}

void QColorShower::showCurrentColor()
{
   lab->setColor(currentColor());
   lab->repaint();
}

void QColorShower::rgbEd()
{
   rgbOriginal = true;
   curCol = qRgba(rEd->value(), gEd->value(), bEd->value(), currentAlpha());

   rgb2hsv(currentColor(), hue, sat, val);

   hEd->setValue(hue);
   sEd->setValue(sat);
   vEd->setValue(val);

   showCurrentColor();
   emit newCol(currentColor());
   updateQColor();
}

void QColorShower::hsvEd()
{
   rgbOriginal = false;
   hue = hEd->value();
   sat = sEd->value();
   val = vEd->value();

   QColor c;
   c.setHsv(hue, sat, val);
   curCol = c.rgb();

   rEd->setValue(qRed(currentColor()));
   gEd->setValue(qGreen(currentColor()));
   bEd->setValue(qBlue(currentColor()));

   showCurrentColor();
   emit newCol(currentColor());
   updateQColor();
}

void QColorShower::setRgb(const QRgb &rgb)
{
   rgbOriginal = true;
   curCol = rgb;

   rgb2hsv(currentColor(), hue, sat, val);

   hEd->setValue(hue);
   sEd->setValue(sat);
   vEd->setValue(val);

   rEd->setValue(qRed(currentColor()));
   gEd->setValue(qGreen(currentColor()));
   bEd->setValue(qBlue(currentColor()));

   showCurrentColor();
   updateQColor();
}

void QColorShower::setHsv(int h, int s, int v)
{
   if (h < -1 || (uint)s > 255 || (uint)v > 255) {
      return;
   }

   rgbOriginal = false;
   hue = h;
   val = v;
   sat = s;
   QColor c;
   c.setHsv(hue, sat, val);
   curCol = c.rgb();

   hEd->setValue(hue);
   sEd->setValue(sat);
   vEd->setValue(val);

   rEd->setValue(qRed(currentColor()));
   gEd->setValue(qGreen(currentColor()));
   bEd->setValue(qBlue(currentColor()));

   showCurrentColor();
   updateQColor();
}

void QColorShower::retranslateStrings()
{
   lblHue->setText(QColorDialog::tr("Hu&e:"));
   lblSat->setText(QColorDialog::tr("&Sat:"));
   lblVal->setText(QColorDialog::tr("&Val:"));
   lblRed->setText(QColorDialog::tr("&Red:"));
   lblGreen->setText(QColorDialog::tr("&Green:"));
   lblBlue->setText(QColorDialog::tr("Bl&ue:"));
   alphaLab->setText(QColorDialog::tr("A&lpha channel:"));
}

void QColorShower::updateQColor()
{
   QColor oldQColor(curQColor);
   curQColor.setRgba(qRgba(qRed(curCol), qGreen(curCol), qBlue(curCol), currentAlpha()));

   if (curQColor != oldQColor) {
      emit currentColorChanged(curQColor);
   }
}

//sets all widgets to display h,s,v
void QColorDialogPrivate::_q_newHsv(int h, int s, int v)
{
   cs->setHsv(h, s, v);
   cp->setCol(h, s);
   lp->setCol(h, s, v);
}

// sets all widgets to display rgb
void QColorDialogPrivate::setCurrentColor(QRgb rgb)
{
   cs->setRgb(rgb);
   _q_newColorTypedIn(rgb);
}

// hack; doesn't keep curCol in sync, so use with care
void QColorDialogPrivate::setCurrentQColor(const QColor &color)
{
   Q_Q(QColorDialog);

   if (cs->curQColor != color) {
      cs->curQColor = color;
      emit q->currentColorChanged(color);
   }
}

bool QColorDialogPrivate::selectColor(const QColor &col)
{
   QRgb color = col.rgb();
   int i = 0, j = 0;
   // Check standard colors
   if (standard) {
      for (i = 0; i < 6; i++) {
         for (j = 0; j < 8; j++) {
            if (color == s_standardRGB[i + j * 6]) {
               _q_newStandard(i, j);
               standard->setCurrent(i, j);
               standard->setSelected(i, j);
               standard->setFocus();
               return true;
            }
         }
      }
   }

   // Check custom colors
   if (custom) {
      for (i = 0; i < 2; i++) {
         for (j = 0; j < 8; j++) {
            if (color == s_customRGB[i + j * 2]) {
               _q_newCustom(i, j);
               custom->setCurrent(i, j);
               custom->setSelected(i, j);
               custom->setFocus();
               return true;
            }
         }
      }
   }
   return false;
}

//sets all widgets except cs to display rgb
void QColorDialogPrivate::_q_newColorTypedIn(const QRgb &rgb)
{
   int h, s, v;
   rgb2hsv(rgb, h, s, v);
   cp->setCol(h, s);
   lp->setCol(h, s, v);
}

void QColorDialogPrivate::_q_newCustom(int r, int c)
{
   int i = r + 2 * c;
   setCurrentColor(s_customRGB[i]);
   nextCust = i;
   if (standard) {
      standard->setSelected(-1, -1);
   }
}

void QColorDialogPrivate::_q_newStandard(int r, int c)
{
   setCurrentColor(s_standardRGB[r + c * 6]);

   if (custom) {
      custom->setSelected(-1, -1);
   }
}

void QColorDialogPrivate::init(const QColor &initial)
{
   Q_Q(QColorDialog);

   q->setSizeGripEnabled(false);
   q->setWindowTitle(QColorDialog::tr("Select Color"));

   nativeDialogInUse = false;

   nextCust = 0;
   QVBoxLayout *mainLay = new QVBoxLayout(q);

   // there's nothing in this dialog that benefits from sizing up
   mainLay->setSizeConstraint(QLayout::SetFixedSize);

   QHBoxLayout *topLay = new QHBoxLayout();
   mainLay->addLayout(topLay);

   leftLay = 0;

#if defined(QT_SMALL_COLORDIALOG)
   smallDisplay = true;
   const int lumSpace = 20;
#else
   // small displays (e.g. PDAs) cannot fit the full color dialog,
   // so just use the color picker.
   smallDisplay = (QApplication::desktop()->width() < 480 || QApplication::desktop()->height() < 350);
   const int lumSpace = topLay->spacing() / 2;
#endif

   if (! smallDisplay) {
      leftLay = new QVBoxLayout;
      topLay->addLayout(leftLay);
   }

   initRGB();

#ifndef QT_NO_SETTINGS
   if (! customSet) {
      QSettings settings(QSettings::UserScope, "CopperSpice");

      for (int i = 0; i < 2 * 8; ++i) {
         QVariant v = settings.value("CS/customColors/" + QString::number(i));

         if (v.isValid()) {
            QRgb rgb = v.toUInt();
            s_customRGB[i] = rgb;
         }
      }
   }
#endif

   if (! smallDisplay) {
      standard = new QColorWell(q, 6, 8, s_standardRGB);
      lblBasicColors = new QLabel(q);

#ifndef QT_NO_SHORTCUT
      lblBasicColors->setBuddy(standard);
#endif

      q->connect(standard, SIGNAL(selected(int, int)), q, SLOT(_q_newStandard(int, int)));
      leftLay->addWidget(lblBasicColors);
      leftLay->addWidget(standard);
      leftLay->addStretch();

      custom = new QColorWell(q, 2, 8, s_customRGB);
      custom->setAcceptDrops(true);

      q->connect(custom, SIGNAL(selected(int, int)), q, SLOT(_q_newCustom(int, int)));

      lblCustomColors = new QLabel(q);

#ifndef QT_NO_SHORTCUT
      lblCustomColors->setBuddy(custom);
#endif

      leftLay->addWidget(lblCustomColors);
      leftLay->addWidget(custom);

      addCusBt = new QPushButton(q);
      QObject::connect(addCusBt, SIGNAL(clicked()), q, SLOT(_q_addCustom()));
      leftLay->addWidget(addCusBt);

   } else {
      // better color picker size for small displays

#if defined(QT_SMALL_COLORDIALOG)
      QSize screenSize = QApplication::desktop()->availableGeometry(QCursor::pos()).size();
      pWidth = pHeight = qMin(screenSize.width(), screenSize.height());
      pHeight -= 20;

      if (screenSize.height() > screenSize.width()) {
         pWidth -= 20;
      }
#else
      pWidth = 150;
      pHeight = 100;
#endif
      custom = 0;
      standard = 0;
   }

   QVBoxLayout *rightLay = new QVBoxLayout;
   topLay->addLayout(rightLay);

   QHBoxLayout *pickLay = new QHBoxLayout;
   rightLay->addLayout(pickLay);

   QVBoxLayout *cLay = new QVBoxLayout;
   pickLay->addLayout(cLay);
   cp = new QColorPicker(q);

   cp->setFrameStyle(QFrame::Panel + QFrame::Sunken);

#if defined(QT_SMALL_COLORDIALOG)
   if (!nonTouchUI) {
      pickLay->addWidget(cp);
      cLay->addSpacing(lumSpace);
   } else {
      cp->hide();
   }
#else
   cLay->addSpacing(lumSpace);
   cLay->addWidget(cp);
#endif

   cLay->addSpacing(lumSpace);

   lp = new QColorLuminancePicker(q);

#if defined(QT_SMALL_COLORDIALOG)
   QSize screenSize = QApplication::desktop()->availableGeometry(QCursor::pos()).size();
   const int minDimension = qMin(screenSize.height(), screenSize.width());
   //set picker to be finger-usable
   int pickerWidth = !nonTouchUI ? minDimension / 9 : minDimension / 12;
   lp->setFixedWidth(pickerWidth);

   if (!nonTouchUI) {
      pickLay->addWidget(lp);
   } else {
      lp->hide();
   }
#else
   lp->setFixedWidth(20);
   pickLay->addWidget(lp);
#endif

   QObject::connect(cp, SIGNAL(newCol(int, int)), lp, SLOT(setCol(int, int)));
   QObject::connect(lp, SIGNAL(newHsv(int, int, int)), q, SLOT(_q_newHsv(int, int, int)));

   rightLay->addStretch();

   cs = new QColorShower(q);
   QObject::connect(cs, SIGNAL(newCol(const QRgb &)), q, SLOT(_q_newColorTypedIn(const QRgb &)));
   QObject::connect(cs, SIGNAL(currentColorChanged(const QColor &)), q, SLOT(currentColorChanged(const QColor &)));

#if defined(QT_SMALL_COLORDIALOG)
   if (! nonTouchUI) {
      pWidth -= cp->size().width();
   }
   topLay->addWidget(cs);
#else
   rightLay->addWidget(cs);
#endif

   buttons = new QDialogButtonBox(q);
   mainLay->addWidget(buttons);

   ok = buttons->addButton(QDialogButtonBox::Ok);
   QObject::connect(ok, SIGNAL(clicked()), q, SLOT(accept()));
   ok->setDefault(true);
   cancel = buttons->addButton(QDialogButtonBox::Cancel);
   QObject::connect(cancel, SIGNAL(clicked()), q, SLOT(reject()));

   retranslateStrings();

#ifdef Q_OS_MAC
   delegate = 0;
#endif

   q->setCurrentColor(initial);
}

void QColorDialogPrivate::_q_addCustom()
{
   s_customRGB[nextCust] = cs->currentColor();
   if (custom) {
      custom->update();
   }
   nextCust = (nextCust + 1) % 16;
}

void QColorDialogPrivate::retranslateStrings()
{
   if (!smallDisplay) {
      lblBasicColors->setText(QColorDialog::tr("&Basic colors"));
      lblCustomColors->setText(QColorDialog::tr("&Custom colors"));
      addCusBt->setText(QColorDialog::tr("&Add to Custom Colors"));
   }

   cs->retranslateStrings();
}

static const Qt::WindowFlags DefaultWindowFlags =
   Qt::Dialog | Qt::WindowTitleHint | Qt::MSWindowsFixedSizeDialogHint
   | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint;


QColorDialog::QColorDialog(QWidget *parent)
   : QDialog(*new QColorDialogPrivate, parent, DefaultWindowFlags)
{
   Q_D(QColorDialog);
   d->init(Qt::white);
}


QColorDialog::QColorDialog(const QColor &initial, QWidget *parent)
   : QDialog(*new QColorDialogPrivate, parent, DefaultWindowFlags)
{
   Q_D(QColorDialog);
   d->init(initial);
}

void QColorDialog::setCurrentColor(const QColor &color)
{
   Q_D(QColorDialog);
   d->setCurrentColor(color.rgb());
   d->selectColor(color);
   d->setCurrentAlpha(color.alpha());

#ifdef Q_OS_MAC
   d->setCurrentQColor(color);
   d->setCocoaPanelColor(color);
#endif
   if (d->nativeDialogInUse) {
      qt_guiPlatformPlugin()->colorDialogSetCurrentColor(this, color);
   }
}

QColor QColorDialog::currentColor() const
{
   Q_D(const QColorDialog);
   return d->currentQColor();
}


/*!
    Returns the color that the user selected by clicking the \gui{OK}
    or equivalent button.

    \note This color is not always the same as the color held by the
    \l currentColor property since the user can choose different colors
    before finally selecting the one to use.
*/
QColor QColorDialog::selectedColor() const
{
   Q_D(const QColorDialog);
   return d->selectedQColor;
}

/*!
    Sets the given \a option to be enabled if \a on is true;
    otherwise, clears the given \a option.

    \sa options, testOption()
*/
void QColorDialog::setOption(ColorDialogOption option, bool on)
{
   Q_D(QColorDialog);
   if (!(d->opts & option) != !on) {
      setOptions(d->opts ^ option);
   }
}

/*!
    \since 4.5

    Returns true if the given \a option is enabled; otherwise, returns
    false.

    \sa options, setOption()
*/
bool QColorDialog::testOption(ColorDialogOption option) const
{
   Q_D(const QColorDialog);
   return (d->opts & option) != 0;
}

/*!
    \property QColorDialog::options
    \brief the various options that affect the look and feel of the dialog

    By default, all options are disabled.

    Options should be set before showing the dialog. Setting them while the
    dialog is visible is not guaranteed to have an immediate effect on the
    dialog (depending on the option and on the platform).

    \sa setOption(), testOption()
*/
void QColorDialog::setOptions(ColorDialogOptions options)
{
   Q_D(QColorDialog);

   ColorDialogOptions changed = (options ^ d->opts);
   if (!changed) {
      return;
   }

   d->opts = options;
   d->buttons->setVisible(!(options & NoButtons));
   d->showAlpha(options & ShowAlphaChannel);
}

QColorDialog::ColorDialogOptions QColorDialog::options() const
{
   Q_D(const QColorDialog);
   return d->opts;
}

#ifdef Q_OS_MAC
// can only have one Cocoa color panel active
bool QColorDialogPrivate::sharedColorPanelAvailable = true;
#endif

void QColorDialog::setVisible(bool visible)
{
   Q_D(QColorDialog);

   if (visible) {
      if (testAttribute(Qt::WA_WState_ExplicitShowHide) && !testAttribute(Qt::WA_WState_Hidden)) {
         return;
      }
   } else if (testAttribute(Qt::WA_WState_ExplicitShowHide) && testAttribute(Qt::WA_WState_Hidden)) {
      return;
   }

   if (visible) {
      d->selectedQColor = QColor();
   }

#if defined(Q_OS_MAC)
   if (visible) {
      if (d->delegate || (QColorDialogPrivate::sharedColorPanelAvailable &&
                          !(testAttribute(Qt::WA_DontShowOnScreen) || (d->opts & DontUseNativeDialog)))) {
         d->openCocoaColorPanel(currentColor(), parentWidget(), windowTitle(), options());
         QColorDialogPrivate::sharedColorPanelAvailable = false;
         setAttribute(Qt::WA_DontShowOnScreen);
      }
      setWindowFlags(windowModality() == Qt::WindowModal ? Qt::Sheet : DefaultWindowFlags);
   } else {
      if (d->delegate) {
         d->closeCocoaColorPanel();
         QColorDialogPrivate::sharedColorPanelAvailable = true;
         setAttribute(Qt::WA_DontShowOnScreen, false);
      }
   }
#else

   if (!(d->opts & DontUseNativeDialog) && qt_guiPlatformPlugin()->colorDialogSetVisible(this, visible)) {
      d->nativeDialogInUse = true;
      // Set WA_DontShowOnScreen so that QDialog::setVisible(visible) below
      // updates the state correctly, but skips showing the non-native version:
      setAttribute(Qt::WA_DontShowOnScreen);
   } else {
      d->nativeDialogInUse = false;
      setAttribute(Qt::WA_DontShowOnScreen, false);
   }
#endif

   QDialog::setVisible(visible);
}

void QColorDialog::open(QObject *receiver, const QString &member)
{
   Q_D(QColorDialog);

   connect(this, SIGNAL(colorSelected(const QColor &)), receiver, member);
   d->receiverToDisconnectOnClose = receiver;
   d->memberToDisconnectOnClose   = member;

   QDialog::open();
}

QColor QColorDialog::getColor(const QColor &initial, QWidget *parent, const QString &title, ColorDialogOptions options)
{
   QColorDialog dlg(parent);

   if (! title.isEmpty()) {
      dlg.setWindowTitle(title);
   }

   dlg.setOptions(options);
   dlg.setCurrentColor(initial);
   dlg.exec();

   return dlg.selectedColor();
}

QColorDialog::~QColorDialog()
{
   Q_D(QColorDialog);

#if defined(Q_OS_MAC)
   if (d->delegate) {
      d->releaseCocoaColorPanelDelegate();
      QColorDialogPrivate::sharedColorPanelAvailable = true;
   }
#endif

#ifndef QT_NO_SETTINGS
   if (!customSet) {
      QSettings settings(QSettings::UserScope, QLatin1String("CopperSpice"));

      for (int i = 0; i < 2 * 8; ++i) {
         settings.setValue(QLatin1String("CS/customColors/") + QString::number(i), s_customRGB[i]);
      }
   }
#endif
   if (d->nativeDialogInUse) {
      qt_guiPlatformPlugin()->colorDialogDelete(this);
   }

}


/*!
    \reimp
*/
void QColorDialog::changeEvent(QEvent *e)
{
   Q_D(QColorDialog);
   if (e->type() == QEvent::LanguageChange) {
      d->retranslateStrings();
   }
   QDialog::changeEvent(e);
}

/*!
  Closes the dialog and sets its result code to \a result. If this dialog
  is shown with exec(), done() causes the local event loop to finish,
  and exec() to return \a result.

  \sa QDialog::done()
*/
void QColorDialog::done(int result)
{
   Q_D(QColorDialog);

   QDialog::done(result);

   if (result == Accepted) {
      d->selectedQColor = d->currentQColor();
      emit colorSelected(d->selectedQColor);

   } else {
      d->selectedQColor = QColor();
   }

   if (d->receiverToDisconnectOnClose) {
      disconnect(this, SIGNAL(colorSelected(const QColor &)), d->receiverToDisconnectOnClose, d->memberToDisconnectOnClose);

      d->receiverToDisconnectOnClose = 0;
   }
   d->memberToDisconnectOnClose.clear();
}


void QColorDialog::_q_addCustom()
{
   Q_D(QColorDialog);
   d->_q_addCustom();
}

void QColorDialog::_q_newHsv(int h, int s, int v)
{
   Q_D(QColorDialog);
   d->_q_newHsv(h, s, v);
}

void QColorDialog::_q_newColorTypedIn(const QRgb &rgb)
{
   Q_D(QColorDialog);
   d->_q_newColorTypedIn(rgb);
}

void QColorDialog::_q_newCustom(int un_named_arg1, int un_named_arg2)
{
   Q_D(QColorDialog);
   d->_q_newCustom(un_named_arg1, un_named_arg2);
}

void QColorDialog::_q_newStandard(int un_named_arg1, int un_named_arg2)
{
   Q_D(QColorDialog);
   d->_q_newStandard(un_named_arg1, un_named_arg2);
}

#if defined(Q_OS_MAC)
void QColorDialog::_q_macRunNativeAppModalPanel()
{
   Q_D(QColorDialog);
   d->_q_macRunNativeAppModalPanel();
}
#endif

QT_END_NAMESPACE


#endif // QT_NO_COLORDIALOG


