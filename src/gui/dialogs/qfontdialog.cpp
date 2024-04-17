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

#include <qwindowdefs.h>

#ifndef QT_NO_FONTDIALOG

#include <qfontdialog.h>

#include <qapplication.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qdialogbuttonbox.h>
#include <qevent.h>
#include <qfontdatabase.h>
#include <qgroupbox.h>
#include <qheaderview.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qlistview.h>
#include <qpushbutton.h>
#include <qstyle.h>
#include <qstringlistmodel.h>
#include <qvalidator.h>

#include <qdialog_p.h>
#include <qfont_p.h>
#include <qfontdialog_p.h>

class QFontListView : public QListView
{
   GUI_CS_OBJECT(QFontListView)

 public:
   QFontListView(QWidget *parent);

   QStringListModel *model() const {
      return static_cast<QStringListModel *>(QListView::model());
   }

   void setCurrentItem(int item) {
      QListView::setCurrentIndex(static_cast<QAbstractListModel *>(model())->index(item));
   }

   int currentItem() const {
      return QListView::currentIndex().row();
   }

   int count() const {
      return model()->rowCount();
   }

   QString currentText() const {
      int row = QListView::currentIndex().row();
      return row < 0 ? QString() : model()->stringList().at(row);
   }

   void currentChanged(const QModelIndex &current, const QModelIndex &previous)  override {
      QListView::currentChanged(current, previous);

      if (current.isValid()) {
         emit highlighted(current.row());
      }
   }

   QString text(int i) const {
      return model()->stringList().at(i);
   }

   GUI_CS_SIGNAL_1(Public, void highlighted(int data))
   GUI_CS_SIGNAL_2(highlighted, data)
};

QFontListView::QFontListView(QWidget *parent)
   : QListView(parent)
{
   setModel(new QStringListModel(parent));
   setEditTriggers(NoEditTriggers);
}

static constexpr const Qt::WindowFlags DefaultWindowFlags =
   Qt::Dialog | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint;

QFontDialogPrivate::QFontDialogPrivate()
   : writingSystem(QFontDatabase::Any), options(QSharedPointer<QFontDialogOptions>::create())
{
}

QFontDialogPrivate::~QFontDialogPrivate()
{
}

QFontDialog::QFontDialog(QWidget *parent)
   : QDialog(*new QFontDialogPrivate, parent, DefaultWindowFlags)
{
   Q_D(QFontDialog);
   d->init();
}

QFontDialog::QFontDialog(const QFont &initial, QWidget *parent)
   : QDialog(*new QFontDialogPrivate, parent, DefaultWindowFlags)
{
   Q_D(QFontDialog);
   d->init();
   setCurrentFont(initial);
}

void QFontDialogPrivate::init()
{
   Q_Q(QFontDialog);

   q->setSizeGripEnabled(true);
   q->setWindowTitle(QFontDialog::tr("Select Font"));

   // grid
   familyEdit = new QLineEdit(q);
   familyEdit->setReadOnly(true);
   familyList = new QFontListView(q);
   familyEdit->setFocusProxy(familyList);

   familyAccel = new QLabel(q);
#ifndef QT_NO_SHORTCUT
   familyAccel->setBuddy(familyList);
#endif
   familyAccel->setIndent(2);

   styleEdit = new QLineEdit(q);
   styleEdit->setReadOnly(true);
   styleList = new QFontListView(q);
   styleEdit->setFocusProxy(styleList);

   styleAccel = new QLabel(q);
#ifndef QT_NO_SHORTCUT
   styleAccel->setBuddy(styleList);
#endif
   styleAccel->setIndent(2);

   sizeEdit = new QLineEdit(q);
   sizeEdit->setFocusPolicy(Qt::ClickFocus);
   QIntValidator *validator = new QIntValidator(1, 512, q);
   sizeEdit->setValidator(validator);
   sizeList = new QFontListView(q);

   sizeAccel = new QLabel(q);
#ifndef QT_NO_SHORTCUT
   sizeAccel->setBuddy(sizeEdit);
#endif
   sizeAccel->setIndent(2);

   // effects box
   effects = new QGroupBox(q);
   QVBoxLayout *vbox = new QVBoxLayout(effects);

   strikeout = new QCheckBox(effects);
   vbox->addWidget(strikeout);

   underline = new QCheckBox(effects);
   vbox->addWidget(underline);

   sample = new QGroupBox(q);

   QHBoxLayout *hbox = new QHBoxLayout(sample);

   sampleEdit = new QLineEdit(sample);
   sampleEdit->setSizePolicy(QSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored));
   sampleEdit->setAlignment(Qt::AlignCenter);

   // sample text is *not* translated with tr(), since the
   // characters depend on the charset encoding

   sampleEdit->setText("AaBbYyZz");
   hbox->addWidget(sampleEdit);

   writingSystemCombo = new QComboBox(q);

   writingSystemAccel = new QLabel(q);

#ifndef QT_NO_SHORTCUT
   writingSystemAccel->setBuddy(writingSystemCombo);
#endif

   writingSystemAccel->setIndent(2);

   size = 0;
   smoothScalable = false;

   QObject::connect(writingSystemCombo, cs_mp_cast<int>(&QComboBox::activated), q,
      &QFontDialog::_q_writingSystemHighlighted);

   QObject::connect(familyList,         &QFontListView::highlighted, q, &QFontDialog::_q_familyHighlighted);
   QObject::connect(styleList,          &QFontListView::highlighted, q, &QFontDialog::_q_styleHighlighted);
   QObject::connect(sizeList,           &QFontListView::highlighted, q, &QFontDialog::_q_sizeHighlighted);

   QObject::connect(sizeEdit,           &QLineEdit::textChanged,     q, &QFontDialog::_q_sizeChanged);
   QObject::connect(strikeout,          &QCheckBox::clicked,         q, &QFontDialog::_q_updateSample);
   QObject::connect(underline,          &QCheckBox::clicked,         q, &QFontDialog::_q_updateSample);

   for (int i = 0; i < QFontDatabase::WritingSystemsCount; ++i) {
      QFontDatabase::WritingSystem ws = QFontDatabase::WritingSystem(i);
      QString writingSystemName = QFontDatabase::writingSystemName(ws);

      if (writingSystemName.isEmpty()) {
         break;
      }

      writingSystemCombo->addItem(writingSystemName);
   }

   updateFamilies();
   if (familyList->count() != 0) {
      familyList->setCurrentItem(0);
      sizeList->setCurrentItem(0);
   }

   // grid layout
   QGridLayout *mainGrid = new QGridLayout(q);

   int spacing = mainGrid->spacing();

   if (spacing >= 0) {
      // uniform spacing
      mainGrid->setSpacing(0);

      mainGrid->setColumnMinimumWidth(1, spacing);
      mainGrid->setColumnMinimumWidth(3, spacing);

      int margin = 0;
      mainGrid->getContentsMargins(nullptr, nullptr, nullptr, &margin);

      mainGrid->setRowMinimumHeight(3, margin);
      mainGrid->setRowMinimumHeight(6, 2);
      mainGrid->setRowMinimumHeight(8, margin);
   }

   mainGrid->addWidget(familyAccel, 0, 0);
   mainGrid->addWidget(familyEdit, 1, 0);
   mainGrid->addWidget(familyList, 2, 0);

   mainGrid->addWidget(styleAccel, 0, 2);
   mainGrid->addWidget(styleEdit, 1, 2);
   mainGrid->addWidget(styleList, 2, 2);

   mainGrid->addWidget(sizeAccel, 0, 4);
   mainGrid->addWidget(sizeEdit, 1, 4);
   mainGrid->addWidget(sizeList, 2, 4);

   mainGrid->setColumnStretch(0, 38);
   mainGrid->setColumnStretch(2, 24);
   mainGrid->setColumnStretch(4, 10);

   mainGrid->addWidget(effects, 4, 0);

   mainGrid->addWidget(sample, 4, 2, 4, 3);

   mainGrid->addWidget(writingSystemAccel, 5, 0);
   mainGrid->addWidget(writingSystemCombo, 7, 0);

   buttonBox = new QDialogButtonBox(q);
   mainGrid->addWidget(buttonBox, 9, 0, 1, 5);

   QPushButton *button = static_cast<QPushButton *>(buttonBox->addButton(QDialogButtonBox::Ok));
   button->setDefault(true);

   buttonBox->addButton(QDialogButtonBox::Cancel);

   QObject::connect(buttonBox, &QDialogButtonBox::accepted, q, &QFontDialog::accept);
   QObject::connect(buttonBox, &QDialogButtonBox::rejected, q, &QFontDialog::reject);

   q->resize(500, 360);

   sizeEdit->installEventFilter(q);
   familyList->installEventFilter(q);
   styleList->installEventFilter(q);
   sizeList->installEventFilter(q);

   familyList->setFocus();
   retranslateStrings();

   sampleEdit->setObjectName("qt_fontDialog_sampleEdit");
}

// internal
QFontDialog::~QFontDialog()
{
}

QFont QFontDialog::getFont(bool *ok, const QFont &initial, QWidget *parent, const QString &title,
   FontDialogOptions options)
{
   return QFontDialogPrivate::getFont(ok, initial, parent, title, options);
}

QFont QFontDialog::getFont(bool *ok, QWidget *parent)
{
   QFont initial;
   return QFontDialogPrivate::getFont(ok, initial, parent, QString(), Qt::EmptyFlag);
}

QFont QFontDialogPrivate::getFont(bool *ok, const QFont &initial, QWidget *parent,
   const QString &title, QFontDialog::FontDialogOptions options)
{
   QFontDialog dlg(parent);
   dlg.setOptions(options);
   dlg.setCurrentFont(initial);

   if (! title.isEmpty()) {
      dlg.setWindowTitle(title);
   }

   int ret = (dlg.exec() || (options & QFontDialog::NoButtons));
   if (ok) {
      *ok = !!ret;
   }

   if (ret) {
      return dlg.selectedFont();
   } else {
      return initial;
   }
}

//  event filter to make the Up, Down, PageUp and PageDown keys work correctly in the line edits.
//  the source of the event is the object o and the event is e.
bool QFontDialog::eventFilter(QObject *o, QEvent *e)
{
   Q_D(QFontDialog);

   if (e->type() == QEvent::KeyPress) {
      QKeyEvent *k = (QKeyEvent *)e;

      if (o == d->sizeEdit &&
         (k->key() == Qt::Key_Up ||
            k->key() == Qt::Key_Down ||
            k->key() == Qt::Key_PageUp ||
            k->key() == Qt::Key_PageDown)) {

         int ci = d->sizeList->currentItem();
         (void)QApplication::sendEvent(d->sizeList, k);

         if (ci != d->sizeList->currentItem()
            && style()->styleHint(QStyle::SH_FontDialog_SelectAssociatedText, nullptr, this)) {
            d->sizeEdit->selectAll();
         }

         return true;

      } else if ((o == d->familyList || o == d->styleList) &&
         (k->key() == Qt::Key_Return || k->key() == Qt::Key_Enter)) {
         k->accept();
         accept();

         return true;
      }

   } else if (e->type() == QEvent::FocusIn
         && style()->styleHint(QStyle::SH_FontDialog_SelectAssociatedText, nullptr, this)) {

      if (o == d->familyList) {
         d->familyEdit->selectAll();
      } else if (o == d->styleList) {
         d->styleEdit->selectAll();
      } else if (o == d->sizeList) {
         d->sizeEdit->selectAll();
      }

   } else if (e->type() == QEvent::MouseButtonPress && o == d->sizeList) {
      d->sizeEdit->setFocus();
   }

   return QDialog::eventFilter(o, e);
}

void QFontDialogPrivate::initHelper(QPlatformDialogHelper *h)
{
   QFontDialog *d = q_func();

   QPlatformFontDialogHelper *tmp = static_cast<QPlatformFontDialogHelper *>(h);

   QObject::connect(tmp, &QPlatformFontDialogHelper::currentFontChanged, d, &QFontDialog::currentFontChanged);
   QObject::connect(tmp, &QPlatformFontDialogHelper::fontSelected,       d, &QFontDialog::fontSelected);

   tmp->setOptions(options);
}

void QFontDialogPrivate::helperPrepareShow(QPlatformDialogHelper *)
{
   options->setWindowTitle(q_func()->windowTitle());
}

void QFontDialogPrivate::updateFamilies()
{
   Q_Q(QFontDialog);

   enum match_t {
      MATCH_NONE        = 0,
      MATCH_LAST_RESORT = 1,
      MATCH_APP         = 2,
      MATCH_FAMILY      = 3
   };

   const QFontDialog::FontDialogOptions scalableMask = (QFontDialog::ScalableFonts | QFontDialog::NonScalableFonts);
   const QFontDialog::FontDialogOptions spacingMask = (QFontDialog::ProportionalFonts | QFontDialog::MonospacedFonts);
   const QFontDialog::FontDialogOptions options = q->options();

   QFontDatabase fdb;
   QStringList familyNames;

   for (const QString &family : fdb.families(writingSystem)) {
      if (fdb.isPrivateFamily(family)) {
         continue;
      }

      if ((options & scalableMask) && (options & scalableMask) != scalableMask) {
         if (bool(options & QFontDialog::ScalableFonts) != fdb.isSmoothlyScalable(family)) {
            continue;
         }
      }

      if ((options & spacingMask) && (options & spacingMask) != spacingMask) {
         if (bool(options & QFontDialog::MonospacedFonts) != fdb.isFixedPitch(family)) {
            continue;
         }
      }

      familyNames << family;
   }

   familyList->model()->setStringList(familyNames);

   QString foundryName1, familyName1, foundryName2, familyName2;
   int bestFamilyMatch = -1;
   match_t bestFamilyType = MATCH_NONE;

   QFont f;

   // do the right thing for a list of family names in the font.
   QFontDatabase::parseFontName(family, foundryName1, familyName1);

   QStringList::const_iterator it = familyNames.constBegin();
   int i = 0;
   for (; it != familyNames.constEnd(); ++it, ++i) {
      QFontDatabase::parseFontName(*it, foundryName2, familyName2);

      // try to match
      if (familyName1 == familyName2) {
         bestFamilyType = MATCH_FAMILY;
         if (foundryName1 == foundryName2) {
            bestFamilyMatch = i;
            break;
         }

         if (bestFamilyMatch < MATCH_FAMILY) {
            bestFamilyMatch = i;
         }
      }

      // try some fall backs
      match_t type = MATCH_NONE;
      if (bestFamilyType <= MATCH_NONE && familyName2 == f.lastResortFamily()) {
         type = MATCH_LAST_RESORT;
      }

      if (bestFamilyType <= MATCH_LAST_RESORT && familyName2 == f.family()) {
         type = MATCH_APP;
      }

      // add fallback for writingSystem
      if (type != MATCH_NONE) {
         bestFamilyType = type;
         bestFamilyMatch = i;
      }
   }

   if (i != -1 && bestFamilyType != MATCH_NONE) {
      familyList->setCurrentItem(bestFamilyMatch);
   } else {
      familyList->setCurrentItem(0);
   }

   familyEdit->setText(familyList->currentText());

   if (q->style()->styleHint(QStyle::SH_FontDialog_SelectAssociatedText, nullptr, q)
         && familyList->hasFocus()) {
      familyEdit->selectAll();
   }

   updateStyles();
}

void QFontDialogPrivate::updateStyles()
{
   Q_Q(QFontDialog);

   QStringList styles = fdb.styles(familyList->currentText());
   styleList->model()->setStringList(styles);

   if (styles.isEmpty()) {
      styleEdit->clear();
      smoothScalable = false;

   } else {
      if (!style.isEmpty()) {
         bool found = false;
         bool first = true;
         QString cstyle = style;

      redo:
         for (int i = 0; i < (int)styleList->count(); i++) {
            if (cstyle == styleList->text(i)) {
               styleList->setCurrentItem(i);
               found = true;
               break;
            }
         }

         if (! found && first) {
            if (cstyle.contains("Italic")) {
               cstyle.replace("Italic", "Oblique");
               first = false;
               goto redo;

            } else if (cstyle.contains("Oblique")) {
               cstyle.replace("Oblique", "Italic");
               first = false;
               goto redo;
            }
         }

         if (! found) {
            styleList->setCurrentItem(0);
         }

      } else {
         styleList->setCurrentItem(0);
      }

      styleEdit->setText(styleList->currentText());
      if (q->style()->styleHint(QStyle::SH_FontDialog_SelectAssociatedText, nullptr, q)
         && styleList->hasFocus()) {
         styleEdit->selectAll();
      }

      smoothScalable = fdb.isSmoothlyScalable(familyList->currentText(), styleList->currentText());
   }

   updateSizes();
}

void QFontDialogPrivate::updateSizes()
{
   Q_Q(QFontDialog);

   if (! familyList->currentText().isEmpty()) {
      QList<int> sizes = fdb.pointSizes(familyList->currentText(), styleList->currentText());

      int i       = 0;
      int current = -1;
      QStringList str_sizes;

      for (auto item : sizes) {
         str_sizes.append(QString::number(item));

         if (current == -1 && item >= size) {
            current = i;
         }

         ++i;
      }

      sizeList->model()->setStringList(str_sizes);

      if (current != -1) {
         sizeList->setCurrentItem(current);
      }

      sizeEdit->blockSignals(true);
      sizeEdit->setText((smoothScalable ? QString::number(size) : sizeList->currentText()));

      if (q->style()->styleHint(QStyle::SH_FontDialog_SelectAssociatedText, nullptr, q) && sizeList->hasFocus()) {
         sizeEdit->selectAll();
      }

      sizeEdit->blockSignals(false);

   } else {
      sizeEdit->clear();
   }

   _q_updateSample();
}

void QFontDialogPrivate::_q_updateSample()
{
   // compute new font
   int pSize = sizeEdit->text().toInteger<int>();
   QFont newFont(fdb.font(familyList->currentText(), style, pSize));
   newFont.setStrikeOut(strikeout->isChecked());
   newFont.setUnderline(underline->isChecked());

   if (familyList->currentText().isEmpty()) {
      sampleEdit->clear();
   }

   updateSampleFont(newFont);
}

void QFontDialogPrivate::updateSampleFont(const QFont &newFont)
{
   Q_Q(QFontDialog);

   if (newFont != sampleEdit->font()) {
      sampleEdit->setFont(newFont);
      emit q->currentFontChanged(newFont);
   }
}

void QFontDialogPrivate::_q_writingSystemHighlighted(int index)
{
   writingSystem = QFontDatabase::WritingSystem(index);
   sampleEdit->setText(fdb.writingSystemSample(writingSystem));
   updateFamilies();
}

void QFontDialogPrivate::_q_familyHighlighted(int i)
{
   Q_Q(QFontDialog);

   family = familyList->text(i);
   familyEdit->setText(family);

   if (q->style()->styleHint(QStyle::SH_FontDialog_SelectAssociatedText, nullptr, q)
      && familyList->hasFocus()) {
      familyEdit->selectAll();
   }

   updateStyles();
}

void QFontDialogPrivate::_q_styleHighlighted(int index)
{
   Q_Q(QFontDialog);

   QString s = styleList->text(index);
   styleEdit->setText(s);

   if (q->style()->styleHint(QStyle::SH_FontDialog_SelectAssociatedText, nullptr, q)
         && styleList->hasFocus()) {
      styleEdit->selectAll();
   }

   style = s;

   updateSizes();
}

void QFontDialogPrivate::_q_sizeHighlighted(int index)
{
   Q_Q(QFontDialog);

   QString s = sizeList->text(index);
   sizeEdit->setText(s);

   if (q->style()->styleHint(QStyle::SH_FontDialog_SelectAssociatedText, nullptr, q)
         && sizeEdit->hasFocus()) {
      sizeEdit->selectAll();
   }

   size = s.toInteger<int>();
   _q_updateSample();
}

void QFontDialogPrivate::_q_sizeChanged(const QString &s)
{
   // no need to check if the conversion is valid, since we have an QIntValidator in the size edit
   int size = s.toInteger<int>();
   if (this->size == size) {
      return;
   }

   this->size = size;
   if (sizeList->count() != 0) {
      int i;
      for (i = 0; i < sizeList->count() - 1; i++) {
         if (sizeList->text(i).toInteger<int>() >= this->size) {
            break;
         }
      }
      sizeList->blockSignals(true);

      if (sizeList->text(i).toInteger<int>() == this->size) {
         sizeList->setCurrentItem(i);

      } else {
         sizeList->clearSelection();

      }

      sizeList->blockSignals(false);
   }

   _q_updateSample();
}

void QFontDialogPrivate::retranslateStrings()
{
   familyAccel->setText(QFontDialog::tr("&Font"));
   styleAccel->setText(QFontDialog::tr("Font st&yle"));
   sizeAccel->setText(QFontDialog::tr("&Size"));
   effects->setTitle(QFontDialog::tr("Effects"));
   strikeout->setText(QFontDialog::tr("Stri&keout"));
   underline->setText(QFontDialog::tr("&Underline"));
   sample->setTitle(QFontDialog::tr("Sample"));
   writingSystemAccel->setText(QFontDialog::tr("Wr&iting System"));
}

void QFontDialog::changeEvent(QEvent *e)
{
   Q_D(QFontDialog);

   if (e->type() == QEvent::LanguageChange) {
      d->retranslateStrings();
   }

   QDialog::changeEvent(e);
}

void QFontDialog::setCurrentFont(const QFont &font)
{
   Q_D(QFontDialog);

   d->family = font.family();
   d->style  = d->fdb.styleString(font);
   d->size   = font.pointSize();

   if (d->size == -1) {
      QFontInfo fi(font);
      d->size = fi.pointSize();
   }

   d->strikeout->setChecked(font.strikeOut());
   d->underline->setChecked(font.underline());
   d->updateFamilies();

   if (d->canBeNativeDialog()) {
      if (QPlatformFontDialogHelper *helper = d->platformFontDialogHelper()) {
         helper->setCurrentFont(font);
      }
   }
}

QFont QFontDialog::currentFont() const
{
   Q_D(const QFontDialog);

   if (d->canBeNativeDialog()) {
      if (const QPlatformFontDialogHelper *helper = d->platformFontDialogHelper()) {
         return helper->currentFont();
      }
   }

   return d->sampleEdit->font();
}

QFont QFontDialog::selectedFont() const
{
   Q_D(const QFontDialog);
   return d->selectedFont;
}

void QFontDialog::setOption(FontDialogOption option, bool on)
{
   const QFontDialog::FontDialogOptions previousOptions = options();

   if (! (previousOptions & option) != !on) {
      setOptions(previousOptions ^ option);
   }
}

bool QFontDialog::testOption(FontDialogOption option) const
{
   Q_D(const QFontDialog);
   return d->options->testOption(static_cast<QFontDialogOptions::FontDialogOption>(option));
}

void QFontDialog::setOptions(FontDialogOptions options)
{
   Q_D(QFontDialog);

   if (QFontDialog::options() == options) {
      return;
   }

   d->options->setOptions(QFontDialogOptions::FontDialogOptions(int(options)));
   d->buttonBox->setVisible(!(options & NoButtons));
}

QFontDialog::FontDialogOptions QFontDialog::options() const
{
   Q_D(const QFontDialog);
   return QFontDialog::FontDialogOptions(int(d->options->options()));
}

void QFontDialog::open(QObject *receiver, const QString &member)
{
   Q_D(QFontDialog);

   connect(this, SIGNAL(fontSelected(QFont)), receiver, member);
   d->receiverToDisconnectOnClose = receiver;
   d->memberToDisconnectOnClose = member;

   QDialog::open();
}

void QFontDialog::setVisible(bool visible)
{
   if (testAttribute(Qt::WA_WState_ExplicitShowHide) && testAttribute(Qt::WA_WState_Hidden) != visible) {
      return;
   }

   Q_D(QFontDialog);

   if (d->canBeNativeDialog()) {
      d->setNativeDialogVisible(visible);
   }

   if (d->nativeDialogInUse) {
      // Set WA_DontShowOnScreen so that QDialog::setVisible(visible) below
      // updates the state correctly, but skips showing the non-native version:
      setAttribute(Qt::WA_DontShowOnScreen, true);
   } else {
      d->nativeDialogInUse = false;
      setAttribute(Qt::WA_DontShowOnScreen, false);
   }

   QDialog::setVisible(visible);
}

void QFontDialog::done(int result)
{
   Q_D(QFontDialog);

   QDialog::done(result);

   if (result == Accepted) {
      // check if this is the same font we had before, if so we emit currentFontChanged
      QFont selectedFont = currentFont();

      if (selectedFont != d->selectedFont) {
         emit(currentFontChanged(selectedFont));
      }

      d->selectedFont = selectedFont;
      emit fontSelected(d->selectedFont);

   } else {
      d->selectedFont = QFont();
   }

   if (d->receiverToDisconnectOnClose) {
      disconnect(this, SIGNAL(fontSelected(QFont)), d->receiverToDisconnectOnClose, d->memberToDisconnectOnClose);

      d->receiverToDisconnectOnClose = nullptr;
   }

   d->memberToDisconnectOnClose.clear();
}

bool QFontDialogPrivate::canBeNativeDialog() const
{
   // do not use Q_Q since this method is called from ~QDialog which
   // can result in undefined behavior (invalid cast in q_func()

   const QDialog *const q = static_cast<const QDialog *>(q_ptr);

   if (nativeDialogInUse) {
      return true;
   }

   if (q->testAttribute(Qt::WA_DontShowOnScreen)) {
      return false;
   }

   if (options->options() & QFontDialog::DontUseNativeDialog) {
      return false;
   }

   QString staticName(QFontDialog::staticMetaObject().className());
   QString dynamicName(q->metaObject()->className());

   return (staticName == dynamicName);
}

void QFontDialog::_q_sizeChanged(const QString &data)
{
   Q_D(QFontDialog);
   d->_q_sizeChanged(data);
}

void QFontDialog::_q_familyHighlighted(int data)
{
   Q_D(QFontDialog);
   d->_q_familyHighlighted(data);
}

void QFontDialog::_q_writingSystemHighlighted(int data)
{
   Q_D(QFontDialog);
   d->_q_writingSystemHighlighted(data);
}

void QFontDialog::_q_styleHighlighted(int data)
{
   Q_D(QFontDialog);
   d->_q_styleHighlighted(data);
}

void QFontDialog::_q_sizeHighlighted(int data)
{
   Q_D(QFontDialog);
   d->_q_sizeHighlighted(data);
}

void QFontDialog::_q_updateSample()
{
   Q_D(QFontDialog);
   d->_q_updateSample();
}

#endif // QT_NO_FONTDIALOG
