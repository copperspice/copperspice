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

#include <qcolordialog.h>

#ifndef QT_NO_COLORDIALOG

#include <qapplication.h>
#include <qcursor.h>
#include <qdesktopwidget.h>
#include <qdialogbuttonbox.h>
#include <qdrag.h>
#include <qdrawutil.h>
#include <qevent.h>
#include <qimage.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qmenu.h>
#include <qmimedata.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qpushbutton.h>
#include <qscreen.h>
#include <qsettings.h>
#include <qsharedpointer.h>
#include <qspinbox.h>
#include <qstyle.h>
#include <qstyleoption.h>
#include <qtimer.h>
#include <qvalidator.h>
#include <qwindow.h>

#include <qdialog_p.h>

#include <algorithm>

class QColorLuminancePicker;
class QColorPicker;
class QColorPickingEventFilter;
class QColorShower;
class QWellArray;

class QColorDialogPrivate : public QDialogPrivate
{
   Q_DECLARE_PUBLIC(QColorDialog)

 public:
   enum SetColorMode {
      ShowColor = 0x1,
      SelectColor = 0x2,
      SetColorAll = ShowColor | SelectColor
   };

   QColorDialogPrivate() : options(new QColorDialogOptions)
#ifdef Q_OS_WIN
      , updateTimer(nullptr)
#endif
   {}

   QPlatformColorDialogHelper *platformColorDialogHelper() const {
      return static_cast<QPlatformColorDialogHelper *>(platformHelper());
   }

   void init(const QColor &initial);
   void initWidgets();
   QRgb currentColor() const;
   QColor currentQColor() const;
   void setCurrentColor(const QColor &color, SetColorMode setColorMode = SetColorAll);
   void setCurrentRgbColor(QRgb rgb);
   void setCurrentQColor(const QColor &color);
   bool selectColor(const QColor &color);
   QColor grabScreenColor(const QPoint &p);

   int currentAlpha() const;
   void setCurrentAlpha(int a);
   void showAlpha(bool b);
   bool isAlphaVisible() const;
   void retranslateStrings();

   void _q_addCustom();

   void _q_newHsv(int h, int s, int v);
   void _q_newColorTypedIn(QRgb rgb);
   void _q_nextCustom(int, int);
   void _q_newCustom(int, int);
   void _q_newStandard(int, int);
   void _q_pickScreenColor();
   void _q_updateColorPicking();
   void updateColorLabelText(const QPoint &);
   void updateColorPicking(const QPoint &pos);
   void releaseColorPicking();
   bool handleColorPickingMouseMove(QMouseEvent *e);
   bool handleColorPickingMouseButtonRelease(QMouseEvent *e);
   bool handleColorPickingKeyPress(QKeyEvent *e);

   bool canBeNativeDialog() const override;

   QWellArray *custom;
   QWellArray *standard;

   QDialogButtonBox *buttons;
   QVBoxLayout *leftLay;
   QColorPicker *cp;
   QColorLuminancePicker *lp;
   QColorShower *cs;
   QLabel *lblBasicColors;
   QLabel *lblCustomColors;
   QLabel *lblScreenColorInfo;

   QPushButton *okButton;
   QPushButton *cancelButton;

   QPushButton *addCusBt;
   QPushButton *screenColorPickerButton;
   QColor selectedQColor;
   int nextCust;
   bool smallDisplay;
   bool screenColorPicking;
   QColorPickingEventFilter *colorPickingEventFilter;
   QRgb beforeScreenColorPicking;
   QSharedPointer<QColorDialogOptions> options;

   QPointer<QObject> receiverToDisconnectOnClose;
   QString memberToDisconnectOnClose;

#ifdef Q_OS_WIN
   QTimer *updateTimer;
   QWindow dummyTransparentWindow;
#endif

 private:
   virtual void initHelper(QPlatformDialogHelper *h) override;
   virtual void helperPrepareShow(QPlatformDialogHelper *h) override;
};

class QWellArray : public QWidget
{
   GUI_CS_OBJECT(QWellArray)

   GUI_CS_PROPERTY_READ(selectedColumn, selectedColumn)
   GUI_CS_PROPERTY_READ(selectedRow, selectedRow)

 public:
   QWellArray(int rows, int cols, QWidget *parent = nullptr);

   QWellArray(const QWellArray &) = delete;
   QWellArray &operator=(const QWellArray &) = delete;

   ~QWellArray()
   {
   }

   QString cellContent(int row, int col) const;

   int selectedColumn() const {
      return selCol;
   }

   int selectedRow() const {
      return selRow;
   }

   virtual void setCurrent(int row, int col);
   virtual void setSelected(int row, int col);

   QSize sizeHint() const override;

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

   GUI_CS_SIGNAL_1(Public, void currentChanged(int row, int col))
   GUI_CS_SIGNAL_2(currentChanged, row, col)

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
   int nrows;
   int ncols;
   int cellw;
   int cellh;
   int curRow;
   int curCol;
   int selRow;
   int selCol;
};

void QWellArray::paintEvent(QPaintEvent *e)
{
   QRect r = e->rect();

   int cx = r.x();
   int cy = r.y();
   int ch = r.height();
   int cw = r.width();

   int colfirst = columnAt(cx);
   int collast  = columnAt(cx + cw);
   int rowfirst = rowAt(cy);
   int rowlast  = rowAt(cy + ch);

   if (isRightToLeft()) {
      int t = colfirst;
      colfirst = collast;
      collast = t;
   }

   QPainter painter(this);
   QPainter *p = &painter;

   QRect newRect(0, 0, cellWidth(), cellHeight());

   if (collast < 0 || collast >= ncols) {
      collast = ncols - 1;
   }

   if (rowlast < 0 || rowlast >= nrows) {
      rowlast = nrows - 1;
   }

   // Go through the rows
   for (int row = rowfirst; row <= rowlast; ++row) {

      // get row position and height
      int rowPos = rowY(row);

      // Go through the columns in the row if we know from where to where,
      // go through [colfirst, collast], else go through all of them

      for (int col = colfirst; col <= collast; ++col) {
         // get position and width of column col
         int colPos = columnX(col);

         // Translate painter and draw the cell
         newRect.translate(colPos, rowPos);
         paintCell(p, row, col, newRect);
         newRect.translate(-colPos, -rowPos);
      }
   }
}

QWellArray::QWellArray(int rows, int cols, QWidget *parent)
   : QWidget(parent), nrows(rows), ncols(cols)
{
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
   int b = 3;    // margin

   const QPalette &g = palette();
   QStyleOptionFrame opt;

   int dfw = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
   opt.lineWidth    = dfw;
   opt.midLineWidth = 1;

   opt.rect    = rect.adjusted(b, b, -b, -b);
   opt.palette = g;
   opt.state   = QStyle::State_Enabled | QStyle::State_Sunken;

   style()->drawPrimitive(QStyle::PE_Frame, &opt, p, this);
   b += dfw;

   if ((row == curRow) && (col == curCol)) {
      if (hasFocus()) {
         QStyleOptionFocusRect tmpOption;
         tmpOption.palette = g;
         tmpOption.rect    = rect;
         tmpOption.state   = QStyle::State_None | QStyle::State_KeyboardFocusChange;
         style()->drawPrimitive(QStyle::PE_FrameFocusRect, &tmpOption, p, this);
      }
   }

   paintCellContents(p, row, col, opt.rect.adjusted(dfw, dfw, -dfw, -dfw));
}

void QWellArray::paintCellContents(QPainter *p, int row, int col, const QRect &r)
{
   (void) row;
   (void) col;

   p->fillRect(r, Qt::white);
   p->setPen(Qt::black);
   p->drawLine(r.topLeft(), r.bottomRight());
   p->drawLine(r.topRight(), r.bottomLeft());

}

void QWellArray::mousePressEvent(QMouseEvent *e)
{
   // The current cell marker is set to the cell the mouse is pressed in
   QPoint pos = e->pos();
   setCurrent(rowAt(pos.y()), columnAt(pos.x()));
}

void QWellArray::mouseReleaseEvent(QMouseEvent *)
{
   // The current cell marker is set to the cell the mouse is clicked in
   setSelected(curRow, curCol);
}

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
   emit currentChanged(curRow, curCol);
}

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
   emit currentChanged(curRow, curCol);
}

void QWellArray::focusOutEvent(QFocusEvent *)
{
   updateCell(curRow, curCol);
}

void QWellArray::keyPressEvent(QKeyEvent *e)
{
   switch (e->key()) {                         // Look at the key code
      case Qt::Key_Left:                       // If 'left arrow'-key,
         if (curCol > 0) {                     // and cr't not in leftmost col
            setCurrent(curRow, curCol - 1);    // set cr't to next left column
         }
         break;

      case Qt::Key_Right:                      // Correspondingly...
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

// Event filter to be installed on the dialog while in color-picking mode.
class QColorPickingEventFilter : public QObject
{
 public:
   explicit QColorPickingEventFilter(QColorDialogPrivate *dp, QObject *parent = nullptr) : QObject(parent), m_dp(dp) {}

   bool eventFilter(QObject *, QEvent *event) override {
      switch (event->type()) {
         case QEvent::MouseMove:
            return m_dp->handleColorPickingMouseMove(static_cast<QMouseEvent *>(event));

         case QEvent::MouseButtonRelease:
            return m_dp->handleColorPickingMouseButtonRelease(static_cast<QMouseEvent *>(event));

         case QEvent::KeyPress:
            return m_dp->handleColorPickingKeyPress(static_cast<QKeyEvent *>(event));

         default:
            break;
      }

      return false;
   }

 private:
   QColorDialogPrivate *m_dp;
};

int QColorDialog::customCount()
{
   return QColorDialogOptions::customColorCount();
}

QColor QColorDialog::customColor(int index)
{
   return QColor(QColorDialogOptions::customColor(index));
}

void QColorDialog::setCustomColor(int index, QColor color)
{
   QColorDialogOptions::setCustomColor(index, color.rgba());
}

QColor QColorDialog::standardColor(int index)
{
   return QColor(QColorDialogOptions::standardColor(index));
}

void QColorDialog::setStandardColor(int index, QColor color)
{
   QColorDialogOptions::setStandardColor(index, color.rgba());
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
   if ((e->mimeData()->colorData()).value<QColor>().isValid()) {
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
   if ((e->mimeData()->colorData()).value<QColor>().isValid()) {
      setCurrent(rowAt(e->pos().y()), columnAt(e->pos().x()));
      e->accept();
   } else {
      e->ignore();
   }
}

void QColorWell::dropEvent(QDropEvent *e)
{
   QColor col = (e->mimeData()->colorData()).value<QColor>();
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
   if (! mousePressed) {
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

   void setCrossVisible(bool visible);

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
   bool crossVisible;
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
   // frame and contents offset
   static constexpr const int F_Offset = 3;
   static constexpr const int C_Offset = 4;

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
   int d = height() - 2 * C_Offset - 1;

   return 255 - (y - C_Offset) * 255 / d;
}

int QColorLuminancePicker::val2y(int v)
{
   int d = height() - 2 * C_Offset - 1;
   return C_Offset + (255 - v) * d / 255;
}

QColorLuminancePicker::QColorLuminancePicker(QWidget *parent)
   : QWidget(parent)
{
   hue = 100;
   val = 100;
   sat = 100;
   pix = nullptr;

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
   pix = nullptr;

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

   QRect r(0, F_Offset, w, height() - 2 * F_Offset);
   int wi = r.width() - 2;
   int hi = r.height() - 2;

   if (!pix || pix->height() != hi || pix->width() != wi) {
      delete pix;

      QImage img(wi, hi, QImage::Format_RGB32);
      int y;
      uint *pixel = (uint *) img.scanLine(0);

      for (y = 0; y < hi; y++) {
         uint *end = pixel + wi;

         std::fill(pixel, end, QColor::fromHsv(hue, sat, y2val(y + C_Offset)).rgb());
         pixel = end;
      }

      pix = new QPixmap(QPixmap::fromImage(img));
   }

   QPainter p(this);
   p.drawPixmap(1, C_Offset, *pix);

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

void QColorLuminancePicker::setCol(int h, int s, int v)
{
   val = v;
   hue = h;
   sat = s;
   delete pix;
   pix = nullptr;
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
   : QFrame(parent), crossVisible(true)
{
   hue = 0;
   sat = 0;
   setCol(150, 255);

   setAttribute(Qt::WA_NoSystemBackground);
   setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
}

QColorPicker::~QColorPicker()
{
}

void QColorPicker::setCrossVisible(bool visible)
{
   if (crossVisible != visible) {
      crossVisible = visible;
      update();
   }
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
   if (crossVisible) {
      QPoint pt = colPt() + r.topLeft();
      p.setPen(Qt::black);

      p.fillRect(pt.x() - 9, pt.y(), 20, 2, Qt::black);
      p.fillRect(pt.x(), pt.y() - 9, 2, 20, Qt::black);
   }

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

   GUI_CS_SIGNAL_1(Public, void newCol(QRgb rgb))
   GUI_CS_SIGNAL_2(newCol, rgb)

   GUI_CS_SIGNAL_1(Public, void currentColorChanged(const QColor &color))
   GUI_CS_SIGNAL_2(currentColorChanged, color)

   GUI_CS_SLOT_1(Public, void setRgb(QRgb rgb))
   GUI_CS_SLOT_2(setRgb)

 private:
   GUI_CS_SLOT_1(Private, void rgbEd())
   GUI_CS_SLOT_2(rgbEd)

   GUI_CS_SLOT_1(Private, void hsvEd())
   GUI_CS_SLOT_2(hsvEd)

   GUI_CS_SLOT_1(Private, void htmlEd())
   GUI_CS_SLOT_2(htmlEd)

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
   QLabel *lblHtml;
   QColSpinBox *hEd;
   QColSpinBox *sEd;
   QColSpinBox *vEd;
   QColSpinBox *rEd;
   QColSpinBox *gEd;
   QColSpinBox *bEd;
   QColSpinBox *alphaEd;
   QLabel *alphaLab;
   QLineEdit *htEd;
   QColorShowLabel *lab;
   bool rgbOriginal;

   QColorDialog *colorDialog;
   QGridLayout *gl;

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

   GUI_CS_SIGNAL_1(Public, void colorDropped(QRgb arg))
   GUI_CS_SIGNAL_2(colorDropped, arg)

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
   (void) e;

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
   if ((e->mimeData()->colorData()).value<QColor>().isValid()) {
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
   QColor color = (e->mimeData()->colorData()).value<QColor>();
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

   gl = new QGridLayout(this);
   gl->setMargin(gl->spacing());

   lab = new QColorShowLabel(this);

#ifdef QT_SMALL_COLORDIALOG
   lab->setMinimumHeight(60);
#endif

   lab->setMinimumWidth(60);

   // On mobile, due to small screen and different screen layouts need to re-arrange the widgets.
   // For QVGA screens only the comboboxes and color label are visible.
   // For nHD screens only color and luminence pickers and color label are visible.

#if ! defined(QT_SMALL_COLORDIALOG)
   gl->addWidget(lab, 0, 0, -1, 1);

#else
   gl->addWidget(lab, 0, 0, 1, -1);

#endif

   connect(lab, &QColorShowLabel::colorDropped, this, &QColorShower::newCol);
   connect(lab, &QColorShowLabel::colorDropped, this, &QColorShower::setRgb);

   hEd = new QColSpinBox(this);
   hEd->setRange(0, 359);
   lblHue = new QLabel(this);

#ifndef QT_NO_SHORTCUT
   lblHue->setBuddy(hEd);
#endif

   lblHue->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

#if ! defined(QT_SMALL_COLORDIALOG)
   gl->addWidget(lblHue, 0, 1);
   gl->addWidget(hEd, 0, 2);
#else
   gl->addWidget(lblHue, 1, 0);
   gl->addWidget(hEd, 2, 0);
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
   gl->addWidget(lblSat, 1, 1);
   gl->addWidget(sEd, 2, 1);
#endif

   vEd = new QColSpinBox(this);
   lblVal = new QLabel(this);

#ifndef QT_NO_SHORTCUT
   lblVal->setBuddy(vEd);
#endif

   lblVal->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

#if ! defined(QT_SMALL_COLORDIALOG)
   gl->addWidget(lblVal, 2, 1);
   gl->addWidget(vEd, 2, 2);
#else
   gl->addWidget(lblVal, 1, 2);
   gl->addWidget(vEd, 2, 2);
#endif

   rEd = new QColSpinBox(this);
   lblRed = new QLabel(this);

#ifndef QT_NO_SHORTCUT
   lblRed->setBuddy(rEd);
#endif

   lblRed->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

#if ! defined(QT_SMALL_COLORDIALOG)
   gl->addWidget(lblRed, 0, 3);
   gl->addWidget(rEd, 0, 4);
#else

   gl->addWidget(lblRed, 3, 0);
   gl->addWidget(rEd, 4, 0);

#endif

   gEd = new QColSpinBox(this);
   lblGreen = new QLabel(this);

#ifndef QT_NO_SHORTCUT
   lblGreen->setBuddy(gEd);
#endif

   lblGreen->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

#if ! defined(QT_SMALL_COLORDIALOG)
   gl->addWidget(lblGreen, 1, 3);
   gl->addWidget(gEd, 1, 4);
#else
   gl->addWidget(lblGreen, 3, 1);
   gl->addWidget(gEd, 4, 1);
#endif

   bEd = new QColSpinBox(this);
   lblBlue = new QLabel(this);

#ifndef QT_NO_SHORTCUT
   lblBlue->setBuddy(bEd);
#endif

   lblBlue->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

#if ! defined(QT_SMALL_COLORDIALOG)
   gl->addWidget(lblBlue, 2, 3);
   gl->addWidget(bEd, 2, 4);
#else
   gl->addWidget(lblBlue, 3, 2);
   gl->addWidget(bEd, 4, 2);
#endif

   alphaEd = new QColSpinBox(this);
   alphaLab = new QLabel(this);

#ifndef QT_NO_SHORTCUT
   alphaLab->setBuddy(alphaEd);
#endif

   alphaLab->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

#if ! defined(QT_SMALL_COLORDIALOG)
   gl->addWidget(alphaLab, 3, 1, 1, 3);
   gl->addWidget(alphaEd, 3, 4);

#else
   gl->addWidget(alphaLab, 1, 3, 3, 1);
   gl->addWidget(alphaEd, 4, 3);

#endif

   alphaEd->hide();
   alphaLab->hide();

   lblHtml = new QLabel(this);
   htEd    = new QLineEdit(this);

#ifndef QT_NO_SHORTCUT
   lblHtml->setBuddy(htEd);
#endif

   QRegularExpression regExp("#([A-Fa-f0-9]{6}|[A-Fa-f0-9]{3})");
   QRegularExpressionValidator *validator = new QRegularExpressionValidator(regExp, this);

   htEd->setValidator(validator);
   htEd->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);
   lblHtml->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

#if defined(QT_SMALL_COLORDIALOG)
   gl->addWidget(lblHtml, 5, 0);
   gl->addWidget(htEd, 5, 1, 1, 2);
#else
   gl->addWidget(lblHtml, 5, 1);
   gl->addWidget(htEd, 5, 2, 1, 3);
#endif

   connect(hEd,     cs_mp_cast<int>(&QColSpinBox::valueChanged), this, &QColorShower::hsvEd);
   connect(sEd,     cs_mp_cast<int>(&QColSpinBox::valueChanged), this, &QColorShower::hsvEd);
   connect(vEd,     cs_mp_cast<int>(&QColSpinBox::valueChanged), this, &QColorShower::hsvEd);
   connect(rEd,     cs_mp_cast<int>(&QColSpinBox::valueChanged), this, &QColorShower::rgbEd);
   connect(gEd,     cs_mp_cast<int>(&QColSpinBox::valueChanged), this, &QColorShower::rgbEd);
   connect(bEd,     cs_mp_cast<int>(&QColSpinBox::valueChanged), this, &QColorShower::rgbEd);
   connect(alphaEd, cs_mp_cast<int>(&QColSpinBox::valueChanged), this, &QColorShower::rgbEd);

   connect(htEd,    &QLineEdit::textEdited,     this, &QColorShower::htmlEd);

   retranslateStrings();
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
   if (nativeDialogInUse) {
      return platformColorDialogHelper()->currentColor();
   }

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
   htEd->setText(QColor(curCol).name());

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

   htEd->setText(c.name());
   showCurrentColor();
   emit newCol(currentColor());
   updateQColor();
}

void QColorShower::htmlEd()
{
   QColor c;
   QString t = htEd->text();
   c.setNamedColor(t);

   if (!c.isValid()) {
      return;
   }

   curCol = qRgba(c.red(), c.green(), c.blue(), currentAlpha());
   rgb2hsv(curCol, hue, sat, val);

   hEd->setValue(hue);
   sEd->setValue(sat);
   vEd->setValue(val);

   rEd->setValue(qRed(currentColor()));
   gEd->setValue(qGreen(currentColor()));
   bEd->setValue(qBlue(currentColor()));

   showCurrentColor();
   emit newCol(currentColor());
   updateQColor();
}
void QColorShower::setRgb(QRgb rgb)
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

   htEd->setText(QColor(rgb).name());
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
   htEd->setText(c.name());

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
   lblHtml->setText(QColorDialog::tr("&HTML:"));
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
   if (!nativeDialogInUse) {
      cs->setHsv(h, s, v);
      cp->setCol(h, s);
      lp->setCol(h, s, v);
   }
}

// sets all widgets to display rgb
void QColorDialogPrivate::setCurrentRgbColor(QRgb rgb)
{
   if (!nativeDialogInUse) {
      cs->setRgb(rgb);
      _q_newColorTypedIn(rgb);
   }
}

// does not keep curCol in sync,use with care
void QColorDialogPrivate::setCurrentQColor(const QColor &color)
{
   Q_Q(QColorDialog);

   if (cs->curQColor != color) {
      cs->curQColor = color;
      emit q->currentColorChanged(color);
   }
}

// size of standard and custom color selector
static constexpr const int ColorColumns      = 8;
static constexpr const int StandardColorRows = 6;
static constexpr const int CustomColorRows   = 2;

bool QColorDialogPrivate::selectColor(const QColor &col)
{
   QRgb color = col.rgb();

   // Check standard colors
   if (standard) {
      const QRgb *standardColors    = QColorDialogOptions::standardColors();
      const QRgb *standardColorsEnd = standardColors + StandardColorRows * ColorColumns;
      const QRgb *match = std::find(standardColors, standardColorsEnd, color);

      if (match != standardColorsEnd) {
         const int index    = int(match - standardColors);
         const int column = index / StandardColorRows;
         const int row    = index % StandardColorRows;

         _q_newStandard(row, column);
         standard->setCurrent(row, column);
         standard->setSelected(row, column);
         standard->setFocus();

         return true;
      }
   }


   // Check custom colors
   if (custom) {
      const QRgb *customColors    = QColorDialogOptions::customColors();
      const QRgb *customColorsEnd = customColors + CustomColorRows * ColorColumns;
      const QRgb *match = std::find(customColors, customColorsEnd, color);

      if (match != customColorsEnd) {
         const int index = int(match - customColors);
         const int column = index / CustomColorRows;
         const int row = index % CustomColorRows;

         _q_newCustom(row, column);

         custom->setCurrent(row, column);
         custom->setSelected(row, column);
         custom->setFocus();
         return true;
      }
   }

   return false;
}

QColor QColorDialogPrivate::grabScreenColor(const QPoint &p)
{
   const QDesktopWidget *desktop = QApplication::desktop();
   const QPixmap pixmap = QGuiApplication::screens().at(desktop->screenNumber())->grabWindow(desktop->winId(),
         p.x(), p.y(), 1, 1);
   QImage i = pixmap.toImage();

   return i.pixel(0, 0);
}

//sets all widgets except cs to display rgb
void QColorDialogPrivate::_q_newColorTypedIn(QRgb rgb)
{
   if (!nativeDialogInUse) {
      int h, s, v;
      rgb2hsv(rgb, h, s, v);
      cp->setCol(h, s);
      lp->setCol(h, s, v);
   }
}

void QColorDialogPrivate::_q_nextCustom(int r, int c)
{
   nextCust = r + CustomColorRows * c;
}

void QColorDialogPrivate::_q_newCustom(int r, int c)
{
   const int i = r + CustomColorRows * c;
   setCurrentRgbColor(QColorDialogOptions::customColor(i));

   if (standard) {
      standard->setSelected(-1, -1);
   }
}

void QColorDialogPrivate::_q_newStandard(int r, int c)
{
   setCurrentRgbColor(QColorDialogOptions::standardColor(r + c * 6));

   if (custom) {
      custom->setSelected(-1, -1);
   }
}


void QColorDialogPrivate::_q_pickScreenColor()
{
   Q_Q(QColorDialog);
   if (!colorPickingEventFilter) {
      colorPickingEventFilter = new QColorPickingEventFilter(this);
   }
   q->installEventFilter(colorPickingEventFilter);
   // If user pushes Escape, the last color before picking will be restored.
   beforeScreenColorPicking = cs->currentColor();

#ifndef QT_NO_CURSOR
   q->grabMouse(Qt::CrossCursor);
#else
   q->grabMouse();
#endif

#ifdef Q_OS_WIN
   // excludes WinRT

   updateTimer->start(30);
   dummyTransparentWindow.show();
#endif

   q->grabKeyboard();
   q->setMouseTracking(true);
   addCusBt->setDisabled(true);
   buttons->setDisabled(true);
   screenColorPickerButton->setDisabled(true);
   const QPoint globalPos = QCursor::pos();
   q->setCurrentColor(grabScreenColor(globalPos));
   updateColorLabelText(globalPos);
}

void QColorDialogPrivate::updateColorLabelText(const QPoint &globalPos)
{
   lblScreenColorInfo->setText(QColorDialog::tr("Cursor at %1, %2\nPress ESC to cancel")
      .formatArg(globalPos.x())
      .formatArg(globalPos.y()));
}
void QColorDialogPrivate::releaseColorPicking()
{
   Q_Q(QColorDialog);
   cp->setCrossVisible(true);
   q->removeEventFilter(colorPickingEventFilter);
   q->releaseMouse();

#ifdef Q_OS_WIN
   updateTimer->stop();
   dummyTransparentWindow.setVisible(false);
#endif

   q->releaseKeyboard();
   q->setMouseTracking(false);
   lblScreenColorInfo->setText(QString("\n"));
   addCusBt->setDisabled(false);
   buttons->setDisabled(false);
   screenColorPickerButton->setDisabled(false);
}

void QColorDialogPrivate::init(const QColor &initial)
{
   Q_Q(QColorDialog);

   q->setSizeGripEnabled(false);
   q->setWindowTitle(QColorDialog::tr("Select Color"));

   nativeDialogInUse = (platformColorDialogHelper() != nullptr);
   colorPickingEventFilter = nullptr;
   nextCust = 0;
   if (!nativeDialogInUse) {
      initWidgets();
   }

#ifdef Q_OS_WIN
   dummyTransparentWindow.resize(1, 1);
   dummyTransparentWindow.setFlags(Qt::Tool | Qt::FramelessWindowHint);
#endif

   q->setCurrentColor(initial);
}

void QColorDialogPrivate::initWidgets()
{
   Q_Q(QColorDialog);

   QVBoxLayout *mainLay = new QVBoxLayout(q);

   // there's nothing in this dialog that benefits from sizing up
   mainLay->setSizeConstraint(QLayout::SetFixedSize);

   QHBoxLayout *topLay = new QHBoxLayout();
   mainLay->addLayout(topLay);

   leftLay = nullptr;

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

      standard       = new QColorWell(q, StandardColorRows, ColorColumns, QColorDialogOptions::standardColors());
      lblBasicColors = new QLabel(q);

#ifndef QT_NO_SHORTCUT
      lblBasicColors->setBuddy(standard);
#endif

      q->connect(standard, &QColorWell::selected, q, &QColorDialog::_q_newStandard);
      leftLay->addWidget(lblBasicColors);
      leftLay->addWidget(standard);

#if ! defined(QT_SMALL_COLORDIALOG)
      // The screen color picker button
      screenColorPickerButton = new QPushButton();
      leftLay->addWidget(screenColorPickerButton);

      lblScreenColorInfo = new QLabel("\n");
      leftLay->addWidget(lblScreenColorInfo);
      q->connect(screenColorPickerButton, &QPushButton::clicked, q, &QColorDialog::_q_pickScreenColor);
#endif

      leftLay->addStretch();

      custom = new QColorWell(q, CustomColorRows, ColorColumns, QColorDialogOptions::customColors());
      custom->setAcceptDrops(true);

      QObject::connect(custom, &QColorWell::selected,       q, &QColorDialog::_q_newCustom);
      QObject::connect(custom, &QColorWell::currentChanged, q, &QColorDialog::_q_nextCustom);

      lblCustomColors = new QLabel(q);

#ifndef QT_NO_SHORTCUT
      lblCustomColors->setBuddy(custom);
#endif

      leftLay->addWidget(lblCustomColors);
      leftLay->addWidget(custom);

      addCusBt = new QPushButton(q);
      QObject::connect(addCusBt, &QPushButton::clicked, q, &QColorDialog::_q_addCustom);
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
      custom = nullptr;
      standard = nullptr;
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
   cp->hide();

#else
   cLay->addSpacing(lumSpace);
   cLay->addWidget(cp);
#endif

   cLay->addSpacing(lumSpace);

   lp = new QColorLuminancePicker(q);

#if defined(QT_SMALL_COLORDIALOG)
   lp->hide();

#else
   lp->setFixedWidth(20);
   pickLay->addSpacing(10);
   pickLay->addWidget(lp);
   pickLay->addStretch();
#endif

   QObject::connect(cp, &QColorPicker::newCol,          lp, cs_mp_cast<int, int>(&QColorLuminancePicker::setCol));
   QObject::connect(lp, &QColorLuminancePicker::newHsv, q,  &QColorDialog::_q_newHsv);

   rightLay->addStretch();

   cs = new QColorShower(q);
   pickLay->setMargin(cs->gl->margin());

   QObject::connect(cs, &QColorShower::newCol,              q, &QColorDialog::_q_newColorTypedIn);
   QObject::connect(cs, &QColorShower::currentColorChanged, q, &QColorDialog::currentColorChanged);

#if defined(QT_SMALL_COLORDIALOG)
   topLay->addWidget(cs);
#else
   rightLay->addWidget(cs);
   if (leftLay) {
      leftLay->addSpacing(cs->gl->margin());
   }
#endif

   buttons = new QDialogButtonBox(q);
   mainLay->addWidget(buttons);

   okButton = buttons->addButton(QDialogButtonBox::Ok);
   QObject::connect(okButton, &QPushButton::clicked,     q, &QColorDialog::accept);

   okButton->setDefault(true);

   cancelButton = buttons->addButton(QDialogButtonBox::Cancel);
   QObject::connect(cancelButton, &QPushButton::clicked, q, &QColorDialog::reject);

#ifdef Q_OS_WIN
   updateTimer = new QTimer(q);
   QObject::connect(updateTimer, &QTimer::timeout, q, &QColorDialog::_q_updateColorPicking);
#endif

   retranslateStrings();
}

void QColorDialogPrivate::initHelper(QPlatformDialogHelper *h)
{
   QColorDialog *d = q_func();

   QPlatformColorDialogHelper *tmp = static_cast<QPlatformColorDialogHelper *>(h);

   QObject::connect(tmp, &QPlatformColorDialogHelper::currentColorChanged, d, &QColorDialog::currentColorChanged);
   QObject::connect(tmp, &QPlatformColorDialogHelper::colorSelected,       d, &QColorDialog::colorSelected);

   tmp->setOptions(options);
}

void QColorDialogPrivate::helperPrepareShow(QPlatformDialogHelper *)
{
   options->setWindowTitle(q_func()->windowTitle());
}

void QColorDialogPrivate::_q_addCustom()
{
   QColorDialogOptions::setCustomColor(nextCust, cs->currentColor());
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
      screenColorPickerButton->setText(QColorDialog::tr("&Pick Screen Color"));
   }

   cs->retranslateStrings();
}

bool QColorDialogPrivate::canBeNativeDialog() const
{
   // Don't use Q_Q here! This function is called from ~QDialog,
   // so Q_Q calling q_func() invokes undefined behavior (invalid cast in q_func()).
   const QDialog *const q = static_cast<const QDialog *>(q_ptr);

   if (nativeDialogInUse) {
      return true;
   }

   if (q->testAttribute(Qt::WA_DontShowOnScreen)) {
      return false;
   }

   if (options->options() & QColorDialog::DontUseNativeDialog) {
      return false;
   }

   QString staticName(QColorDialog::staticMetaObject().className());
   QString dynamicName(q->metaObject()->className());

   return (staticName == dynamicName);
}

static constexpr const Qt::WindowFlags DefaultWindowFlags =
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

void QColorDialogPrivate::setCurrentColor(const QColor &color,  SetColorMode setColorMode)
{
   if (nativeDialogInUse) {
      platformColorDialogHelper()->setCurrentColor(color);
      return;
   }
   if (setColorMode & ShowColor) {
      setCurrentRgbColor(color.rgb());
      setCurrentAlpha(color.alpha());
   }
   if (setColorMode & SelectColor) {
      selectColor(color);
   }
}
void QColorDialog::setCurrentColor(const QColor &color)
{
   Q_D(QColorDialog);
   d->setCurrentColor(color);
}

QColor QColorDialog::currentColor() const
{
   Q_D(const QColorDialog);
   return d->currentQColor();
}

QColor QColorDialog::selectedColor() const
{
   Q_D(const QColorDialog);
   return d->selectedQColor;
}

void QColorDialog::setOption(ColorDialogOption option, bool on)
{
   const QColorDialog::ColorDialogOptions previousOptions = options();
   if (!(previousOptions & option) != !on) {
      setOptions(previousOptions ^ option);
   }
}

bool QColorDialog::testOption(ColorDialogOption option) const
{
   Q_D(const QColorDialog);
   return d->options->testOption(static_cast<QColorDialogOptions::ColorDialogOption>(option));
}

void QColorDialog::setOptions(ColorDialogOptions options)
{
   Q_D(QColorDialog);

   if (QColorDialog::options() == options) {
      return;
   }

   d->options->setOptions(QColorDialogOptions::ColorDialogOptions(int(options)));
   if ((options & DontUseNativeDialog) && d->nativeDialogInUse) {
      d->nativeDialogInUse = false;
      d->initWidgets();
   }
   if (!d->nativeDialogInUse) {
      d->buttons->setVisible(!(options & NoButtons));
      d->showAlpha(options & ShowAlphaChannel);
   }
}

QColorDialog::ColorDialogOptions QColorDialog::options() const
{
   Q_D(const QColorDialog);
   return QColorDialog::ColorDialogOptions(int(d->options->options()));
}


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

   if (d->nativeDialogInUse) {
      d->setNativeDialogVisible(visible);
      // Set WA_DontShowOnScreen so that QDialog::setVisible(visible) below
      // updates the state correctly, but skips showing the non-native version:
      setAttribute(Qt::WA_DontShowOnScreen);
   } else {
      setAttribute(Qt::WA_DontShowOnScreen, false);
   }

   QDialog::setVisible(visible);
}

void QColorDialog::open(QObject *receiver, const QString &member)
{
   Q_D(QColorDialog);

   connect(this, SIGNAL(colorSelected(QColor)), receiver, member);
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

QRgb QColorDialog::getRgba(QRgb initial, bool *ok, QWidget *parent)
{
   const QColor color = getColor(QColor::fromRgba(initial), parent, QString(),
         ShowAlphaChannel);
   QRgb result = color.isValid() ? color.rgba() : initial;
   if (ok) {
      *ok = color.isValid();
   }
   return result;
}
QColorDialog::~QColorDialog()
{
}
void QColorDialog::changeEvent(QEvent *e)
{
   Q_D(QColorDialog);
   if (e->type() == QEvent::LanguageChange) {
      d->retranslateStrings();
   }
   QDialog::changeEvent(e);
}
void QColorDialogPrivate::_q_updateColorPicking()
{
#ifndef QT_NO_CURSOR
   Q_Q(QColorDialog);

   static QPoint lastGlobalPos;
   QPoint newGlobalPos = QCursor::pos();

   if (lastGlobalPos == newGlobalPos) {
      return;
   }

   lastGlobalPos = newGlobalPos;

   if (! q->rect().contains(q->mapFromGlobal(newGlobalPos))) {
      // Inside the dialog mouse tracking works, handleColorPickingMouseMove will be called
      updateColorPicking(newGlobalPos);

#ifdef Q_OS_WIN
      dummyTransparentWindow.setPosition(newGlobalPos);
#endif
   }

#endif

}
void QColorDialogPrivate::updateColorPicking(const QPoint &globalPos)
{
   const QColor color = grabScreenColor(globalPos);
   setCurrentColor(color, ShowColor);
   updateColorLabelText(globalPos);
}

bool QColorDialogPrivate::handleColorPickingMouseMove(QMouseEvent *e)
{
   cp->setCrossVisible(!cp->geometry().contains(e->pos()));

   updateColorPicking(e->globalPos());
   return true;
}
bool QColorDialogPrivate::handleColorPickingMouseButtonRelease(QMouseEvent *e)
{
   setCurrentColor(grabScreenColor(e->globalPos()), SetColorAll);
   releaseColorPicking();
   return true;
}

bool QColorDialogPrivate::handleColorPickingKeyPress(QKeyEvent *e)
{
   Q_Q(QColorDialog);
   if (e->matches(QKeySequence::Cancel)) {
      releaseColorPicking();
      q->setCurrentColor(beforeScreenColorPicking);
   } else if (e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter) {
      q->setCurrentColor(grabScreenColor(QCursor::pos()));
      releaseColorPicking();
   }
   e->accept();
   return true;
}

void QColorDialog::done(int result)
{
   Q_D(QColorDialog);

   if (result == Accepted) {
      d->selectedQColor = d->currentQColor();
      emit colorSelected(d->selectedQColor);

   } else {
      d->selectedQColor = QColor();
   }

   QDialog::done(result);

   if (d->receiverToDisconnectOnClose) {
      disconnect(this, SIGNAL(colorSelected(QColor)), d->receiverToDisconnectOnClose, d->memberToDisconnectOnClose);
      d->receiverToDisconnectOnClose = nullptr;
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

void QColorDialog::_q_newColorTypedIn(QRgb rgb)
{
   Q_D(QColorDialog);
   d->_q_newColorTypedIn(rgb);
}

void QColorDialog::_q_newCustom(int arg1, int arg2)
{
   Q_D(QColorDialog);
   d->_q_newCustom(arg1, arg2);
}

void QColorDialog::_q_nextCustom(int arg1, int arg2)
{
   Q_D(QColorDialog);
   d->_q_nextCustom(arg1, arg2);
}

void QColorDialog::_q_newStandard(int arg1, int arg2)
{
   Q_D(QColorDialog);
   d->_q_newStandard(arg1, arg2);
}

void QColorDialog::_q_pickScreenColor()
{
   Q_D(QColorDialog);
   d->_q_pickScreenColor();
}

void QColorDialog::_q_updateColorPicking()
{
   Q_D(QColorDialog);
   d->_q_updateColorPicking();
}

#endif // QT_NO_COLORDIALOG


