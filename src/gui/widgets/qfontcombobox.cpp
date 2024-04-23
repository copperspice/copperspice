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

#include <qfontcombobox.h>

#ifndef QT_NO_FONTCOMBOBOX

#include <qstringlistmodel.h>
#include <qitemdelegate.h>
#include <qlistview.h>
#include <qpainter.h>
#include <qevent.h>
#include <qapplication.h>
#include <qcombobox_p.h>
#include <qdesktopwidget.h>
#include <qdebug.h>

static QFontDatabase::WritingSystem writingSystemFromScript(QLocale::Script script)
{
   switch (script) {
      case QLocale::ArabicScript:
         return QFontDatabase::Arabic;

      case QLocale::CyrillicScript:
         return QFontDatabase::Cyrillic;

      case QLocale::GurmukhiScript:
         return QFontDatabase::Gurmukhi;

      case QLocale::SimplifiedHanScript:
         return QFontDatabase::SimplifiedChinese;

      case QLocale::TraditionalHanScript:
         return QFontDatabase::TraditionalChinese;

      case QLocale::LatinScript:
         return QFontDatabase::Latin;

      case QLocale::ArmenianScript:
         return QFontDatabase::Armenian;

      case QLocale::BengaliScript:
         return QFontDatabase::Bengali;

      case QLocale::DevanagariScript:
         return QFontDatabase::Devanagari;

      case QLocale::GeorgianScript:
         return QFontDatabase::Georgian;

      case QLocale::GreekScript:
         return QFontDatabase::Greek;

      case QLocale::GujaratiScript:
         return QFontDatabase::Gujarati;

      case QLocale::HebrewScript:
         return QFontDatabase::Hebrew;

      case QLocale::JapaneseScript:
         return QFontDatabase::Japanese;

      case QLocale::KhmerScript:
         return QFontDatabase::Khmer;

      case QLocale::KannadaScript:
         return QFontDatabase::Kannada;

      case QLocale::KoreanScript:
         return QFontDatabase::Korean;

      case QLocale::LaoScript:
         return QFontDatabase::Lao;

      case QLocale::MalayalamScript:
         return QFontDatabase::Malayalam;

      case QLocale::MyanmarScript:
         return QFontDatabase::Myanmar;

      case QLocale::TamilScript:
         return QFontDatabase::Tamil;

      case QLocale::TeluguScript:
         return QFontDatabase::Telugu;

      case QLocale::ThaanaScript:
         return QFontDatabase::Thaana;

      case QLocale::ThaiScript:
         return QFontDatabase::Thai;

      case QLocale::TibetanScript:
         return QFontDatabase::Tibetan;

      case QLocale::SinhalaScript:
         return QFontDatabase::Sinhala;

      case QLocale::SyriacScript:
         return QFontDatabase::Syriac;

      case QLocale::OriyaScript:
         return QFontDatabase::Oriya;

      case QLocale::OghamScript:
         return QFontDatabase::Ogham;

      case QLocale::RunicScript:
         return QFontDatabase::Runic;

      case QLocale::NkoScript:
         return QFontDatabase::Nko;

      default:
         return QFontDatabase::Any;
   }
}

static QFontDatabase::WritingSystem writingSystemFromLocale()
{
   QStringList uiLanguages = QLocale::system().uiLanguages();
   QLocale::Script script;

   if (! uiLanguages.isEmpty()) {
      script = QLocale(uiLanguages.at(0)).script();
   } else {
      script = QLocale::system().script();
   }

   return writingSystemFromScript(script);
}

static QFontDatabase::WritingSystem writingSystemForFont(const QFont &font, bool *hasLatin)
{
   QList<QFontDatabase::WritingSystem> writingSystems = QFontDatabase().writingSystems(font.family());

   // this just confuses the algorithm below. Vietnamese is Latin with lots of special chars
   writingSystems.removeOne(QFontDatabase::Vietnamese);
   *hasLatin = writingSystems.removeOne(QFontDatabase::Latin);

   if (writingSystems.isEmpty()) {
      return QFontDatabase::Any;
   }

   QFontDatabase::WritingSystem system = writingSystemFromLocale();

   if (writingSystems.contains(system)) {
      return system;
   }

   if (system == QFontDatabase::TraditionalChinese
      && writingSystems.contains(QFontDatabase::SimplifiedChinese)) {
      return QFontDatabase::SimplifiedChinese;
   }

   if (system == QFontDatabase::SimplifiedChinese
      && writingSystems.contains(QFontDatabase::TraditionalChinese)) {
      return QFontDatabase::TraditionalChinese;
   }

   system = writingSystems.last();

   if (!*hasLatin) {
      // we need to show something
      return system;
   }

   if (writingSystems.count() == 1 && system > QFontDatabase::Cyrillic) {
      return system;
   }

   if (writingSystems.count() <= 2 && system > QFontDatabase::Armenian && system < QFontDatabase::Vietnamese) {
      return system;
   }

   if (writingSystems.count() <= 5 && system >= QFontDatabase::SimplifiedChinese && system <= QFontDatabase::Korean) {
      return system;
   }

   return QFontDatabase::Any;
}

class QFontFamilyDelegate : public QAbstractItemDelegate
{
   GUI_CS_OBJECT(QFontFamilyDelegate)

 public:
   explicit QFontFamilyDelegate(QObject *parent);

   // painting
   void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
   QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

   QIcon truetype;
   QIcon bitmap;
   QFontDatabase::WritingSystem writingSystem;
};

QFontFamilyDelegate::QFontFamilyDelegate(QObject *parent)
   : QAbstractItemDelegate(parent)
{
   truetype = QIcon(":/copperspice/styles/commonstyle/images/fonttruetype-16.png");
   bitmap   = QIcon(":/copperspice/styles/commonstyle/images/fontbitmap-16.png");
   writingSystem = QFontDatabase::Any;
}

void QFontFamilyDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
   const QModelIndex &index) const
{
   QString text = index.data(Qt::DisplayRole).toString();

   QFont font(option.font);
   font.setPointSize(QFontInfo(font).pointSize() * 3 / 2);

   QFont font2 = font;
   font2.setFamily(text);

   bool hasLatin;
   QFontDatabase::WritingSystem system = writingSystemForFont(font2, &hasLatin);
   if (hasLatin) {
      font = font2;
   }

   QRect r = option.rect;

   if (option.state & QStyle::State_Selected) {
      painter->save();
      painter->setBrush(option.palette.highlight());
      painter->setPen(Qt::NoPen);
      painter->drawRect(option.rect);
      painter->setPen(QPen(option.palette.highlightedText(), 0));
   }

   const QIcon *icon = &bitmap;
   if (QFontDatabase().isSmoothlyScalable(text)) {
      icon = &truetype;
   }
   QSize actualSize = icon->actualSize(r.size());

   icon->paint(painter, r, Qt::AlignLeft | Qt::AlignVCenter);
   if (option.direction == Qt::RightToLeft) {
      r.setRight(r.right() - actualSize.width() - 4);
   } else {
      r.setLeft(r.left() + actualSize.width() + 4);
   }

   QFont old = painter->font();
   painter->setFont(font);

   // If the ascent of the font is larger than the height of the rect,
   // we will clip the text, so it's better to align the tight bounding rect in this case
   // This is specifically for fonts where the ascent is very large compared to
   // the descent, like certain of the Stix family.
   QFontMetricsF fontMetrics(font);

   if (fontMetrics.ascent() > r.height()) {
      QRectF tbr = fontMetrics.tightBoundingRect(text);
      painter->drawText(r.x(), r.y() + (r.height() + tbr.height()) / 2.0, text);
   } else {
      painter->drawText(r, Qt::AlignVCenter | Qt::AlignLeading | Qt::TextSingleLine, text);
   }

   if (writingSystem != QFontDatabase::Any) {
      system = writingSystem;
   }

   if (system != QFontDatabase::Any) {
      int w = painter->fontMetrics().width(text + QLatin1String("  "));
      painter->setFont(font2);
      QString sample = QFontDatabase().writingSystemSample(system);
      if (option.direction == Qt::RightToLeft) {
         r.setRight(r.right() - w);
      } else {
         r.setLeft(r.left() + w);
      }
      painter->drawText(r, Qt::AlignVCenter | Qt::AlignLeading | Qt::TextSingleLine, sample);
   }
   painter->setFont(old);

   if (option.state & QStyle::State_Selected) {
      painter->restore();
   }
}

QSize QFontFamilyDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
   QString text = index.data(Qt::DisplayRole).toString();

   QFont font(option.font);
   // font.setFamily(text);

   font.setPointSize(QFontInfo(font).pointSize() * 3 / 2);

   QFontMetrics fontMetrics(font);

   return QSize(fontMetrics.width(text), fontMetrics.height());
}

class QFontComboBoxPrivate : public QComboBoxPrivate
{
 public:
   inline QFontComboBoxPrivate() {
      filters = QFontComboBox::AllFonts;
   }

   QFontComboBox::FontFilters filters;
   QFont currentFont;

   void _q_updateModel();
   void _q_currentChanged(const QString &);

   Q_DECLARE_PUBLIC(QFontComboBox)
};

void QFontComboBoxPrivate::_q_updateModel()
{
   Q_Q(QFontComboBox);

   const int scalableMask = (QFontComboBox::ScalableFonts | QFontComboBox::NonScalableFonts);
   const int spacingMask = (QFontComboBox::ProportionalFonts | QFontComboBox::MonospacedFonts);

   QStringListModel *m = qobject_cast<QStringListModel *>(q->model());
   if (! m) {
      return;
   }

   QFontFamilyDelegate *delegate = qobject_cast<QFontFamilyDelegate *>(q->view()->itemDelegate());
   QFontDatabase::WritingSystem system = delegate ? delegate->writingSystem : QFontDatabase::Any;

   QFontDatabase fdb;
   QStringList list = fdb.families(system);
   QStringList result;

   int offset = 0;
   QFontInfo fi(currentFont);

   for (int i = 0; i < list.size(); ++i) {
      if (fdb.isPrivateFamily(list.at(i))) {
         continue;
      }

      if ((filters & scalableMask) && (filters & scalableMask) != scalableMask) {
         if (bool(filters & QFontComboBox::ScalableFonts) != fdb.isSmoothlyScalable(list.at(i))) {
            continue;
         }
      }

      if ((filters & spacingMask) && (filters & spacingMask) != spacingMask) {
         if (bool(filters & QFontComboBox::MonospacedFonts) != fdb.isFixedPitch(list.at(i))) {
            continue;
         }
      }

      result += list.at(i);

      if (list.at(i) == fi.family() || list.at(i).startsWith(fi.family() + QLatin1String(" ["))) {
         offset = result.count() - 1;
      }
   }

   list = result;

   // need to block the signals so that the model doesn't emit reset
   // this prevents the current index from changing
   // it will be updated just after this
   // TODO: we should finda way to avoid blocking signals and have a real update of the model

   const bool old = m->blockSignals(true);
   m->setStringList(list);
   m->blockSignals(old);

   if (list.isEmpty()) {
      if (currentFont != QFont()) {
         currentFont = QFont();
         emit q->currentFontChanged(currentFont);
      }

   } else {
      q->setCurrentIndex(offset);
   }
}

void QFontComboBoxPrivate::_q_currentChanged(const QString &text)
{
   Q_Q(QFontComboBox);
   if (currentFont.family() != text) {
      currentFont.setFamily(text);
      emit q->currentFontChanged(currentFont);
   }
}

QFontComboBox::QFontComboBox(QWidget *parent)
   : QComboBox(*new QFontComboBoxPrivate, parent)
{
   Q_D(QFontComboBox);

   d->currentFont = font();
   setEditable(true);

   QStringListModel *m = new QStringListModel(this);
   setModel(m);
   setItemDelegate(new QFontFamilyDelegate(this));

   QListView *lview = dynamic_cast<QListView *>(view());

   if (lview) {
      lview->setUniformItemSizes(true);
   }
   setWritingSystem(QFontDatabase::Any);

   // broom - should be QFontComboBox ( not a major issue )
   connect(this, cs_mp_cast<const QString &>(&QComboBox::currentIndexChanged), this, &QFontComboBox::_q_currentChanged);

   connect(qApp, &QApplication::fontDatabaseChanged,  this, &QFontComboBox::_q_updateModel);
}

QFontComboBox::~QFontComboBox()
{
}

void QFontComboBox::setWritingSystem(QFontDatabase::WritingSystem script)
{
   Q_D(QFontComboBox);

   QFontFamilyDelegate *delegate = qobject_cast<QFontFamilyDelegate *>(view()->itemDelegate());

   if (delegate) {
      delegate->writingSystem = script;
   }

   d->_q_updateModel();
}

QFontDatabase::WritingSystem QFontComboBox::writingSystem() const
{
   QFontFamilyDelegate *delegate = qobject_cast<QFontFamilyDelegate *>(view()->itemDelegate());

   if (delegate) {
      return delegate->writingSystem;
   }

   return QFontDatabase::Any;
}

void QFontComboBox::setFontFilters(FontFilters filters)
{
   Q_D(QFontComboBox);
   d->filters = filters;
   d->_q_updateModel();
}

QFontComboBox::FontFilters QFontComboBox::fontFilters() const
{
   Q_D(const QFontComboBox);
   return d->filters;
}

QFont QFontComboBox::currentFont() const
{
   Q_D(const QFontComboBox);
   return d->currentFont;
}

void QFontComboBox::setCurrentFont(const QFont &font)
{
   Q_D(QFontComboBox);

   if (font != d->currentFont) {
      d->currentFont = font;
      d->_q_updateModel();
      if (d->currentFont == font) { //else the signal has already be emitted by _q_updateModel
         emit currentFontChanged(d->currentFont);
      }
   }
}

bool QFontComboBox::event(QEvent *e)
{
   if (e->type() == QEvent::Resize) {
      QListView *lview = qobject_cast<QListView *>(view());
      if (lview) {
         lview->window()->setFixedWidth(qMin(width() * 5 / 3,
               QApplication::desktop()->availableGeometry(lview).width()));
      }
   }

   return QComboBox::event(e);
}

QSize QFontComboBox::sizeHint() const
{
   QSize sz = QComboBox::sizeHint();
   QFontMetrics fm(font());
   sz.setWidth(fm.width(QLatin1Char('m')) * 14);
   return sz;
}

void QFontComboBox::_q_currentChanged(const QString &text)
{
   Q_D(QFontComboBox);
   d->_q_currentChanged(text);
}

void QFontComboBox::_q_updateModel()
{
   Q_D(QFontComboBox);
   d->_q_updateModel();
}

#endif // QT_NO_FONTCOMBOBOX
