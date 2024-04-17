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

#include <qprintpreviewdialog.h>

#include <qaction.h>
#include <qboxlayout.h>
#include <qcombobox.h>
#include <qcoreapplication.h>
#include <qfiledialog.h>
#include <qformlayout.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qmainwindow.h>
#include <qpagesetupdialog.h>
#include <qprintdialog.h>
#include <qprinter.h>
#include <qprintpreviewwidget.h>
#include <qstyle.h>
#include <qtoolbar.h>
#include <qtoolbutton.h>
#include <qvalidator.h>

#include <qdialog_p.h>
#include <qprinter_p.h>

#ifndef QT_NO_PRINTPREVIEWDIALOG

class QPrintPreviewMainWindow : public QMainWindow
{
 public:
   QPrintPreviewMainWindow(QWidget *parent)
      : QMainWindow(parent)
   { }

   QMenu *createPopupMenu() override {
      return nullptr;
   }
};

class ZoomFactorValidator : public QDoubleValidator
{
 public:
   ZoomFactorValidator(QObject *parent)
      : QDoubleValidator(parent)
   { }

   ZoomFactorValidator(qreal bottom, qreal top, int decimals, QObject *parent)
      : QDoubleValidator(bottom, top, decimals, parent)
   { }

   State validate(QString &input, int &pos) const override {
      bool replacePercent = false;
      if (input.endsWith(QLatin1Char('%'))) {
         input = input.left(input.length() - 1);
         replacePercent = true;
      }

      State state = QDoubleValidator::validate(input, pos);
      if (replacePercent) {
         input += QLatin1Char('%');
      }

      const int num_size = 4;
      if (state == Intermediate) {
         int i = input.indexOf(QLocale::system().decimalPoint());

         if ((i == -1 && input.size() > num_size) || (i != -1 && i > num_size)) {
            return Invalid;
         }
      }

      return state;
   }
};

class LineEdit : public QLineEdit
{
   GUI_CS_OBJECT(LineEdit)

 public:
   LineEdit(QWidget *parent = nullptr)
      : QLineEdit(parent)
   {
      setContextMenuPolicy(Qt::NoContextMenu);
      connect(this, &LineEdit::returnPressed, this, &LineEdit::handleReturnPressed);
   }

 protected:
   void focusInEvent(QFocusEvent *e) override {
      origText = text();
      QLineEdit::focusInEvent(e);
   }

   void focusOutEvent(QFocusEvent *e) override {
      if (isModified() && ! hasAcceptableInput()) {
         setText(origText);
      }

      QLineEdit::focusOutEvent(e);
   }

 private:
   GUI_CS_SLOT_1(Private, void handleReturnPressed())
   GUI_CS_SLOT_2(handleReturnPressed)

   QString origText;
};

void LineEdit::handleReturnPressed()
{
   origText = text();
}

class QPrintPreviewDialogPrivate : public QDialogPrivate
{
   Q_DECLARE_PUBLIC(QPrintPreviewDialog)

 public:
   QPrintPreviewDialogPrivate()
      : printDialog(nullptr), ownPrinter(false), initialized(false)
   {
   }

   // private slots
   void _q_fit(QAction *action);
   void _q_zoomIn();
   void _q_zoomOut();
   void _q_navigate(QAction *action);
   void _q_setMode(QAction *action);
   void _q_pageNumEdited();
   void _q_print();
   void _q_pageSetup();
   void _q_previewChanged();
   void _q_zoomFactorChanged();

   void init(QPrinter *printer = nullptr);
   void populateScene();
   void layoutPages();
   void setupActions();
   void updateNavActions();
   void setFitting(bool on);
   bool isFitting();
   void updatePageNumLabel();
   void updateZoomFactor();

   QPrintDialog *printDialog;
   QPrintPreviewWidget *preview;
   QPrinter *printer;
   bool ownPrinter;
   bool initialized;

   // widgets
   QLineEdit *pageNumEdit;
   QLabel *pageNumLabel;
   QComboBox *zoomFactor;

   // actions:
   QActionGroup *navGroup;
   QAction *nextPageAction;
   QAction *prevPageAction;
   QAction *firstPageAction;
   QAction *lastPageAction;

   QActionGroup *fitGroup;
   QAction *fitWidthAction;
   QAction *fitPageAction;

   QActionGroup *zoomGroup;
   QAction *zoomInAction;
   QAction *zoomOutAction;

   QActionGroup *orientationGroup;
   QAction *portraitAction;
   QAction *landscapeAction;

   QActionGroup *modeGroup;
   QAction *singleModeAction;
   QAction *facingModeAction;
   QAction *overviewModeAction;

   QActionGroup *printerGroup;
   QAction *printAction;
   QAction *pageSetupAction;

   QPointer<QObject> receiverToDisconnectOnClose;
   QString memberToDisconnectOnClose;
};

void QPrintPreviewDialogPrivate::init(QPrinter *_printer)
{
   Q_Q(QPrintPreviewDialog);

   if (_printer) {
      preview = new QPrintPreviewWidget(_printer, q);
      printer = _printer;

   } else {
      ownPrinter = true;
      printer = new QPrinter;
      preview = new QPrintPreviewWidget(printer, q);
   }

   QObject::connect(preview, &QPrintPreviewWidget::paintRequested, q, &QPrintPreviewDialog::paintRequested);
   QObject::connect(preview, &QPrintPreviewWidget::previewChanged, q, &QPrintPreviewDialog::_q_previewChanged);
   setupActions();

   pageNumEdit = new LineEdit;
   pageNumEdit->setAlignment(Qt::AlignRight);
   pageNumEdit->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
   pageNumLabel = new QLabel;

   QObject::connect(pageNumEdit, &LineEdit::editingFinished, q, &QPrintPreviewDialog::_q_pageNumEdited);

   zoomFactor = new QComboBox;
   zoomFactor->setEditable(true);
   zoomFactor->setMinimumContentsLength(7);
   zoomFactor->setInsertPolicy(QComboBox::NoInsert);
   LineEdit *zoomEditor = new LineEdit;
   zoomEditor->setValidator(new ZoomFactorValidator(1, 1000, 1, zoomEditor));
   zoomFactor->setLineEdit(zoomEditor);
   static const short factorsX2[] = { 25, 50, 100, 200, 250, 300, 400, 800, 1600 };

   for (int i = 0; i < int(sizeof(factorsX2) / sizeof(factorsX2[0])); ++i) {
      zoomFactor->addItem(QPrintPreviewDialog::tr("%1%").formatArg(factorsX2[i] / 2.0));
   }

   QObject::connect(zoomFactor->lineEdit(), &QLineEdit::editingFinished, q, &QPrintPreviewDialog::_q_zoomFactorChanged);
   QObject::connect(zoomFactor, cs_mp_cast<int>(&QComboBox::currentIndexChanged), q, &QPrintPreviewDialog::_q_zoomFactorChanged);

   QPrintPreviewMainWindow *mw = new QPrintPreviewMainWindow(q);
   QToolBar *toolbar = new QToolBar(mw);

   toolbar->addAction(fitWidthAction);
   toolbar->addAction(fitPageAction);
   toolbar->addSeparator();
   toolbar->addWidget(zoomFactor);
   toolbar->addAction(zoomOutAction);
   toolbar->addAction(zoomInAction);
   toolbar->addSeparator();
   toolbar->addAction(portraitAction);
   toolbar->addAction(landscapeAction);
   toolbar->addSeparator();
   toolbar->addAction(firstPageAction);
   toolbar->addAction(prevPageAction);

   // this is to ensure the label text and the editor text are
   // aligned in all styles - the extra QVBoxLayout is a workaround
   // for bug in QFormLayout
   QWidget *pageEdit = new QWidget(toolbar);
   QVBoxLayout *vboxLayout = new QVBoxLayout;
   vboxLayout->setContentsMargins(0, 0, 0, 0);

#ifdef Q_OS_DARWIN
   // We query the widgets about their size and then we fix the size.
   // This should do the trick for the laying out part...
   QSize pageNumEditSize, pageNumLabelSize;
   pageNumEditSize = pageNumEdit->minimumSizeHint();
   pageNumLabelSize = pageNumLabel->minimumSizeHint();
   pageNumEdit->resize(pageNumEditSize);
   pageNumLabel->resize(pageNumLabelSize);
#endif

   QFormLayout *formLayout = new QFormLayout;

#ifdef Q_OS_DARWIN
   // We have to change the growth policy in Mac.
   formLayout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
#endif

   formLayout->setWidget(0, QFormLayout::LabelRole, pageNumEdit);
   formLayout->setWidget(0, QFormLayout::FieldRole, pageNumLabel);
   vboxLayout->addLayout(formLayout);
   vboxLayout->setAlignment(Qt::AlignVCenter);
   pageEdit->setLayout(vboxLayout);
   toolbar->addWidget(pageEdit);

   toolbar->addAction(nextPageAction);
   toolbar->addAction(lastPageAction);
   toolbar->addSeparator();
   toolbar->addAction(singleModeAction);
   toolbar->addAction(facingModeAction);
   toolbar->addAction(overviewModeAction);
   toolbar->addSeparator();
   toolbar->addAction(pageSetupAction);
   toolbar->addAction(printAction);

   // Cannot use the actions' triggered signal here, since it doesn't autorepeat
   QToolButton *zoomInButton = static_cast<QToolButton *>(toolbar->widgetForAction(zoomInAction));
   QToolButton *zoomOutButton = static_cast<QToolButton *>(toolbar->widgetForAction(zoomOutAction));
   zoomInButton->setAutoRepeat(true);
   zoomInButton->setAutoRepeatInterval(200);
   zoomInButton->setAutoRepeatDelay(200);
   zoomOutButton->setAutoRepeat(true);
   zoomOutButton->setAutoRepeatInterval(200);
   zoomOutButton->setAutoRepeatDelay(200);

   QObject::connect(zoomInButton,  &QToolButton::clicked, q, &QPrintPreviewDialog::_q_zoomIn);
   QObject::connect(zoomOutButton, &QToolButton::clicked, q, &QPrintPreviewDialog::_q_zoomOut);

   mw->addToolBar(toolbar);
   mw->setCentralWidget(preview);

   // QMainWindows are always created as top levels, force it to be a plain widget
   mw->setParent(q, Qt::Widget);

   QVBoxLayout *topLayout = new QVBoxLayout;
   topLayout->addWidget(mw);
   topLayout->setMargin(0);
   q->setLayout(topLayout);

   QString caption = QCoreApplication::translate("QPrintPreviewDialog", "Print Preview");

   if (!printer->docName().isEmpty()) {
      caption += QString::fromLatin1(": ") + printer->docName();
   }
   q->setWindowTitle(caption);

   if (! printer->isValid()

#if defined(Q_OS_WIN) || defined(Q_OS_DARWIN)
      || printer->outputFormat() != QPrinter::NativeFormat
#endif

   ) {
      pageSetupAction->setEnabled(false);
   }

   preview->setFocus();
}

static inline void qt_setupActionIcon(QAction *action, const QString &name)
{
   static const QString imagePrefix(":/copperspice/printing/images/");

   QIcon icon;
   icon.addFile(imagePrefix + name + "-24.png", QSize(24, 24));
   icon.addFile(imagePrefix + name + "-32.png", QSize(32, 32));
   action->setIcon(icon);
}

void QPrintPreviewDialogPrivate::setupActions()
{
   Q_Q(QPrintPreviewDialog);

   // Navigation
   navGroup = new QActionGroup(q);
   navGroup->setExclusive(false);
   nextPageAction  = navGroup->addAction(QCoreApplication::translate("QPrintPreviewDialog", "Next page"));
   prevPageAction  = navGroup->addAction(QCoreApplication::translate("QPrintPreviewDialog", "Previous page"));
   firstPageAction = navGroup->addAction(QCoreApplication::translate("QPrintPreviewDialog", "First page"));
   lastPageAction  = navGroup->addAction(QCoreApplication::translate("QPrintPreviewDialog", "Last page"));

   qt_setupActionIcon(nextPageAction,  "go-next");
   qt_setupActionIcon(prevPageAction,  "go-previous");
   qt_setupActionIcon(firstPageAction, "go-first");
   qt_setupActionIcon(lastPageAction,  "go-last");

   QObject::connect(navGroup, &QActionGroup::triggered, q, &QPrintPreviewDialog::_q_navigate);

   fitGroup = new QActionGroup(q);
   fitWidthAction = fitGroup->addAction(QCoreApplication::translate("QPrintPreviewDialog", "Fit width"));
   fitPageAction  = fitGroup->addAction(QCoreApplication::translate("QPrintPreviewDialog", "Fit page"));

   fitWidthAction->setObjectName("fitWidthAction");
   fitPageAction->setObjectName("fitPageAction");
   fitWidthAction->setCheckable(true);
   fitPageAction->setCheckable(true);

   qt_setupActionIcon(fitWidthAction, "fit-width");
   qt_setupActionIcon(fitPageAction,  "fit-page");

   QObject::connect(fitGroup, &QActionGroup::triggered, q, &QPrintPreviewDialog::_q_fit);

   // Zoom
   zoomGroup = new QActionGroup(q);
   zoomInAction = zoomGroup->addAction(QCoreApplication::translate("QPrintPreviewDialog", "Zoom in"));
   zoomOutAction = zoomGroup->addAction(QCoreApplication::translate("QPrintPreviewDialog", "Zoom out"));
   qt_setupActionIcon(zoomInAction,  QLatin1String("zoom-in"));
   qt_setupActionIcon(zoomOutAction, QLatin1String("zoom-out"));

   // Portrait/Landscape
   orientationGroup = new QActionGroup(q);
   portraitAction = orientationGroup->addAction(QCoreApplication::translate("QPrintPreviewDialog", "Portrait"));
   landscapeAction = orientationGroup->addAction(QCoreApplication::translate("QPrintPreviewDialog", "Landscape"));
   portraitAction->setCheckable(true);
   landscapeAction->setCheckable(true);
   qt_setupActionIcon(portraitAction, QLatin1String("layout-portrait"));
   qt_setupActionIcon(landscapeAction, QLatin1String("layout-landscape"));

   QObject::connect(portraitAction,  &QAction::triggered, preview, &QPrintPreviewWidget::setPortraitOrientation);
   QObject::connect(landscapeAction, &QAction::triggered, preview, &QPrintPreviewWidget::setLandscapeOrientation);

   // Display mode
   modeGroup = new QActionGroup(q);
   singleModeAction = modeGroup->addAction(QCoreApplication::translate("QPrintPreviewDialog",   "Show single page"));
   facingModeAction = modeGroup->addAction(QCoreApplication::translate("QPrintPreviewDialog",   "Show facing pages"));
   overviewModeAction = modeGroup->addAction(QCoreApplication::translate("QPrintPreviewDialog", "Show overview of all pages"));

   qt_setupActionIcon(singleModeAction, QLatin1String("view-page-one"));
   qt_setupActionIcon(facingModeAction, QLatin1String("view-page-sided"));
   qt_setupActionIcon(overviewModeAction, QLatin1String("view-page-multi"));
   singleModeAction->setObjectName(QLatin1String("singleModeAction"));
   facingModeAction->setObjectName(QLatin1String("facingModeAction"));
   overviewModeAction->setObjectName(QLatin1String("overviewModeAction"));

   singleModeAction->setCheckable(true);
   facingModeAction->setCheckable(true);
   overviewModeAction->setCheckable(true);

   QObject::connect(modeGroup, &QActionGroup::triggered, q, &QPrintPreviewDialog::_q_setMode);

   // Print
   printerGroup = new QActionGroup(q);
   printAction = printerGroup->addAction(QCoreApplication::translate("QPrintPreviewDialog", "Print"));
   pageSetupAction = printerGroup->addAction(QCoreApplication::translate("QPrintPreviewDialog", "Page setup"));
   qt_setupActionIcon(printAction, QLatin1String("print"));
   qt_setupActionIcon(pageSetupAction, QLatin1String("page-setup"));

   QObject::connect(printAction,     &QAction::triggered, q, &QPrintPreviewDialog::_q_print);
   QObject::connect(pageSetupAction, &QAction::triggered, q, &QPrintPreviewDialog::_q_pageSetup);

   // Initial state:
   fitPageAction->setChecked(true);
   singleModeAction->setChecked(true);

   if (preview->orientation() == QPageLayout::Orientation::Portrait) {
      portraitAction->setChecked(true);
   } else {
      landscapeAction->setChecked(true);
   }
}

bool QPrintPreviewDialogPrivate::isFitting()
{
   return (fitGroup->isExclusive()
         && (fitWidthAction->isChecked() || fitPageAction->isChecked()));
}

void QPrintPreviewDialogPrivate::setFitting(bool on)
{
   if (isFitting() == on) {
      return;
   }
   fitGroup->setExclusive(on);

   if (on) {
      QAction *action = fitWidthAction->isChecked() ? fitWidthAction : fitPageAction;
      action->setChecked(true);
      if (fitGroup->checkedAction() != action) {
         // work around exclusitivity problem
         fitGroup->removeAction(action);
         fitGroup->addAction(action);
      }
   } else {
      fitWidthAction->setChecked(false);
      fitPageAction->setChecked(false);
   }
}

void QPrintPreviewDialogPrivate::updateNavActions()
{
   int curPage = preview->currentPage();
   int numPages = preview->pageCount();

   nextPageAction->setEnabled(curPage < numPages);
   prevPageAction->setEnabled(curPage > 1);
   firstPageAction->setEnabled(curPage > 1);
   lastPageAction->setEnabled(curPage < numPages);
   pageNumEdit->setText(QString::number(curPage));
}

void QPrintPreviewDialogPrivate::updatePageNumLabel()
{
   Q_Q(QPrintPreviewDialog);

   int numPages = preview->pageCount();
   int maxChars = QString::number(numPages).length();

   pageNumLabel->setText(QString("/ %1").formatArg(numPages));

   int cyphersWidth = q->fontMetrics().width(QString().fill('8', maxChars));
   int maxWidth = pageNumEdit->minimumSizeHint().width() + cyphersWidth;

   pageNumEdit->setMinimumWidth(maxWidth);
   pageNumEdit->setMaximumWidth(maxWidth);
   pageNumEdit->setValidator(new QIntValidator(1, numPages, pageNumEdit));
   // any old one will be deleted later along with its parent pageNumEdit
}

void QPrintPreviewDialogPrivate::updateZoomFactor()
{
   zoomFactor->lineEdit()->setText(QString("%1%%").formatArg(preview->zoomFactor() * 100, 0, 'f', 1));
}

void QPrintPreviewDialogPrivate::_q_fit(QAction *action)
{
   setFitting(true);

   if (action == fitPageAction) {
      preview->fitInView();
   } else {
      preview->fitToWidth();
   }
}

void QPrintPreviewDialogPrivate::_q_zoomIn()
{
   setFitting(false);
   preview->zoomIn();
   updateZoomFactor();
}

void QPrintPreviewDialogPrivate::_q_zoomOut()
{
   setFitting(false);
   preview->zoomOut();
   updateZoomFactor();
}

void QPrintPreviewDialogPrivate::_q_pageNumEdited()
{
   bool ok = false;
   int res = pageNumEdit->text().toInteger<int>(&ok);

   if (ok) {
      preview->setCurrentPage(res);
   }
}

void QPrintPreviewDialogPrivate::_q_navigate(QAction *action)
{
   int curPage = preview->currentPage();

   if (action == prevPageAction) {
      preview->setCurrentPage(curPage - 1);
   } else if (action == nextPageAction) {
      preview->setCurrentPage(curPage + 1);
   } else if (action == firstPageAction) {
      preview->setCurrentPage(1);
   } else if (action == lastPageAction) {
      preview->setCurrentPage(preview->pageCount());
   }
   updateNavActions();
}

void QPrintPreviewDialogPrivate::_q_setMode(QAction *action)
{
   if (action == overviewModeAction) {
      preview->setViewMode(QPrintPreviewWidget::AllPagesView);
      setFitting(false);
      fitGroup->setEnabled(false);
      navGroup->setEnabled(false);
      pageNumEdit->setEnabled(false);
      pageNumLabel->setEnabled(false);
   } else if (action == facingModeAction) {
      preview->setViewMode(QPrintPreviewWidget::FacingPagesView);
   } else {
      preview->setViewMode(QPrintPreviewWidget::SinglePageView);
   }
   if (action == facingModeAction || action == singleModeAction) {
      fitGroup->setEnabled(true);
      navGroup->setEnabled(true);
      pageNumEdit->setEnabled(true);
      pageNumLabel->setEnabled(true);
      setFitting(true);
   }
}

void QPrintPreviewDialogPrivate::_q_print()
{
   Q_Q(QPrintPreviewDialog);

#if defined(Q_OS_WIN) || defined(Q_OS_DARWIN)
   if (printer->outputFormat() != QPrinter::NativeFormat) {

      QString title = QCoreApplication::translate("QPrintPreviewDialog", "Export to PDF");
      QString suffix = QLatin1String(".pdf");

      QString fileName = QFileDialog::getSaveFileName(q, title, printer->outputFileName(),
            QLatin1Char('*') + suffix);
      if (!fileName.isEmpty()) {
         if (QFileInfo(fileName).suffix().isEmpty()) {
            fileName.append(suffix);
         }
         printer->setOutputFileName(fileName);
      }
      if (!printer->outputFileName().isEmpty()) {
         preview->print();
      }
      q->accept();
      return;
   }
#endif

   if (!printDialog) {
      printDialog = new QPrintDialog(printer, q);
   }
   if (printDialog->exec() == QDialog::Accepted) {
      preview->print();
      q->accept();
   }
}

void QPrintPreviewDialogPrivate::_q_pageSetup()
{
   Q_Q(QPrintPreviewDialog);

   QPageSetupDialog pageSetup(printer, q);

   if (pageSetup.exec() == QDialog::Accepted) {
      // update possible orientation changes
      if (preview->orientation() == QPageLayout::Orientation::Portrait) {
         portraitAction->setChecked(true);
         preview->setPortraitOrientation();

      } else {
         landscapeAction->setChecked(true);
         preview->setLandscapeOrientation();
      }
   }
}

void QPrintPreviewDialogPrivate::_q_previewChanged()
{
   updateNavActions();
   updatePageNumLabel();
   updateZoomFactor();
}

void QPrintPreviewDialogPrivate::_q_zoomFactorChanged()
{
   QString text = zoomFactor->lineEdit()->text();
   bool ok;
   qreal factor = text.remove(QLatin1Char('%')).toFloat(&ok);
   factor = qMax(qreal(1.0), qMin(qreal(1000.0), factor));
   if (ok) {
      preview->setZoomFactor(factor / 100.0);
      zoomFactor->setEditText(QString::fromLatin1("%1%").formatArg(factor));
      setFitting(false);
   }
}

QPrintPreviewDialog::QPrintPreviewDialog(QPrinter *printer, QWidget *parent, Qt::WindowFlags flags)
   : QDialog(*new QPrintPreviewDialogPrivate, parent, flags)
{
   Q_D(QPrintPreviewDialog);
   d->init(printer);
}

QPrintPreviewDialog::QPrintPreviewDialog(QWidget *parent, Qt::WindowFlags flags)
   : QDialog(*new QPrintPreviewDialogPrivate, parent, flags)
{
   Q_D(QPrintPreviewDialog);
   d->init();
}

QPrintPreviewDialog::~QPrintPreviewDialog()
{
   Q_D(QPrintPreviewDialog);
   if (d->ownPrinter) {
      delete d->printer;
   }
   delete d->printDialog;
}

void QPrintPreviewDialog::setVisible(bool visible)
{
   Q_D(QPrintPreviewDialog);
   // this will make the dialog get a decent default size
   if (visible && !d->initialized) {
      d->preview->updatePreview();
      d->initialized = true;
   }
   QDialog::setVisible(visible);
}

void QPrintPreviewDialog::done(int result)
{
   Q_D(QPrintPreviewDialog);
   QDialog::done(result);

   if (d->receiverToDisconnectOnClose) {
      disconnect(this, SIGNAL(finished(int)), d->receiverToDisconnectOnClose, d->memberToDisconnectOnClose);
      d->receiverToDisconnectOnClose = nullptr;
   }

   d->memberToDisconnectOnClose.clear();
}

void QPrintPreviewDialog::open(QObject *receiver, const QString &member)
{
   Q_D(QPrintPreviewDialog);

   // the int parameter isn't very useful here; we could just as well connect
   // to reject(), but this feels less robust somehow

   connect(this, SIGNAL(finished(int)), receiver, member);
   d->receiverToDisconnectOnClose = receiver;
   d->memberToDisconnectOnClose = member;
   QDialog::open();
}

QPrinter *QPrintPreviewDialog::printer()
{
   Q_D(QPrintPreviewDialog);
   return d->printer;
}

void QPrintPreviewDialog::_q_fit(QAction *action)
{
   Q_D(QPrintPreviewDialog);
   d->_q_fit(action);
}

void QPrintPreviewDialog::_q_zoomIn()
{
   Q_D(QPrintPreviewDialog);
   d->_q_zoomIn();
}

void QPrintPreviewDialog::_q_zoomOut()
{
   Q_D(QPrintPreviewDialog);
   d->_q_zoomOut();
}

void QPrintPreviewDialog::_q_navigate(QAction *action)
{
   Q_D(QPrintPreviewDialog);
   d->_q_navigate(action);
}

void QPrintPreviewDialog::_q_setMode(QAction *action)
{
   Q_D(QPrintPreviewDialog);
   d->_q_setMode(action);
}

void QPrintPreviewDialog::_q_pageNumEdited()
{
   Q_D(QPrintPreviewDialog);
   d->_q_pageNumEdited();
}

void QPrintPreviewDialog::_q_print()
{
   Q_D(QPrintPreviewDialog);
   d->_q_print();
}

void QPrintPreviewDialog::_q_pageSetup()
{
   Q_D(QPrintPreviewDialog);
   d->_q_pageSetup();
}

void QPrintPreviewDialog::_q_previewChanged()
{
   Q_D(QPrintPreviewDialog);
   d->_q_previewChanged();
}

void QPrintPreviewDialog::_q_zoomFactorChanged()
{
   Q_D(QPrintPreviewDialog);
   d->_q_zoomFactorChanged();
}

#endif // QT_NO_PRINTPREVIEWDIALOG
