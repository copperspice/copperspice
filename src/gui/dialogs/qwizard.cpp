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

#include <qwizard.h>

#ifndef QT_NO_WIZARD

#include <qabstractspinbox.h>
#include <qalgorithms.h>
#include <qapplication.h>
#include <qdebug.h>
#include <qboxlayout.h>
#include <qlayoutitem.h>
#include <qdesktopwidget.h>
#include <qevent.h>
#include <qframe.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qpainter.h>
#include <qpushbutton.h>
#include <qset.h>
#include <qstyle.h>
#include <qvarlengtharray.h>
#include <qwindow.h>

#if defined(Q_OS_DARWIN)
#include <qplatform_nativeinterface.h>

#elif ! defined(QT_NO_STYLE_WINDOWSVISTA)
#include <qwizard_win_p.h>
#include <qtimer.h>

#endif

#include <qdialog_p.h>

#include <string.h>
#include <algorithm>

// These terms were needed a few places to obtain pixel-perfect results
const int GapBetweenLogoAndRightEdge = 5;
const int ModernHeaderTopMargin      = 2;
const int ClassicHMargin             = 4;
const int MacButtonTopMargin         = 13;
const int MacLayoutLeftMargin        = 20;

//const int MacLayoutTopMargin  = 14;       // Unused, Save some space and avoid warning.
const int MacLayoutRightMargin  = 20;
const int MacLayoutBottomMargin = 17;

static void changeSpacerSize(QLayout *layout, int index, int width, int height)
{
   QSpacerItem *spacer = layout->itemAt(index)->spacerItem();

   if (! spacer) {
      return;
   }

   spacer->changeSize(width, height);
}

static QWidget *iWantTheFocus(QWidget *ancestor)
{
   const int MaxIterations = 100;

   QWidget *candidate = ancestor;

   for (int i = 0; i < MaxIterations; ++i) {
      candidate = candidate->nextInFocusChain();
      if (! candidate) {
         break;
      }

      if (candidate->focusPolicy() & Qt::TabFocus) {
         if (candidate != ancestor && ancestor->isAncestorOf(candidate)) {
            return candidate;
         }
      }
   }

   return nullptr;
}

static bool objectInheritsXAndXIsCloserThanY(const QObject *object, const QString &classX, const QString &classY)
{
   const QMetaObject *metaObject = object->metaObject();

   while (metaObject) {
      if (metaObject->className() == classX) {
         return true;
      }

      if (metaObject->className() == classY) {
         return false;
      }

      metaObject = metaObject->superClass();
   }

   return false;
}

const size_t NFallbackDefaultProperties = 7;

struct FallbackData {

   FallbackData(const char *var1, const char *var2)
      : className(QString::fromLatin1(var1)), property(QString::fromLatin1(var2))
   { }

   const QString className;
   const QString property;
};

const FallbackData fallbackProperties[NFallbackDefaultProperties] = {
   { "QAbstractButton", "checked"       },
   { "QAbstractSlider", "value"         },
   { "QComboBox",       "currentIndex"  },
   { "QDateTimeEdit",   "dateTime"      },
   { "QLineEdit",       "text"          },
   { "QListWidget",     "currentRow"    },
   { "QSpinBox",        "value"         }
};

static QString changed_signal(int which)
{
   QString retval;

   switch (which) {
      case 0:
         retval = "toggled(bool)";
         break;

      case 1:
         retval = "valueChanged(int)";
         break;

      case 2:
         retval = "currentIndexChanged(int)";
         break;

      case 3:
         retval = "dateTimeChanged(QDateTime)";
         break;

      case 4:
         retval = "textChanged(QString)";
         break;

      case 5:
         retval = "currentRowChanged(int)";
         break;

      case 6:
         retval = "valueChanged(int)";
         break;
   };

   static_assert(7 == NFallbackDefaultProperties, "Incorrect values");

   return retval;
}

class QWizardDefaultProperty
{
 public:
   QWizardDefaultProperty()
   { }

   QWizardDefaultProperty(const QString &className, const QString &property, const QString &changedSignal)
      : m_className(className), m_property(property), m_changedSignal(changedSignal)
   { }

   QString m_className;
   QString m_property;
   QString m_changedSignal;
};

class QWizardField
{
 public:
   QWizardField()
   { }

   QWizardField(QWizardPage *page, const QString &spec, QObject *object, const QString &property, const QString &changedSignal);

   void resolve(const QVector<QWizardDefaultProperty> &defaultPropertyTable);
   void findProperty(const QWizardDefaultProperty *properties, int propertyCount);

   QWizardPage *page;
   QObject *object;

   bool mandatory;

   QString  name;
   QString  property;
   QString  changedSignal;
   QVariant initialValue;
};

QWizardField::QWizardField(QWizardPage *page, const QString &spec, QObject *object,
      const QString &property, const QString &changedSignal)
   : page(page), object(object), mandatory(false), name(spec), property(property), changedSignal(changedSignal)
{
   if (name.endsWith('*')) {
      name.chop(1);
      mandatory = true;
   }
}

void QWizardField::resolve(const QVector<QWizardDefaultProperty> &defaultPropertyTable)
{
   if (property.isEmpty()) {
      findProperty(defaultPropertyTable.constData(), defaultPropertyTable.count());
   }

   initialValue = object->property(property);
}

void QWizardField::findProperty(const QWizardDefaultProperty *properties, int propertyCount)
{
   QString className;

   for (int i = 0; i < propertyCount; ++i) {
      if (objectInheritsXAndXIsCloserThanY(object, properties[i].m_className, className)) {
         className     = properties[i].m_className;
         property      = properties[i].m_property;
         changedSignal = properties[i].m_changedSignal;
      }
   }
}

class QWizardLayoutInfo
{
 public:
   QWizardLayoutInfo()
      : topLevelMarginLeft(-1), topLevelMarginRight(-1), topLevelMarginTop(-1),
        topLevelMarginBottom(-1), childMarginLeft(-1), childMarginRight(-1),
        childMarginTop(-1), childMarginBottom(-1), hspacing(-1), vspacing(-1),
        wizStyle(QWizard::ClassicStyle), header(false), watermark(false), title(false),
        subTitle(false), extension(false), sideWidget(false)
   { }

   int topLevelMarginLeft;
   int topLevelMarginRight;
   int topLevelMarginTop;
   int topLevelMarginBottom;
   int childMarginLeft;
   int childMarginRight;
   int childMarginTop;
   int childMarginBottom;
   int hspacing;
   int vspacing;
   int buttonSpacing;
   QWizard::WizardStyle wizStyle;
   bool header;
   bool watermark;
   bool title;
   bool subTitle;
   bool extension;
   bool sideWidget;

   bool operator==(const QWizardLayoutInfo &other);

   inline bool operator!=(const QWizardLayoutInfo &other) {
      return !operator==(other);
   }
};

bool QWizardLayoutInfo::operator==(const QWizardLayoutInfo &other)
{
   return topLevelMarginLeft == other.topLevelMarginLeft
      && topLevelMarginRight == other.topLevelMarginRight
      && topLevelMarginTop == other.topLevelMarginTop
      && topLevelMarginBottom == other.topLevelMarginBottom
      && childMarginLeft == other.childMarginLeft
      && childMarginRight == other.childMarginRight
      && childMarginTop == other.childMarginTop
      && childMarginBottom == other.childMarginBottom
      && hspacing == other.hspacing
      && vspacing == other.vspacing
      && buttonSpacing == other.buttonSpacing
      && wizStyle == other.wizStyle
      && header == other.header
      && watermark == other.watermark
      && title == other.title
      && subTitle == other.subTitle
      && extension == other.extension
      && sideWidget == other.sideWidget;
}

class QWizardHeader : public QWidget
{
 public:
   enum RulerType {
      Ruler
   };

   inline QWizardHeader(RulerType ruler, QWidget *parent = nullptr)
      : QWidget(parent) {

      (void) ruler;
      setFixedHeight(2);
   }

   QWizardHeader(QWidget *parent = nullptr);

   void setup(const QWizardLayoutInfo &info, const QString &title,
      const QString &subTitle, const QPixmap &logo, const QPixmap &banner,
      Qt::TextFormat titleFormat, Qt::TextFormat subTitleFormat);

 protected:
   void paintEvent(QPaintEvent *event) override;

 private:

#if ! defined(QT_NO_STYLE_WINDOWSVISTA)
   bool vistaDisabled() const;
#endif

   QLabel *titleLabel;
   QLabel *subTitleLabel;
   QLabel *logoLabel;
   QGridLayout *layout;
   QPixmap bannerPixmap;
};

QWizardHeader::QWizardHeader(QWidget *parent)
   : QWidget(parent)
{
   setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
   setBackgroundRole(QPalette::Base);

   titleLabel = new QLabel(this);
   titleLabel->setBackgroundRole(QPalette::Base);

   subTitleLabel = new QLabel(this);
   subTitleLabel->setAlignment(Qt::AlignTop | Qt::AlignLeft);
   subTitleLabel->setWordWrap(true);

   logoLabel = new QLabel(this);

   QFont font = titleLabel->font();
   font.setBold(true);
   titleLabel->setFont(font);

   layout = new QGridLayout(this);
   layout->setMargin(0);
   layout->setSpacing(0);

   layout->setRowMinimumHeight(3, 1);
   layout->setRowStretch(4, 1);

   layout->setColumnStretch(2, 1);
   layout->setColumnMinimumWidth(4, 2 * GapBetweenLogoAndRightEdge);
   layout->setColumnMinimumWidth(6, GapBetweenLogoAndRightEdge);

   layout->addWidget(titleLabel, 2, 1, 1, 2);
   layout->addWidget(subTitleLabel, 4, 2);
   layout->addWidget(logoLabel, 1, 5, 5, 1);
}

#if ! defined(QT_NO_STYLE_WINDOWSVISTA)
bool QWizardHeader::vistaDisabled() const
{
   bool styleDisabled = false;
   QWizard *wiz = parentWidget() ? qobject_cast <QWizard *>(parentWidget()->parentWidget()) : nullptr;
   if (wiz) {
      // Designer dosen't support the Vista style for Wizards. This property is used to turn
      // off the Vista style.
      const QVariant v = wiz->property("_q_wizard_vista_off");
      styleDisabled = v.isValid() && v.toBool();
   }
   return styleDisabled;
}
#endif

void QWizardHeader::setup(const QWizardLayoutInfo &info, const QString &title,
   const QString &subTitle, const QPixmap &logo, const QPixmap &banner,
   Qt::TextFormat titleFormat, Qt::TextFormat subTitleFormat)
{
   bool modern = ((info.wizStyle == QWizard::ModernStyle)

#if ! defined(QT_NO_STYLE_WINDOWSVISTA)
         || ((info.wizStyle == QWizard::AeroStyle && QVistaHelper::vistaState() == QVistaHelper::Classic) || vistaDisabled())
#endif

      );

   layout->setRowMinimumHeight(0, modern ? ModernHeaderTopMargin : 0);
   layout->setRowMinimumHeight(1, modern ? info.topLevelMarginTop - ModernHeaderTopMargin - 1 : 0);
   layout->setRowMinimumHeight(6, (modern ? 3 : GapBetweenLogoAndRightEdge) + 2);

   int minColumnWidth0 = modern ? info.topLevelMarginLeft + info.topLevelMarginRight : 0;
   int minColumnWidth1 = modern ? info.topLevelMarginLeft + info.topLevelMarginRight + 1
      : info.topLevelMarginLeft + ClassicHMargin;

   layout->setColumnMinimumWidth(0, minColumnWidth0);
   layout->setColumnMinimumWidth(1, minColumnWidth1);

   titleLabel->setTextFormat(titleFormat);
   titleLabel->setText(title);
   logoLabel->setPixmap(logo);

   subTitleLabel->setTextFormat(subTitleFormat);
   subTitleLabel->setText("Pq\nPq");

   int desiredSubTitleHeight = subTitleLabel->sizeHint().height();
   subTitleLabel->setText(subTitle);

   if (modern) {
      bannerPixmap = banner;
   } else {
      bannerPixmap = QPixmap();
   }

   if (bannerPixmap.isNull()) {
      /*
          There is no widthForHeight() function, so we simulate it with a loop.
      */
      int candidateSubTitleWidth = qMin(512, 2 * QApplication::desktop()->width() / 3);
      int delta = candidateSubTitleWidth >> 1;
      while (delta > 0) {
         if (subTitleLabel->heightForWidth(candidateSubTitleWidth - delta) <= desiredSubTitleHeight) {
            candidateSubTitleWidth -= delta;
         }
         delta >>= 1;
      }

      subTitleLabel->setMinimumSize(candidateSubTitleWidth, desiredSubTitleHeight);

      QSize size = layout->totalMinimumSize();
      setMinimumSize(size);
      setMaximumSize(QWIDGETSIZE_MAX, size.height());

   } else {
      subTitleLabel->setMinimumSize(0, 0);
      setFixedSize(banner.size() + QSize(0, 2));
   }
   updateGeometry();
}

void QWizardHeader::paintEvent(QPaintEvent *)
{
   QPainter painter(this);
   painter.drawPixmap(0, 0, bannerPixmap);

   int x = width() - 2;
   int y = height() - 2;
   const QPalette &pal = palette();
   painter.setPen(pal.mid().color());
   painter.drawLine(0, y, x, y);
   painter.setPen(pal.base().color());
   painter.drawPoint(x + 1, y);
   painter.drawLine(0, y + 1, x + 1, y + 1);
}

// save one vtable by basing QWizardRuler on QWizardHeader
class QWizardRuler : public QWizardHeader
{
 public:
   inline QWizardRuler(QWidget *parent = nullptr)
      : QWizardHeader(Ruler, parent) {}
};

class QWatermarkLabel : public QLabel
{
 public:
   QWatermarkLabel(QWidget *parent, QWidget *sideWidget) : QLabel(parent), m_sideWidget(sideWidget) {
      m_layout = new QVBoxLayout(this);
      if (m_sideWidget) {
         m_layout->addWidget(m_sideWidget);
      }
   }

   QSize minimumSizeHint() const  override {
      if (!pixmap() && !pixmap()->isNull()) {
         return pixmap()->size();
      }
      return QFrame::minimumSizeHint();
   }

   void setSideWidget(QWidget *widget) {
      if (m_sideWidget == widget) {
         return;
      }
      if (m_sideWidget) {
         m_layout->removeWidget(m_sideWidget);
         m_sideWidget->hide();
      }
      m_sideWidget = widget;
      if (m_sideWidget) {
         m_layout->addWidget(m_sideWidget);
      }
   }

   QWidget *sideWidget() const {
      return m_sideWidget;
   }

 private:
   QVBoxLayout *m_layout;
   QWidget *m_sideWidget;
};

class QWizardPagePrivate : public QWidgetPrivate
{
   Q_DECLARE_PUBLIC(QWizardPage)

 public:
   enum TriState {
      Tri_Unknown = -1,
      Tri_False,
      Tri_True
   };

   QWizardPagePrivate()
      : wizard(nullptr), completeState(Tri_Unknown), explicitlyFinal(false), commit(false)
   { }

   bool cachedIsComplete() const;
   void _q_changedSignal();
   void _q_updateCachedCompleteState();

   QWizard *wizard;
   QString title;
   QString subTitle;
   QPixmap pixmaps[QWizard::NPixmaps];
   QVector<QWizardField> pendingFields;
   mutable TriState completeState;
   bool explicitlyFinal;
   bool commit;
   QMap<int, QString> buttonCustomTexts;
};

bool QWizardPagePrivate::cachedIsComplete() const
{
   Q_Q(const QWizardPage);
   if (completeState == Tri_Unknown) {
      completeState = q->isComplete() ? Tri_True : Tri_False;
   }
   return completeState == Tri_True;
}

void QWizardPagePrivate::_q_changedSignal()
{
   Q_Q(QWizardPage);

   TriState newState = q->isComplete() ? Tri_True : Tri_False;

   if (newState != completeState) {
      emit q->completeChanged();
   }
}

void QWizardPagePrivate::_q_updateCachedCompleteState()
{
   Q_Q(QWizardPage);
   completeState = q->isComplete() ? Tri_True : Tri_False;
}

class QWizardAntiFlickerWidget : public QWidget
{

#if ! defined(QT_NO_STYLE_WINDOWSVISTA)
 public:
   QWizardAntiFlickerWidget(QWizard *wizard, QWizardPrivate *wizardPrivate)
      : QWidget(wizard), wizardPrivate(wizardPrivate)
   { }

   QWizardPrivate *wizardPrivate;

 protected:
   void paintEvent(QPaintEvent *) override;

#else
 public:
   QWizardAntiFlickerWidget(QWizard *wizard, QWizardPrivate *)
      : QWidget(wizard)
   { }

#endif

};

class QWizardPrivate : public QDialogPrivate
{
   Q_DECLARE_PUBLIC(QWizard)

 public:
   using PageMap = QMap<int, QWizardPage *>;

   enum Direction {
      Backward,
      Forward
   };

   QWizardPrivate()
      : start(-1), startSetByUser(false), current(-1), canContinue(false), canFinish(false),
        disableUpdatesCount(0), wizStyle(QWizard::ClassicStyle), opts(Qt::EmptyFlag),
        buttonsHaveCustomLayout(false), titleFmt(Qt::AutoText), subTitleFmt(Qt::AutoText),
        placeholderWidget1(nullptr), placeholderWidget2(nullptr), headerWidget(nullptr),
        watermarkLabel(nullptr), sideWidget(nullptr), pageFrame(nullptr), titleLabel(nullptr),
        subTitleLabel(nullptr), bottomRuler(nullptr),

#if ! defined(QT_NO_STYLE_WINDOWSVISTA)
        vistaHelper(nullptr), vistaInitPending(false), vistaState(QVistaHelper::VistaState::Dirty),
        vistaStateChanged(false), inHandleAeroStyleChange(false),
#endif
        minimumWidth(0), minimumHeight(0), maximumWidth(QWIDGETSIZE_MAX) , maximumHeight(QWIDGETSIZE_MAX)
   {
      std::fill(btns, btns + QWizard::NButtons, static_cast<QAbstractButton *>(nullptr));

#if ! defined(QT_NO_STYLE_WINDOWSVISTA)
      if (QSysInfo::WindowsVersion >= QSysInfo::WV_VISTA && (QSysInfo::WindowsVersion & QSysInfo::WV_NT_based)) {
         vistaInitPending = true;
      }
#endif
   }

   void init();
   void reset();
   void cleanupPagesNotInHistory();
   void addField(const QWizardField &field);
   void removeFieldAt(int index);
   void switchToPage(int newId, Direction direction);

   QWizardLayoutInfo layoutInfoForCurrentPage();

   void recreateLayout(const QWizardLayoutInfo &info);
   void updateLayout();
   void updateMinMaxSizes(const QWizardLayoutInfo &info);
   void updateCurrentPage();
   bool ensureButton(QWizard::WizardButton which) const;
   void connectButton(QWizard::WizardButton which) const;
   void updateButtonTexts();
   void updateButtonLayout();
   void setButtonLayout(const QWizard::WizardButton *array, int maxSize);
   bool buttonLayoutContains(QWizard::WizardButton which);
   void updatePixmap(QWizard::WizardPixmap which);

#if ! defined(QT_NO_STYLE_WINDOWSVISTA)
   bool vistaDisabled() const;
   bool isVistaThemeEnabled(QVistaHelper::VistaState state) const;
   bool handleAeroStyleChange();
#endif

   bool isVistaThemeEnabled() const;
   void disableUpdates();
   void enableUpdates();
   void _q_emitCustomButtonClicked();
   void _q_updateButtonStates();
   void _q_handleFieldObjectDestroyed(QObject *);
   void setStyle(QStyle *style);

#ifdef Q_OS_DARWIN
   static QPixmap findDefaultBackgroundPixmap();
#endif

   PageMap pageMap;
   QVector<QWizardField> fields;
   QMap<QString, int> fieldIndexMap;
   QVector<QWizardDefaultProperty> defaultPropertyTable;
   QList<int> history;
   QSet<int> initialized; // ### remove and move bit to QWizardPage?

   int start;
   bool startSetByUser;
   int current;
   bool canContinue;
   bool canFinish;
   QWizardLayoutInfo layoutInfo;
   int disableUpdatesCount;

   QWizard::WizardStyle wizStyle;
   QWizard::WizardOptions opts;
   QMap<int, QString> buttonCustomTexts;
   bool buttonsHaveCustomLayout;
   QList<QWizard::WizardButton> buttonsCustomLayout;
   Qt::TextFormat titleFmt;
   Qt::TextFormat subTitleFmt;
   mutable QPixmap defaultPixmaps[QWizard::NPixmaps];

   union {
      // keep in sync with QWizard::WizardButton
      mutable struct {
         QAbstractButton *back;
         QAbstractButton *next;
         QAbstractButton *commit;
         QAbstractButton *finish;
         QAbstractButton *cancel;
         QAbstractButton *help;
      } btn;

      mutable QAbstractButton *btns[QWizard::NButtons];
   };

   QWizardAntiFlickerWidget *antiFlickerWidget;
   QWidget *placeholderWidget1;
   QWidget *placeholderWidget2;
   QWizardHeader *headerWidget;
   QWatermarkLabel *watermarkLabel;
   QWidget *sideWidget;
   QFrame *pageFrame;
   QLabel *titleLabel;
   QLabel *subTitleLabel;

   QWizardRuler *bottomRuler;
   QVBoxLayout *pageVBoxLayout;
   QHBoxLayout *buttonLayout;
   QGridLayout *mainLayout;

#if ! defined(QT_NO_STYLE_WINDOWSVISTA)
   QVistaHelper *vistaHelper;
   bool vistaInitPending;
   QVistaHelper::VistaState vistaState;
   bool vistaStateChanged;
   bool inHandleAeroStyleChange;
#endif

   int minimumWidth;
   int minimumHeight;
   int maximumWidth;
   int maximumHeight;
};

static QString buttonDefaultText(int wstyle, int which, const QWizardPrivate *wizardPrivate)
{
#if defined(QT_NO_STYLE_WINDOWSVISTA)
   (void) wizardPrivate;

#endif
   const bool macStyle = (wstyle == QWizard::MacStyle);

   switch (which) {
      case QWizard::BackButton:
         return macStyle ? QWizard::tr("Go Back") : QWizard::tr("< &Back");

      case QWizard::NextButton:
         if (macStyle) {
            return QWizard::tr("Continue");
         } else {
            return wizardPrivate->isVistaThemeEnabled()
               ? QWizard::tr("&Next") : QWizard::tr("&Next >");
         }

      case QWizard::CommitButton:
         return QWizard::tr("Commit");

      case QWizard::FinishButton:
         return macStyle ? QWizard::tr("Done") : QWizard::tr("&Finish");

      case QWizard::CancelButton:
         return QWizard::tr("Cancel");

      case QWizard::HelpButton:
         return macStyle ? QWizard::tr("Help") : QWizard::tr("&Help");

      default:
         return QString();
   }
}

void QWizardPrivate::init()
{
   Q_Q(QWizard);

   antiFlickerWidget = new QWizardAntiFlickerWidget(q, this);
   wizStyle = QWizard::WizardStyle(q->style()->styleHint(QStyle::SH_WizardStyle, nullptr, q));

   if (wizStyle == QWizard::MacStyle) {
      opts = (QWizard::NoDefaultButton | QWizard::NoCancelButton);
   } else if (wizStyle == QWizard::ModernStyle) {
      opts = QWizard::HelpButtonOnRight;
   }

#if ! defined(QT_NO_STYLE_WINDOWSVISTA)
   vistaHelper = new QVistaHelper(q);
#endif

   // create these buttons right away; create the other buttons as necessary
   ensureButton(QWizard::BackButton);
   ensureButton(QWizard::NextButton);
   ensureButton(QWizard::CommitButton);
   ensureButton(QWizard::FinishButton);

   pageFrame = new QFrame(antiFlickerWidget);
   pageFrame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

   pageVBoxLayout = new QVBoxLayout(pageFrame);
   pageVBoxLayout->setSpacing(0);
   pageVBoxLayout->addSpacing(0);
   QSpacerItem *spacerItem = new QSpacerItem(0, 0, QSizePolicy::Ignored, QSizePolicy::MinimumExpanding);
   pageVBoxLayout->addItem(spacerItem);

   buttonLayout = new QHBoxLayout;
   mainLayout = new QGridLayout(antiFlickerWidget);
   mainLayout->setSizeConstraint(QLayout::SetNoConstraint);

   updateButtonLayout();

   defaultPropertyTable.reserve(NFallbackDefaultProperties);

   for (uint i = 0; i < NFallbackDefaultProperties; ++i) {
      defaultPropertyTable.append(QWizardDefaultProperty(fallbackProperties[i].className,
            fallbackProperties[i].property, changed_signal(i)));
   }
}

void QWizardPrivate::reset()
{
   Q_Q(QWizard);
   if (current != -1) {
      q->currentPage()->hide();
      cleanupPagesNotInHistory();
      for (int i = history.count() - 1; i >= 0; --i) {
         q->cleanupPage(history.at(i));
      }
      history.clear();
      initialized.clear();

      current = -1;
      emit q->currentIdChanged(-1);
   }
}

void QWizardPrivate::cleanupPagesNotInHistory()
{
   Q_Q(QWizard);

   const QSet<int> original = initialized;
   QSet<int>::const_iterator i = original.constBegin();
   QSet<int>::const_iterator end = original.constEnd();

   for (; i != end; ++i) {
      if (!history.contains(*i)) {
         q->cleanupPage(*i);
         initialized.remove(*i);
      }
   }
}

void QWizardPrivate::addField(const QWizardField &field)
{
   Q_Q(QWizard);

   QWizardField myField = field;
   myField.resolve(defaultPropertyTable);

   if (fieldIndexMap.contains(myField.name)) {
      qWarning("QWizardPage::addField() Duplicate field %s", csPrintable(myField.name));
      return;
   }

   fieldIndexMap.insert(myField.name, fields.count());
   fields += myField;

   if (myField.mandatory && ! myField.changedSignal.isEmpty()) {
      QObject::connect(myField.object, myField.changedSignal, myField.page, SLOT(_q_changedSignal()));
   }

   QObject::connect(myField.object, &QObject::destroyed, q, &QWizard::_q_handleFieldObjectDestroyed);
}

void QWizardPrivate::removeFieldAt(int index)
{
   Q_Q(QWizard);

   const QWizardField &field = fields.at(index);
   fieldIndexMap.remove(field.name);

   if (field.mandatory && ! field.changedSignal.isEmpty()) {
      QObject::disconnect(field.object, field.changedSignal, field.page, SLOT(_q_changedSignal()));
   }

   QObject::disconnect(field.object, &QObject::destroyed, q, &QWizard::_q_handleFieldObjectDestroyed);
   fields.remove(index);
}

void QWizardPrivate::switchToPage(int newId, Direction direction)
{
   Q_Q(QWizard);

   disableUpdates();

   int oldId = current;
   if (QWizardPage *oldPage = q->currentPage()) {
      oldPage->hide();

      if (direction == Backward) {
         if (!(opts & QWizard::IndependentPages)) {
            q->cleanupPage(oldId);
            initialized.remove(oldId);
         }
         Q_ASSERT(history.last() == oldId);
         history.removeLast();
         Q_ASSERT(history.last() == newId);
      }
   }

   current = newId;

   QWizardPage *newPage = q->currentPage();
   if (newPage) {
      if (direction == Forward) {
         if (!initialized.contains(current)) {
            initialized.insert(current);
            q->initializePage(current);
         }
         history.append(current);
      }
      newPage->show();
   }

   canContinue = (q->nextId() != -1);
   canFinish = (newPage && newPage->isFinalPage());

   _q_updateButtonStates();
   updateButtonTexts();

   const QWizard::WizardButton nextOrCommit = newPage && newPage->isCommitPage() ? QWizard::CommitButton : QWizard::NextButton;
   QAbstractButton *nextOrFinishButton       = btns[canContinue ? nextOrCommit : QWizard::FinishButton];

   QWidget *candidate = nullptr;

   /*
       If there is no default button and the Next or Finish button
       is enabled, give focus directly to it as a convenience to the
       user. This is the normal case on Mac OS X.

       Otherwise, give the focus to the new page's first child that
       can handle it. If there is no such child, give the focus to
       Next or Finish.
   */

   if ((opts & QWizard::NoDefaultButton) && nextOrFinishButton->isEnabled()) {
      candidate = nextOrFinishButton;

   } else if (newPage) {
      candidate = iWantTheFocus(newPage);
   }

   if (!candidate) {
      candidate = nextOrFinishButton;
   }
   candidate->setFocus();

   if (wizStyle == QWizard::MacStyle) {
      q->updateGeometry();
   }

   enableUpdates();
   updateLayout();

   emit q->currentIdChanged(current);
}

// keep in sync with QWizard::WizardButton
static QString buttonSlots(QWizard::WizardButton which)
{
   switch (which) {
      case QWizard::BackButton:
         return QString("back()");

      case QWizard::NextButton:
      case QWizard::CommitButton:
         return QString("next()");

      case QWizard::FinishButton:
         return QString("accept()");

      case QWizard::CancelButton:
         return QString("reject()");

      case QWizard::HelpButton:
         return QString("helpRequested()");

      case QWizard::CustomButton1:
      case QWizard::CustomButton2:
      case QWizard::CustomButton3:
      case QWizard::Stretch:
      case QWizard::NoButton:
         // error, may want to throw
         break;
   }

   return QString();
}

QWizardLayoutInfo QWizardPrivate::layoutInfoForCurrentPage()
{
   Q_Q(QWizard);
   QStyle *style = q->style();

   QWizardLayoutInfo info;

   const int layoutHorizontalSpacing = style->pixelMetric(QStyle::PM_LayoutHorizontalSpacing);
   info.topLevelMarginLeft = style->pixelMetric(QStyle::PM_LayoutLeftMargin, nullptr, q);
   info.topLevelMarginRight = style->pixelMetric(QStyle::PM_LayoutRightMargin, nullptr, q);
   info.topLevelMarginTop = style->pixelMetric(QStyle::PM_LayoutTopMargin, nullptr, q);
   info.topLevelMarginBottom = style->pixelMetric(QStyle::PM_LayoutBottomMargin, nullptr, q);
   info.childMarginLeft = style->pixelMetric(QStyle::PM_LayoutLeftMargin, nullptr, titleLabel);
   info.childMarginRight = style->pixelMetric(QStyle::PM_LayoutRightMargin, nullptr, titleLabel);
   info.childMarginTop = style->pixelMetric(QStyle::PM_LayoutTopMargin, nullptr, titleLabel);
   info.childMarginBottom = style->pixelMetric(QStyle::PM_LayoutBottomMargin, nullptr, titleLabel);

   info.hspacing = (layoutHorizontalSpacing == -1)
      ? style->layoutSpacing(QSizePolicy::DefaultType, QSizePolicy::DefaultType, Qt::Horizontal) : layoutHorizontalSpacing;

   info.vspacing = style->pixelMetric(QStyle::PM_LayoutVerticalSpacing);

   info.buttonSpacing = (layoutHorizontalSpacing == -1)
      ? style->layoutSpacing(QSizePolicy::PushButton, QSizePolicy::PushButton, Qt::Horizontal) : layoutHorizontalSpacing;

   if (wizStyle == QWizard::MacStyle) {
      info.buttonSpacing = 12;
   }

   info.wizStyle = wizStyle;
#if defined(QT_NO_STYLE_WINDOWSVISTA)
   if (info.wizStyle == QWizard::AeroStyle) {

#else
   if ( (info.wizStyle == QWizard::AeroStyle) && (QVistaHelper::vistaState() == QVistaHelper::Classic ||
         vistaDisabled()) ) {
#endif
      info.wizStyle = QWizard::ModernStyle;
   }

   QString titleText;
   QString subTitleText;
   QPixmap backgroundPixmap;
   QPixmap watermarkPixmap;

   if (QWizardPage *page = q->currentPage()) {
      titleText = page->title();
      subTitleText = page->subTitle();
      backgroundPixmap = page->pixmap(QWizard::BackgroundPixmap);
      watermarkPixmap = page->pixmap(QWizard::WatermarkPixmap);
   }

   info.header = (info.wizStyle == QWizard::ClassicStyle || info.wizStyle == QWizard::ModernStyle)
      && !(opts & QWizard::IgnoreSubTitles) && !subTitleText.isEmpty();
   info.sideWidget = sideWidget;
   info.watermark = (info.wizStyle != QWizard::MacStyle) && (info.wizStyle != QWizard::AeroStyle)
      && !watermarkPixmap.isNull();
   info.title = !info.header && !titleText.isEmpty();
   info.subTitle = !(opts & QWizard::IgnoreSubTitles) && !info.header && !subTitleText.isEmpty();
   info.extension = (info.watermark || info.sideWidget) && (opts & QWizard::ExtendedWatermarkPixmap);

   return info;
}

void QWizardPrivate::recreateLayout(const QWizardLayoutInfo &info)
{
   Q_Q(QWizard);

   /*
       Start by undoing the main layout.
   */
   for (int i = mainLayout->count() - 1; i >= 0; --i) {
      QLayoutItem *item = mainLayout->takeAt(i);
      if (item->layout()) {
         item->layout()->setParent(nullptr);
      } else {
         delete item;
      }
   }
   for (int i = mainLayout->columnCount() - 1; i >= 0; --i) {
      mainLayout->setColumnMinimumWidth(i, 0);
   }
   for (int i = mainLayout->rowCount() - 1; i >= 0; --i) {
      mainLayout->setRowMinimumHeight(i, 0);
   }

   /*
       Now, recreate it.
   */

   bool mac = (info.wizStyle == QWizard::MacStyle);
   bool classic = (info.wizStyle == QWizard::ClassicStyle);
   bool modern = (info.wizStyle == QWizard::ModernStyle);
   bool aero = (info.wizStyle == QWizard::AeroStyle);
   int deltaMarginLeft = info.topLevelMarginLeft - info.childMarginLeft;
   int deltaMarginRight = info.topLevelMarginRight - info.childMarginRight;
   int deltaMarginTop = info.topLevelMarginTop - info.childMarginTop;
   int deltaMarginBottom = info.topLevelMarginBottom - info.childMarginBottom;
   int deltaVSpacing = info.topLevelMarginBottom - info.vspacing;

   int row = 0;
   int numColumns;
   if (mac) {
      numColumns = 3;
   } else if (info.watermark || info.sideWidget) {
      numColumns = 2;
   } else {
      numColumns = 1;
   }
   int pageColumn = qMin(1, numColumns - 1);

   if (mac) {
      mainLayout->setMargin(0);
      mainLayout->setSpacing(0);
      buttonLayout->setContentsMargins(MacLayoutLeftMargin, MacButtonTopMargin, MacLayoutRightMargin, MacLayoutBottomMargin);
      pageVBoxLayout->setMargin(7);
   } else {
      if (modern) {
         mainLayout->setMargin(0);
         mainLayout->setSpacing(0);
         pageVBoxLayout->setContentsMargins(deltaMarginLeft, deltaMarginTop,
            deltaMarginRight, deltaMarginBottom);
         buttonLayout->setContentsMargins(info.topLevelMarginLeft, info.topLevelMarginTop,
            info.topLevelMarginRight, info.topLevelMarginBottom);
      } else {
         mainLayout->setContentsMargins(info.topLevelMarginLeft, info.topLevelMarginTop,
            info.topLevelMarginRight, info.topLevelMarginBottom);
         mainLayout->setHorizontalSpacing(info.hspacing);
         mainLayout->setVerticalSpacing(info.vspacing);
         pageVBoxLayout->setContentsMargins(0, 0, 0, 0);
         buttonLayout->setContentsMargins(0, 0, 0, 0);
      }
   }
   buttonLayout->setSpacing(info.buttonSpacing);

   if (info.header) {
      if (!headerWidget) {
         headerWidget = new QWizardHeader(antiFlickerWidget);
      }
      headerWidget->setAutoFillBackground(modern);
      mainLayout->addWidget(headerWidget, row++, 0, 1, numColumns);
   }
   if (headerWidget) {
      headerWidget->setVisible(info.header);
   }

   int watermarkStartRow = row;

   if (mac) {
      mainLayout->setRowMinimumHeight(row++, 10);
   }

   if (info.title) {
      if (!titleLabel) {
         titleLabel = new QLabel(antiFlickerWidget);
         titleLabel->setBackgroundRole(QPalette::Base);
         titleLabel->setWordWrap(true);
      }

      QFont titleFont = q->font();
      titleFont.setPointSize(titleFont.pointSize() + (mac ? 3 : 4));
      titleFont.setBold(true);
      titleLabel->setPalette(QPalette());

      if (aero) {
         // ### hardcoded for now:
         titleFont = QFont("Segoe UI", 12);
         QPalette pal(titleLabel->palette());
         pal.setColor(QPalette::Text, "#003399");
         titleLabel->setPalette(pal);
      }

      titleLabel->setFont(titleFont);
      const int aeroTitleIndent = 25; // ### hardcoded for now - should be calculated somehow
      if (aero) {
         titleLabel->setIndent(aeroTitleIndent);
      } else if (mac) {
         titleLabel->setIndent(2);
      } else if (classic) {
         titleLabel->setIndent(info.childMarginLeft);
      } else {
         titleLabel->setIndent(info.topLevelMarginLeft);
      }
      if (modern) {
         if (!placeholderWidget1) {
            placeholderWidget1 = new QWidget(antiFlickerWidget);
            placeholderWidget1->setBackgroundRole(QPalette::Base);
         }
         placeholderWidget1->setFixedHeight(info.topLevelMarginLeft + 2);
         mainLayout->addWidget(placeholderWidget1, row++, pageColumn);
      }
      mainLayout->addWidget(titleLabel, row++, pageColumn);
      if (modern) {
         if (!placeholderWidget2) {
            placeholderWidget2 = new QWidget(antiFlickerWidget);
            placeholderWidget2->setBackgroundRole(QPalette::Base);
         }
         placeholderWidget2->setFixedHeight(5);
         mainLayout->addWidget(placeholderWidget2, row++, pageColumn);
      }
      if (mac) {
         mainLayout->setRowMinimumHeight(row++, 7);
      }
   }
   if (placeholderWidget1) {
      placeholderWidget1->setVisible(info.title && modern);
   }
   if (placeholderWidget2) {
      placeholderWidget2->setVisible(info.title && modern);
   }

   if (info.subTitle) {
      if (!subTitleLabel) {
         subTitleLabel = new QLabel(pageFrame);
         subTitleLabel->setWordWrap(true);

         subTitleLabel->setContentsMargins(info.childMarginLeft, 0,
            info.childMarginRight, 0);

         pageVBoxLayout->insertWidget(1, subTitleLabel);
      }
   }

   // ### try to replace with margin.
   changeSpacerSize(pageVBoxLayout, 0, 0, info.subTitle ? info.childMarginLeft : 0);

   int hMargin = mac ? 1 : 0;
   int vMargin = hMargin;

   pageFrame->setFrameStyle(mac ? (QFrame::Box | QFrame::Raised) : QFrame::NoFrame);
   pageFrame->setLineWidth(0);
   pageFrame->setMidLineWidth(hMargin);

   if (info.header) {
      if (modern) {
         hMargin = info.topLevelMarginLeft;
         vMargin = deltaMarginBottom;
      } else if (classic) {
         hMargin = deltaMarginLeft + ClassicHMargin;
         vMargin = 0;
      }
   }

   if (aero) {
      int leftMargin   = 18; // ### hardcoded for now - should be calculated somehow
      int topMargin    = vMargin;
      int rightMargin  = hMargin; // ### for now
      int bottomMargin = vMargin;
      pageFrame->setContentsMargins(leftMargin, topMargin, rightMargin, bottomMargin);
   } else {
      pageFrame->setContentsMargins(hMargin, vMargin, hMargin, vMargin);
   }

   if ((info.watermark || info.sideWidget) && !watermarkLabel) {
      watermarkLabel = new QWatermarkLabel(antiFlickerWidget, sideWidget);
      watermarkLabel->setBackgroundRole(QPalette::Base);
      watermarkLabel->setMinimumHeight(1);
      watermarkLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
      watermarkLabel->setAlignment(Qt::AlignLeft | Qt::AlignTop);
   }

   //bool wasSemiTransparent = pageFrame->testAttribute(Qt::WA_SetPalette);
   const bool wasSemiTransparent =
      pageFrame->palette().brush(QPalette::Window).color().alpha() < 255
      || pageFrame->palette().brush(QPalette::Base).color().alpha() < 255;
   if (mac) {
      if (!wasSemiTransparent) {
         QPalette pal = pageFrame->palette();
         pal.setBrush(QPalette::Window, QColor(255, 255, 255, 153));
         // ### The next line is required to ensure visual semitransparency when
         // ### switching from ModernStyle to MacStyle. See TAG1 below.
         pal.setBrush(QPalette::Base, QColor(255, 255, 255, 153));
         pageFrame->setPalette(pal);
         pageFrame->setAutoFillBackground(true);
         antiFlickerWidget->setAutoFillBackground(false);
      }
   } else {
      if (wasSemiTransparent) {
         pageFrame->setPalette(QPalette());
      }

      bool baseBackground = (modern && !info.header); // ### TAG1
      pageFrame->setBackgroundRole(baseBackground ? QPalette::Base : QPalette::Window);

      if (titleLabel) {
         titleLabel->setAutoFillBackground(baseBackground);
      }
      pageFrame->setAutoFillBackground(baseBackground);
      if (watermarkLabel) {
         watermarkLabel->setAutoFillBackground(baseBackground);
      }
      if (placeholderWidget1) {
         placeholderWidget1->setAutoFillBackground(baseBackground);
      }
      if (placeholderWidget2) {
         placeholderWidget2->setAutoFillBackground(baseBackground);
      }

      if (aero) {
         QPalette pal = pageFrame->palette();
         pal.setBrush(QPalette::Window, QColor(255, 255, 255));
         pageFrame->setPalette(pal);
         pageFrame->setAutoFillBackground(true);
         pal = antiFlickerWidget->palette();
         pal.setBrush(QPalette::Window, QColor(255, 255, 255));
         antiFlickerWidget->setPalette(pal);
         antiFlickerWidget->setAutoFillBackground(true);
      }
   }

   mainLayout->addWidget(pageFrame, row++, pageColumn);

   int watermarkEndRow = row;
   if (classic) {
      mainLayout->setRowMinimumHeight(row++, deltaVSpacing);
   }

   if (aero) {
      buttonLayout->setContentsMargins(9, 9, 9, 9);
      mainLayout->setContentsMargins(0, 11, 0, 0);
   }

   int buttonStartColumn = info.extension ? 1 : 0;
   int buttonNumColumns = info.extension ? 1 : numColumns;

   if (classic || modern) {
      if (!bottomRuler) {
         bottomRuler = new QWizardRuler(antiFlickerWidget);
      }
      mainLayout->addWidget(bottomRuler, row++, buttonStartColumn, 1, buttonNumColumns);
   }

   if (classic) {
      mainLayout->setRowMinimumHeight(row++, deltaVSpacing);
   }

   mainLayout->addLayout(buttonLayout, row++, buttonStartColumn, 1, buttonNumColumns);

   if (info.watermark || info.sideWidget) {
      if (info.extension) {
         watermarkEndRow = row;
      }
      mainLayout->addWidget(watermarkLabel, watermarkStartRow, 0,
         watermarkEndRow - watermarkStartRow, 1);
   }

   mainLayout->setColumnMinimumWidth(0, mac && !info.watermark ? 181 : 0);
   if (mac) {
      mainLayout->setColumnMinimumWidth(2, 21);
   }

   if (headerWidget) {
      headerWidget->setVisible(info.header);
   }
   if (titleLabel) {
      titleLabel->setVisible(info.title);
   }
   if (subTitleLabel) {
      subTitleLabel->setVisible(info.subTitle);
   }
   if (bottomRuler) {
      bottomRuler->setVisible(classic || modern);
   }
   if (watermarkLabel) {
      watermarkLabel->setVisible(info.watermark || info.sideWidget);
   }

   layoutInfo = info;
}

void QWizardPrivate::updateLayout()
{
   Q_Q(QWizard);

   disableUpdates();

   QWizardLayoutInfo info = layoutInfoForCurrentPage();
   if (layoutInfo != info) {
      recreateLayout(info);
   }
   QWizardPage *page = q->currentPage();

   // If the page can expand vertically, let it stretch "infinitely" more
   // than the QSpacerItem at the bottom. Otherwise, let the QSpacerItem
   // stretch "infinitely" more than the page. Change the bottom item's
   // policy accordingly. The case that the page has no layout is basically
   // for Designer, only.
   if (page) {
      bool expandPage = !page->layout();
      if (!expandPage) {
         const QLayoutItem *pageItem = pageVBoxLayout->itemAt(pageVBoxLayout->indexOf(page));
         expandPage = pageItem->expandingDirections() & Qt::Vertical;
      }
      QSpacerItem *bottomSpacer = pageVBoxLayout->itemAt(pageVBoxLayout->count() -  1)->spacerItem();
      Q_ASSERT(bottomSpacer);
      bottomSpacer->changeSize(0, 0, QSizePolicy::Ignored, expandPage ? QSizePolicy::Ignored : QSizePolicy::MinimumExpanding);
      pageVBoxLayout->invalidate();
   }

   if (info.header) {
      Q_ASSERT(page);
      headerWidget->setup(info, page->title(), page->subTitle(),
         page->pixmap(QWizard::LogoPixmap), page->pixmap(QWizard::BannerPixmap),
         titleFmt, subTitleFmt);
   }

   if (info.watermark || info.sideWidget) {
      QPixmap pix;
      if (info.watermark) {
         if (page) {
            pix = page->pixmap(QWizard::WatermarkPixmap);
         } else {
            pix = q->pixmap(QWizard::WatermarkPixmap);
         }
      }

      // in case there is no watermark and we show the side widget we need to clear the watermark
      watermarkLabel->setPixmap(pix);
   }

   if (info.title) {
      Q_ASSERT(page);
      titleLabel->setTextFormat(titleFmt);
      titleLabel->setText(page->title());
   }
   if (info.subTitle) {
      Q_ASSERT(page);
      subTitleLabel->setTextFormat(subTitleFmt);
      subTitleLabel->setText(page->subTitle());
   }

   enableUpdates();
   updateMinMaxSizes(info);
}

void QWizardPrivate::updateMinMaxSizes(const QWizardLayoutInfo &info)
{
   Q_Q(QWizard);

   int extraHeight = 0;

#if !defined(QT_NO_STYLE_WINDOWSVISTA)
   if (isVistaThemeEnabled()) {
      extraHeight = vistaHelper->titleBarSize() + vistaHelper->topOffset();
   }
#endif

   QSize minimumSize = mainLayout->totalMinimumSize() + QSize(0, extraHeight);
   QSize maximumSize = mainLayout->totalMaximumSize();
   if (info.header && headerWidget->maximumWidth() != QWIDGETSIZE_MAX) {
      minimumSize.setWidth(headerWidget->maximumWidth());
      maximumSize.setWidth(headerWidget->maximumWidth());
   }
   if (info.watermark && !info.sideWidget) {
      minimumSize.setHeight(mainLayout->totalSizeHint().height());
   }
   if (q->minimumWidth() == minimumWidth) {
      minimumWidth = minimumSize.width();
      q->setMinimumWidth(minimumWidth);
   }
   if (q->minimumHeight() == minimumHeight) {
      minimumHeight = minimumSize.height();
      q->setMinimumHeight(minimumHeight);
   }
   if (q->maximumWidth() == maximumWidth) {
      maximumWidth = maximumSize.width();
      q->setMaximumWidth(maximumWidth);
   }
   if (q->maximumHeight() == maximumHeight) {
      maximumHeight = maximumSize.height();
      q->setMaximumHeight(maximumHeight);
   }
}

void QWizardPrivate::updateCurrentPage()
{
   Q_Q(QWizard);

   if (q->currentPage()) {
      canContinue = (q->nextId() != -1);
      canFinish = q->currentPage()->isFinalPage();
   } else {
      canContinue = false;
      canFinish = false;
   }

   _q_updateButtonStates();
   updateButtonTexts();
}

static QString object_name_for_button(QWizard::WizardButton which)
{
   switch (which) {
      case QWizard::CommitButton:
         return QString("qt_wizard_commit");

      case QWizard::FinishButton:
         return QString("qt_wizard_finish");

      case QWizard::CancelButton:
         return QString("qt_wizard_cancel");

      case QWizard::BackButton:
      case QWizard::NextButton:
      case QWizard::HelpButton:
      case QWizard::CustomButton1:
      case QWizard::CustomButton2:
      case QWizard::CustomButton3:
         // Make navigation buttons detectable as passive interactor in designer
         return QString("__qt__passive_wizardbutton") + QString::number(which);

      case QWizard::Stretch:
      case QWizard::NoButton:
//    case QWizard::NStandardButtons:
//    case QWizard::NButtons:
         break;
   }

   // error, may want to throw

   return QString();
}

bool QWizardPrivate::ensureButton(QWizard::WizardButton which) const
{
   Q_Q(const QWizard);
   if (uint(which) >= QWizard::NButtons) {
      return false;
   }

   if (! btns[which]) {
      QPushButton *pushButton = new QPushButton(antiFlickerWidget);
      QStyle *style = q->style();

      if (style != QApplication::style()) {
         // Propagate style
         pushButton->setStyle(style);
      }
      pushButton->setObjectName(object_name_for_button(which));

#ifdef Q_OS_DARWIN
      pushButton->setAutoDefault(false);
#endif

      pushButton->hide();
      btns[which] = pushButton;

      if (which < QWizard::NStandardButtons) {
         pushButton->setText(buttonDefaultText(wizStyle, which, this));
      }

      connectButton(which);
   }
   return true;
}

void QWizardPrivate::connectButton(QWizard::WizardButton which) const
{
   Q_Q(const QWizard);

   if (which < QWizard::NStandardButtons) {
      QObject::connect(btns[which], SIGNAL(clicked()), q, buttonSlots(which));
   } else {
      QObject::connect(btns[which], &QAbstractButton::clicked, q, &QWizard::_q_emitCustomButtonClicked);
   }
}

void QWizardPrivate::updateButtonTexts()
{
   Q_Q(QWizard);

   for (int i = 0; i < QWizard::NButtons; ++i) {
      if (btns[i]) {
         if (q->currentPage() && (q->currentPage()->d_func()->buttonCustomTexts.contains(i))) {
            btns[i]->setText(q->currentPage()->d_func()->buttonCustomTexts.value(i));

         } else if (buttonCustomTexts.contains(i)) {
            btns[i]->setText(buttonCustomTexts.value(i));

         } else if (i < QWizard::NStandardButtons) {
            btns[i]->setText(buttonDefaultText(wizStyle, i, this));
         }

      }
   }

   if (btns[QWizard::NextButton]) {
      btns[QWizard::NextButton]->setShortcut(isVistaThemeEnabled() ? QKeySequence(Qt::AltModifier | Qt::Key_Right) : QKeySequence());
   }
}

void QWizardPrivate::updateButtonLayout()
{
   if (buttonsHaveCustomLayout) {
      QVarLengthArray<QWizard::WizardButton, QWizard::NButtons> array(buttonsCustomLayout.count());

      for (int i = 0; i < buttonsCustomLayout.count(); ++i) {
         array[i] = buttonsCustomLayout.at(i);
      }

      setButtonLayout(array.constData(), array.count());

   } else {
      // Positions:
      // Help Stretch Custom1 Custom2 Custom3 Cancel Back Next Commit Finish Cancel Help

      const int ArraySize = 12;

      QWizard::WizardButton array[ArraySize];
      memset(array, -1, sizeof(array));
      Q_ASSERT(array[0] == QWizard::NoButton);

      if (opts & QWizard::HaveHelpButton) {
         int i = (opts & QWizard::HelpButtonOnRight) ? 11 : 0;
         array[i] = QWizard::HelpButton;
      }

      array[1] = QWizard::Stretch;
      if (opts & QWizard::HaveCustomButton1) {
         array[2] = QWizard::CustomButton1;
      }

      if (opts & QWizard::HaveCustomButton2) {
         array[3] = QWizard::CustomButton2;
      }

      if (opts & QWizard::HaveCustomButton3) {
         array[4] = QWizard::CustomButton3;
      }

      if (!(opts & QWizard::NoCancelButton)) {
         int i = (opts & QWizard::CancelButtonOnLeft) ? 5 : 10;
         array[i] = QWizard::CancelButton;
      }
      array[6] = QWizard::BackButton;
      array[7] = QWizard::NextButton;
      array[8] = QWizard::CommitButton;
      array[9] = QWizard::FinishButton;

      setButtonLayout(array, ArraySize);
   }
}

void QWizardPrivate::setButtonLayout(const QWizard::WizardButton *array, int maxSize)
{
   QWidget *prev = pageFrame;

   for (int i = buttonLayout->count() - 1; i >= 0; --i) {
      QLayoutItem *item = buttonLayout->takeAt(i);

      if (QWidget *widget = item->widget()) {
         widget->hide();
      }

      delete item;
   }

   for (int i = 0; i < maxSize; ++i) {
      QWizard::WizardButton which = array[i];

      if (which == QWizard::Stretch) {
         buttonLayout->addStretch(1);

      } else if (which != QWizard::NoButton) {
         ensureButton(which);
         buttonLayout->addWidget(btns[which]);

         // Back, Next, Commit, and Finish are handled in _q_updateButtonStates()
         if (which != QWizard::BackButton && which != QWizard::NextButton
            && which != QWizard::CommitButton && which != QWizard::FinishButton) {
            btns[which]->show();
         }

         if (prev) {
            QWidget::setTabOrder(prev, btns[which]);
         }
         prev = btns[which];
      }
   }

   _q_updateButtonStates();
}

bool QWizardPrivate::buttonLayoutContains(QWizard::WizardButton which)
{
   return !buttonsHaveCustomLayout || buttonsCustomLayout.contains(which);
}

void QWizardPrivate::updatePixmap(QWizard::WizardPixmap which)
{
   Q_Q(QWizard);
   if (which == QWizard::BackgroundPixmap) {
      if (wizStyle == QWizard::MacStyle) {
         q->update();
         q->updateGeometry();
      }
   } else {
      updateLayout();
   }
}

#if !defined(QT_NO_STYLE_WINDOWSVISTA)
bool QWizardPrivate::vistaDisabled() const
{
   Q_Q(const QWizard);
   const QVariant v = q->property("_q_wizard_vista_off");
   return v.isValid() && v.toBool();
}

bool QWizardPrivate::isVistaThemeEnabled(QVistaHelper::VistaState state) const
{
   return wizStyle == QWizard::AeroStyle && QVistaHelper::vistaState() == state && !vistaDisabled();
}

bool QWizardPrivate::handleAeroStyleChange()
{
   Q_Q(QWizard);

   if (inHandleAeroStyleChange) {
      return false;   // prevent recursion
   }

   // DWM changes. Delay aero initialization to the show event handling if
   // it does not exist. If we are a child, skip DWM and just make room by
   // moving the antiFlickerWidget.
   const bool isWindow = q->isWindow();
   if (isWindow && (!q->windowHandle() || !q->windowHandle()->handle())) {
      return false;
   }
   inHandleAeroStyleChange = true;

   vistaHelper->disconnectBackButton();
   q->removeEventFilter(vistaHelper);

   bool vistaMargins = false;
   if (isVistaThemeEnabled()) {
      if (isVistaThemeEnabled(QVistaHelper::VistaAero)) {
         if (isWindow) {
            vistaHelper->setDWMTitleBar(QVistaHelper::ExtendedTitleBar);
            q->installEventFilter(vistaHelper);
         }
         q->setMouseTracking(true);

         antiFlickerWidget->move(0, vistaHelper->titleBarSize() + vistaHelper->topOffset());

         // ### should ideally work without the '+ 1'
         vistaHelper->backButton()->move(0, vistaHelper->topOffset()
               - qMin(vistaHelper->topOffset(), vistaHelper->topPadding() + 1));

         vistaMargins = true;
         vistaHelper->backButton()->show();

      } else {
         if (isWindow) {
            vistaHelper->setDWMTitleBar(QVistaHelper::NormalTitleBar);
         }

         q->setMouseTracking(true);
         antiFlickerWidget->move(0, vistaHelper->topOffset());
         vistaHelper->backButton()->move(0, -1); // ### should ideally work with (0, 0)
      }

      if (isWindow) {
         vistaHelper->setTitleBarIconAndCaptionVisible(false);
      }

      QObject::connect(vistaHelper->backButton(), SIGNAL(clicked()), q, buttonSlots(QWizard::BackButton));
      vistaHelper->backButton()->show();

   } else {
      q->setMouseTracking(true); // ### original value possibly different

#ifndef QT_NO_CURSOR
      q->unsetCursor();
#endif

      antiFlickerWidget->move(0, 0);
      vistaHelper->hideBackButton();
      if (isWindow) {
         vistaHelper->setTitleBarIconAndCaptionVisible(true);
      }
   }

   _q_updateButtonStates();

   vistaHelper->updateCustomMargins(vistaMargins);


   inHandleAeroStyleChange = false;
   return true;
}
#endif

bool QWizardPrivate::isVistaThemeEnabled() const
{
#if !defined(QT_NO_STYLE_WINDOWSVISTA)
   return isVistaThemeEnabled(QVistaHelper::VistaAero)
      || isVistaThemeEnabled(QVistaHelper::VistaBasic);
#else
   return false;
#endif
}

void QWizardPrivate::disableUpdates()
{
   Q_Q(QWizard);
   if (disableUpdatesCount++ == 0) {
      q->setUpdatesEnabled(false);
      antiFlickerWidget->hide();
   }
}

void QWizardPrivate::enableUpdates()
{
   Q_Q(QWizard);
   if (--disableUpdatesCount == 0) {
      antiFlickerWidget->show();
      q->setUpdatesEnabled(true);
   }
}

void QWizardPrivate::_q_emitCustomButtonClicked()
{
   Q_Q(QWizard);
   QObject *button = q->sender();
   for (int i = QWizard::NStandardButtons; i < QWizard::NButtons; ++i) {
      if (btns[i] == button) {
         emit q->customButtonClicked(QWizard::WizardButton(i));
         break;
      }
   }
}

void QWizardPrivate::_q_updateButtonStates()
{
   Q_Q(QWizard);

   disableUpdates();

   const QWizardPage *page = q->currentPage();
   bool complete = page && page->isComplete();

   btn.back->setEnabled(history.count() > 1
      && !q->page(history.at(history.count() - 2))->isCommitPage()
      && (!canFinish || !(opts & QWizard::DisabledBackButtonOnLastPage)));
   btn.next->setEnabled(canContinue && complete);
   btn.commit->setEnabled(canContinue && complete);
   btn.finish->setEnabled(canFinish && complete);

   const bool backButtonVisible = buttonLayoutContains(QWizard::BackButton)
      && (history.count() > 1 || !(opts & QWizard::NoBackButtonOnStartPage))
      && (canContinue || !(opts & QWizard::NoBackButtonOnLastPage));
   bool commitPage = page && page->isCommitPage();
   btn.back->setVisible(backButtonVisible);
   btn.next->setVisible(buttonLayoutContains(QWizard::NextButton) && !commitPage
      && (canContinue || (opts & QWizard::HaveNextButtonOnLastPage)));
   btn.commit->setVisible(buttonLayoutContains(QWizard::CommitButton) && commitPage
      && canContinue);
   btn.finish->setVisible(buttonLayoutContains(QWizard::FinishButton)
      && (canFinish || (opts & QWizard::HaveFinishButtonOnEarlyPages)));

   if (!(opts & QWizard::NoCancelButton))
      btn.cancel->setVisible(buttonLayoutContains(QWizard::CancelButton)
         && (canContinue || !(opts & QWizard::NoCancelButtonOnLastPage)));
   bool useDefault = !(opts & QWizard::NoDefaultButton);
   if (QPushButton *nextPush = qobject_cast<QPushButton *>(btn.next)) {
      nextPush->setDefault(canContinue && useDefault && !commitPage);
   }
   if (QPushButton *commitPush = qobject_cast<QPushButton *>(btn.commit)) {
      commitPush->setDefault(canContinue && useDefault && commitPage);
   }
   if (QPushButton *finishPush = qobject_cast<QPushButton *>(btn.finish)) {
      finishPush->setDefault(!canContinue && useDefault);
   }

#if !defined(QT_NO_STYLE_WINDOWSVISTA)
   if (isVistaThemeEnabled()) {
      vistaHelper->backButton()->setEnabled(btn.back->isEnabled());
      vistaHelper->backButton()->setVisible(backButtonVisible);
      btn.back->setVisible(false);
   }
#endif

   enableUpdates();
}

void QWizardPrivate::_q_handleFieldObjectDestroyed(QObject *object)
{
   int destroyed_index = -1;
   QVector<QWizardField>::iterator it = fields.begin();

   while (it != fields.end()) {
      const QWizardField &field = *it;

      if (field.object == object) {
         destroyed_index = fieldIndexMap.value(field.name, -1);
         fieldIndexMap.remove(field.name);
         it = fields.erase(it);
      } else {
         ++it;
      }
   }

   if (destroyed_index != -1) {
      QMap<QString, int>::iterator it2 = fieldIndexMap.begin();

      while (it2 != fieldIndexMap.end()) {
         int index = it2.value();
         if (index > destroyed_index) {
            QString field_name = it2.key();
            fieldIndexMap.insert(field_name, index - 1);
         }
         ++it2;
      }
   }
}

void QWizardPrivate::setStyle(QStyle *style)
{
   for (int i = 0; i < QWizard::NButtons; i++)
      if (btns[i]) {
         btns[i]->setStyle(style);
      }
   const PageMap::const_iterator pcend = pageMap.constEnd();
   for (PageMap::const_iterator it = pageMap.constBegin(); it != pcend; ++it) {
      it.value()->setStyle(style);
   }
}

#ifdef Q_OS_DARWIN

QPixmap QWizardPrivate::findDefaultBackgroundPixmap()
{
   QGuiApplication *app = qobject_cast<QGuiApplication *>(QCoreApplication::instance());
   if (!app) {
      return QPixmap();
   }
   QPlatformNativeInterface *platformNativeInterface = app->platformNativeInterface();
   int at = platformNativeInterface->metaObject()->indexOfMethod("defaultBackgroundPixmapForQWizard()");
   if (at == -1) {
      return QPixmap();
   }
   QMetaMethod defaultBackgroundPixmapForQWizard = platformNativeInterface->metaObject()->method(at);
   QPixmap result;
   if (!defaultBackgroundPixmapForQWizard.invoke(platformNativeInterface, Q_RETURN_ARG(QPixmap, result))) {
      return QPixmap();
   }
   return result;
}

#endif

#if !defined(QT_NO_STYLE_WINDOWSVISTA)
void QWizardAntiFlickerWidget::paintEvent(QPaintEvent *)
{
   if (wizardPrivate->isVistaThemeEnabled()) {
      int leftMargin, topMargin, rightMargin, bottomMargin;
      wizardPrivate->buttonLayout->getContentsMargins(
         &leftMargin, &topMargin, &rightMargin, &bottomMargin);
      const int buttonLayoutTop = wizardPrivate->buttonLayout->contentsRect().top() - topMargin;
      QPainter painter(this);
      const QBrush brush(QColor(240, 240, 240)); // ### hardcoded for now
      painter.fillRect(0, buttonLayoutTop, width(), height() - buttonLayoutTop, brush);
      painter.setPen(QPen(QBrush(QColor(223, 223, 223)), 0)); // ### hardcoded for now
      painter.drawLine(0, buttonLayoutTop, width(), buttonLayoutTop);

      if (wizardPrivate->isVistaThemeEnabled(QVistaHelper::VistaBasic)) {
         if (window()->isActiveWindow()) {
            painter.setPen(QPen(QBrush(QColor(169, 191, 214)), 0));   // ### hardcoded for now
         } else {
            painter.setPen(QPen(QBrush(QColor(182, 193, 204)), 0));   // ### hardcoded for now
         }
         painter.drawLine(0, 0, width(), 0);
      }
   }
}
#endif

QWizard::QWizard(QWidget *parent, Qt::WindowFlags flags)
   : QDialog(*new QWizardPrivate, parent, flags)
{
   Q_D(QWizard);
   d->init();
}

QWizard::~QWizard()
{
   Q_D(QWizard);
   delete d->buttonLayout;
}

int QWizard::addPage(QWizardPage *page)
{
   Q_D(QWizard);
   int theid = 0;
   if (!d->pageMap.isEmpty()) {
      theid = (d->pageMap.constEnd() - 1).key() + 1;
   }
   setPage(theid, page);
   return theid;
}

void QWizard::setPage(int theid, QWizardPage *page)
{
   Q_D(QWizard);

   if (! page) {
      qWarning("QWizard::setPage() Unable to insert invalid page (nullptr)");
      return;
   }

   if (theid == -1) {
      qWarning("QWizard::setPage() Unable to insert page with ID -1");
      return;
   }

   if (d->pageMap.contains(theid)) {
      qWarning("QWizard::setPage() Duplicate page ID %d ignored", theid);
      return;
   }

   page->setParent(d->pageFrame);

   QVector<QWizardField> &pendingFields = page->d_func()->pendingFields;
   for (int i = 0; i < pendingFields.count(); ++i) {
      d->addField(pendingFields.at(i));
   }
   pendingFields.clear();

   connect(page, &QWizardPage::completeChanged, this, &QWizard::_q_updateButtonStates);

   d->pageMap.insert(theid, page);
   page->d_func()->wizard = this;

   int n = d->pageVBoxLayout->count();

   // disable layout to prevent layout updates while adding
   bool pageVBoxLayoutEnabled = d->pageVBoxLayout->isEnabled();
   d->pageVBoxLayout->setEnabled(false);

   d->pageVBoxLayout->insertWidget(n - 1, page);

   // hide new page and reset layout to old status
   page->hide();
   d->pageVBoxLayout->setEnabled(pageVBoxLayoutEnabled);

   if (!d->startSetByUser && d->pageMap.constBegin().key() == theid) {
      d->start = theid;
   }
   emit pageAdded(theid);
}

void QWizard::removePage(int id)
{
   Q_D(QWizard);

   QWizardPage *removedPage = nullptr;

   // update startItem accordingly
   if (d->pageMap.count() > 0) { // only if we have any pages
      if (d->start == id) {
         const int firstId = d->pageMap.constBegin().key();
         if (firstId == id) {
            if (d->pageMap.count() > 1) {
               d->start = (++d->pageMap.constBegin()).key();   // secondId
            } else {
               d->start = -1;   // removing the last page
            }
         } else { // startSetByUser has to be "true" here
            d->start = firstId;
         }
         d->startSetByUser = false;
      }
   }

   if (d->pageMap.contains(id)) {
      emit pageRemoved(id);
   }

   if (!d->history.contains(id)) {
      // Case 1: removing a page not in the history
      removedPage = d->pageMap.take(id);
      d->updateCurrentPage();
   } else if (id != d->current) {
      // Case 2: removing a page in the history before the current page
      removedPage = d->pageMap.take(id);
      d->history.removeOne(id);
      d->_q_updateButtonStates();
   } else if (d->history.count() == 1) {
      // Case 3: removing the current page which is the first (and only) one in the history
      d->reset();
      removedPage = d->pageMap.take(id);
      if (d->pageMap.isEmpty()) {
         d->updateCurrentPage();
      } else {
         restart();
      }
   } else {
      // Case 4: removing the current page which is not the first one in the history
      back();
      removedPage = d->pageMap.take(id);
      d->updateCurrentPage();
   }

   if (removedPage) {
      if (d->initialized.contains(id)) {
         cleanupPage(id);
         d->initialized.remove(id);
      }

      d->pageVBoxLayout->removeWidget(removedPage);

      for (int i = d->fields.count() - 1; i >= 0; --i) {
         if (d->fields.at(i).page == removedPage) {
            removedPage->d_func()->pendingFields += d->fields.at(i);
            d->removeFieldAt(i);
         }
      }
   }
}

QWizardPage *QWizard::page(int theid) const
{
   Q_D(const QWizard);
   return d->pageMap.value(theid);
}


bool QWizard::hasVisitedPage(int theid) const
{
   Q_D(const QWizard);
   return d->history.contains(theid);
}

QList<int> QWizard::visitedPages() const
{
   Q_D(const QWizard);
   return d->history;
}


QList<int> QWizard::pageIds() const
{
   Q_D(const QWizard);
   return d->pageMap.keys();
}

void QWizard::setStartId(int theid)
{
   Q_D(QWizard);
   int newStart = theid;
   if (theid == -1) {
      newStart = d->pageMap.count() ? d->pageMap.constBegin().key() : -1;
   }

   if (d->start == newStart) {
      d->startSetByUser = theid != -1;
      return;
   }

   if (!d->pageMap.contains(newStart)) {
      qWarning("QWizard::setStartId() Invalid page ID %d", newStart);
      return;
   }
   d->start = newStart;
   d->startSetByUser = theid != -1;
}

int QWizard::startId() const
{
   Q_D(const QWizard);
   return d->start;
}

QWizardPage *QWizard::currentPage() const
{
   Q_D(const QWizard);
   return page(d->current);
}

int QWizard::currentId() const
{
   Q_D(const QWizard);
   return d->current;
}

void QWizard::setField(const QString &name, const QVariant &value)
{
   Q_D(QWizard);

   int index = d->fieldIndexMap.value(name, -1);
   if (index != -1) {
      const QWizardField &field = d->fields.at(index);

      if (! field.object->setProperty(field.property, value)) {
         qWarning("QWizard::setField() Unable to write to property '%s'", field.property.constData());
      }

      return;
   }

   qWarning("QWizard::setField() Field '%s' does not exist", csPrintable(name));
}

QVariant QWizard::field(const QString &name) const
{
   Q_D(const QWizard);

   int index = d->fieldIndexMap.value(name, -1);
   if (index != -1) {
      const QWizardField &field = d->fields.at(index);
      return field.object->property(field.property);
   }

   qWarning("QWizard::field() Field '%s' does not exist", csPrintable(name));
   return QVariant();
}

void QWizard::setWizardStyle(WizardStyle style)
{
   Q_D(QWizard);

   const bool styleChange = style != d->wizStyle;

#if ! defined(QT_NO_STYLE_WINDOWSVISTA)
   const bool aeroStyleChange =
      d->vistaInitPending || d->vistaStateChanged || (styleChange && (style == AeroStyle || d->wizStyle == AeroStyle));
   d->vistaStateChanged = false;
   d->vistaInitPending = false;
#endif

   if (styleChange
#if ! defined(QT_NO_STYLE_WINDOWSVISTA)
      || aeroStyleChange
#endif
   ) {
      d->disableUpdates();
      d->wizStyle = style;
      d->updateButtonTexts();

#if !defined(QT_NO_STYLE_WINDOWSVISTA)
      if (aeroStyleChange) {
         // Send a resizeevent since the antiflicker widget probably needs a new size
         // because of the backbutton in the window title
         QResizeEvent ev(geometry().size(), geometry().size());
         QApplication::sendEvent(this, &ev);
      }
#endif

      d->updateLayout();
      updateGeometry();
      d->enableUpdates();

#if !defined(QT_NO_STYLE_WINDOWSVISTA)
      // Delay initialization when activating Aero style fails due to missing native window.
      if (aeroStyleChange && !d->handleAeroStyleChange() && d->wizStyle == AeroStyle) {
         d->vistaInitPending = true;
      }
#endif
   }
}

QWizard::WizardStyle QWizard::wizardStyle() const
{
   Q_D(const QWizard);
   return d->wizStyle;
}

void QWizard::setOption(WizardOption option, bool on)
{
   Q_D(QWizard);
   if (!(d->opts & option) != !on) {
      setOptions(d->opts ^ option);
   }
}

bool QWizard::testOption(WizardOption option) const
{
   Q_D(const QWizard);
   return (d->opts & option) != 0;
}

void QWizard::setOptions(WizardOptions options)
{
   Q_D(QWizard);

   WizardOptions changed = (options ^ d->opts);
   if (!changed) {
      return;
   }

   d->disableUpdates();

   d->opts = options;
   if ((changed & IndependentPages) && !(d->opts & IndependentPages)) {
      d->cleanupPagesNotInHistory();
   }

   if (changed & (NoDefaultButton | HaveHelpButton | HelpButtonOnRight | NoCancelButton
         | CancelButtonOnLeft | HaveCustomButton1 | HaveCustomButton2
         | HaveCustomButton3)) {
      d->updateButtonLayout();

   } else if (changed & (NoBackButtonOnStartPage | NoBackButtonOnLastPage
         | HaveNextButtonOnLastPage | HaveFinishButtonOnEarlyPages
         | DisabledBackButtonOnLastPage | NoCancelButtonOnLastPage)) {
      d->_q_updateButtonStates();
   }

   d->enableUpdates();
   d->updateLayout();
}

QWizard::WizardOptions QWizard::options() const
{
   Q_D(const QWizard);
   return d->opts;
}

void QWizard::setButtonText(WizardButton which, const QString &text)
{
   Q_D(QWizard);

   if (!d->ensureButton(which)) {
      return;
   }

   d->buttonCustomTexts.insert(which, text);

   if (!currentPage() || !currentPage()->d_func()->buttonCustomTexts.contains(which)) {
      d->btns[which]->setText(text);
   }
}

QString QWizard::buttonText(WizardButton which) const
{
   Q_D(const QWizard);

   if (!d->ensureButton(which)) {
      return QString();
   }

   if (d->buttonCustomTexts.contains(which)) {
      return d->buttonCustomTexts.value(which);
   }

   const QString defText = buttonDefaultText(d->wizStyle, which, d);
   if (! defText.isEmpty()) {
      return defText;
   }

   return d->btns[which]->text();
}

void QWizard::setButtonLayout(const QList<WizardButton> &layout)
{
   Q_D(QWizard);

   for (int i = 0; i < layout.count(); ++i) {
      WizardButton button1 = layout.at(i);

      if (button1 == NoButton || button1 == Stretch) {
         continue;
      }
      if (!d->ensureButton(button1)) {
         return;
      }

      // O(n^2), but n is very small
      for (int j = 0; j < i; ++j) {
         WizardButton button2 = layout.at(j);
         if (button2 == button1) {
            qWarning("QWizard::setButtonLayout() Duplicate button in layout");
            return;
         }
      }
   }

   d->buttonsHaveCustomLayout = true;
   d->buttonsCustomLayout = layout;
   d->updateButtonLayout();
}

void QWizard::setButton(WizardButton which, QAbstractButton *button)
{
   Q_D(QWizard);

   if (uint(which) >= NButtons || d->btns[which] == button) {
      return;
   }

   if (QAbstractButton *oldButton = d->btns[which]) {
      d->buttonLayout->removeWidget(oldButton);
      delete oldButton;
   }

   d->btns[which] = button;
   if (button) {
      button->setParent(d->antiFlickerWidget);
      d->buttonCustomTexts.insert(which, button->text());
      d->connectButton(which);
   } else {
      d->buttonCustomTexts.remove(which); // ### what about page-specific texts set for 'which'
      d->ensureButton(which);             // (QWizardPage::setButtonText())? Clear them as well?
   }

   d->updateButtonLayout();
}

QAbstractButton *QWizard::button(WizardButton which) const
{
   Q_D(const QWizard);

#if !defined(QT_NO_STYLE_WINDOWSVISTA)
   if (d->wizStyle == AeroStyle && which == BackButton) {
      return d->vistaHelper->backButton();
   }
#endif
   if (!d->ensureButton(which)) {
      return nullptr;
   }
   return d->btns[which];
}


void QWizard::setTitleFormat(Qt::TextFormat format)
{
   Q_D(QWizard);
   d->titleFmt = format;
   d->updateLayout();
}

Qt::TextFormat QWizard::titleFormat() const
{
   Q_D(const QWizard);
   return d->titleFmt;
}

void QWizard::setSubTitleFormat(Qt::TextFormat format)
{
   Q_D(QWizard);
   d->subTitleFmt = format;
   d->updateLayout();
}

Qt::TextFormat QWizard::subTitleFormat() const
{
   Q_D(const QWizard);
   return d->subTitleFmt;
}

void QWizard::setPixmap(WizardPixmap which, const QPixmap &pixmap)
{
   Q_D(QWizard);

   Q_ASSERT(uint(which) < NPixmaps);
   d->defaultPixmaps[which] = pixmap;
   d->updatePixmap(which);
}

QPixmap QWizard::pixmap(WizardPixmap which) const
{
   Q_D(const QWizard);
   Q_ASSERT(uint(which) < NPixmaps);

#ifdef Q_OS_DARWIN
   if (which == BackgroundPixmap && d->defaultPixmaps[BackgroundPixmap].isNull()) {
      d->defaultPixmaps[BackgroundPixmap] = d->findDefaultBackgroundPixmap();
   }
#endif
   return d->defaultPixmaps[which];
}

void QWizard::setDefaultProperty(const QString &className, const QString &property, const QString &changedSignal)
{
   Q_D(QWizard);

   for (int i = d->defaultPropertyTable.count() - 1; i >= 0; --i) {
      if (d->defaultPropertyTable.at(i).m_className == className) {
         d->defaultPropertyTable.remove(i);
         break;
      }
   }

   d->defaultPropertyTable.append(QWizardDefaultProperty(className, property, changedSignal));
}

void QWizard::setSideWidget(QWidget *widget)
{
   Q_D(QWizard);

   d->sideWidget = widget;

   if (d->watermarkLabel) {
      d->watermarkLabel->setSideWidget(widget);
      d->updateLayout();
   }
}

QWidget *QWizard::sideWidget() const
{
   Q_D(const QWizard);

   return d->sideWidget;
}

void QWizard::setVisible(bool visible)
{
   Q_D(QWizard);

   if (visible) {
      if (d->current == -1) {
         restart();
      }
   }

   QDialog::setVisible(visible);
}

QSize QWizard::sizeHint() const
{
   Q_D(const QWizard);
   QSize result = d->mainLayout->totalSizeHint();
   QSize extra(500, 360);

   if (d->wizStyle == MacStyle && d->current != -1) {
      QSize pixmap(currentPage()->pixmap(BackgroundPixmap).size());
      extra.setWidth(616);
      if (!pixmap.isNull()) {
         extra.setHeight(pixmap.height());

         /*
             The width isn't always reliable as a size hint, as
             some wizard backgrounds just cover the leftmost area.
             Use a rule of thumb to determine if the width is
             reliable or not.
         */
         if (pixmap.width() >= pixmap.height()) {
            extra.setWidth(pixmap.width());
         }
      }
   }

   return result.expandedTo(extra);
}

void QWizard::back()
{
   Q_D(QWizard);

   int n = d->history.count() - 2;

   if (n < 0) {
      return;
   }
   d->switchToPage(d->history.at(n), QWizardPrivate::Backward);
}

void QWizard::next()
{
   Q_D(QWizard);

   if (d->current == -1) {
      return;
   }

   if (validateCurrentPage()) {
      int next = nextId();
      if (next != -1) {
         if (d->history.contains(next)) {
            qWarning("QWizard::next() Page %d already exists", next);
            return;
         }

         if (!d->pageMap.contains(next)) {
            qWarning("QWizard::next() No such page with the value of %d", next);
            return;
         }
         d->switchToPage(next, QWizardPrivate::Forward);
      }
   }
}

void QWizard::restart()
{
   Q_D(QWizard);
   d->disableUpdates();
   d->reset();
   d->switchToPage(startId(), QWizardPrivate::Forward);
   d->enableUpdates();
}

bool QWizard::event(QEvent *event)
{
   Q_D(QWizard);
   if (event->type() == QEvent::StyleChange) { // Propagate style
      d->setStyle(style());
      d->updateLayout();
   }

#if !defined(QT_NO_STYLE_WINDOWSVISTA)

   else if (event->type() == QEvent::Show && d->vistaInitPending) {
      d->vistaInitPending = false;
      if (QVistaHelper::vistaState() != QVistaHelper::Classic) {
         d->wizStyle = AeroStyle;
      }

      d->handleAeroStyleChange();

   } else if (d->isVistaThemeEnabled()) {
      if (event->type() == QEvent::Resize
         || event->type() == QEvent::LayoutDirectionChange) {
         const int buttonLeft = (layoutDirection() == Qt::RightToLeft
               ? width() - d->vistaHelper->backButton()->sizeHint().width()
               : 0);

         d->vistaHelper->backButton()->move(buttonLeft, d->vistaHelper->backButton()->y());
      }

      d->vistaHelper->mouseEvent(event);
   }
#endif

   return QDialog::event(event);
}

void QWizard::resizeEvent(QResizeEvent *event)
{
   Q_D(QWizard);
   int heightOffset = 0;

#if ! defined(QT_NO_STYLE_WINDOWSVISTA)
   if (d->isVistaThemeEnabled()) {
      heightOffset = d->vistaHelper->topOffset();
      if (d->isVistaThemeEnabled(QVistaHelper::VistaAero)) {
         heightOffset += d->vistaHelper->titleBarSize();
      }
   }
#endif

   d->antiFlickerWidget->resize(event->size().width(), event->size().height() - heightOffset);

#if ! defined(QT_NO_STYLE_WINDOWSVISTA)
   if (d->isVistaThemeEnabled()) {
      d->vistaHelper->resizeEvent(event);
   }
#endif

   QDialog::resizeEvent(event);
}

void QWizard::paintEvent(QPaintEvent *event)
{
   Q_D(QWizard);

   if (d->wizStyle == MacStyle && currentPage()) {
      QPixmap backgroundPixmap = currentPage()->pixmap(BackgroundPixmap);
      if (backgroundPixmap.isNull()) {
         return;
      }

      QPainter painter(this);
      painter.drawPixmap(0, (height() - backgroundPixmap.height()) / 2, backgroundPixmap);
   }

#if ! defined(QT_NO_STYLE_WINDOWSVISTA)
   else if (d->isVistaThemeEnabled()) {
      if (d->isVistaThemeEnabled(QVistaHelper::VistaBasic)) {
         QPainter painter(this);
         QColor color = d->vistaHelper->basicWindowFrameColor();
         painter.fillRect(0, 0, width(), QVistaHelper::topOffset(), color);
      }

      d->vistaHelper->paintEvent(event);
   }
#else
   (void) event;
#endif
}

#if defined(Q_OS_WIN)

bool QWizard::nativeEvent(const QByteArray &eventType, void *message, long *result)
{
#if ! defined(QT_NO_STYLE_WINDOWSVISTA)
   Q_D(QWizard);

   if (d->isVistaThemeEnabled() && eventType == "windows_generic_MSG") {
      MSG *windowsMessage = static_cast<MSG *>(message);
      const bool winEventResult = d->vistaHelper->handleWinEvent(windowsMessage, result);

      if (QVistaHelper::vistaState() != d->vistaState) {
         d->vistaState = QVistaHelper::vistaState();
         d->vistaStateChanged = true;
         setWizardStyle(AeroStyle);
      }
      return winEventResult;

   } else {
      return QDialog::nativeEvent(eventType, message, result);
   }

#else
   return QDialog::nativeEvent(eventType, message, result);
#endif

}
#endif

void QWizard::done(int result)
{
   Q_D(QWizard);
   // canceling leaves the wizard in a known state

   if (result == Rejected) {
      d->reset();

   } else {
      if (!validateCurrentPage()) {
         return;
      }
   }

   QDialog::done(result);
}

void QWizard::initializePage(int theid)
{
   QWizardPage *page = this->page(theid);
   if (page) {
      page->initializePage();
   }
}

void QWizard::cleanupPage(int theid)
{
   QWizardPage *page = this->page(theid);
   if (page) {
      page->cleanupPage();
   }
}

bool QWizard::validateCurrentPage()
{
   QWizardPage *page = currentPage();
   if (!page) {
      return true;
   }

   return page->validatePage();
}

int QWizard::nextId() const
{
   const QWizardPage *page = currentPage();

   if (! page) {
      return -1;
   }

   return page->nextId();
}

QWizardPage::QWizardPage(QWidget *parent)
   : QWidget(*new QWizardPagePrivate, parent, Qt::EmptyFlag)
{
   connect(this, &QWizardPage::completeChanged, this, &QWizardPage::_q_updateCachedCompleteState);
}

QWizardPage::~QWizardPage()
{
}

void QWizardPage::setTitle(const QString &title)
{
   Q_D(QWizardPage);
   d->title = title;

   if (d->wizard && d->wizard->currentPage() == this) {
      d->wizard->d_func()->updateLayout();
   }
}

QString QWizardPage::title() const
{
   Q_D(const QWizardPage);
   return d->title;
}

void QWizardPage::setSubTitle(const QString &subTitle)
{
   Q_D(QWizardPage);
   d->subTitle = subTitle;

   if (d->wizard && d->wizard->currentPage() == this) {
      d->wizard->d_func()->updateLayout();
   }
}

QString QWizardPage::subTitle() const
{
   Q_D(const QWizardPage);
   return d->subTitle;
}

void QWizardPage::setPixmap(QWizard::WizardPixmap which, const QPixmap &pixmap)
{
   Q_D(QWizardPage);
   Q_ASSERT(uint(which) < QWizard::NPixmaps);
   d->pixmaps[which] = pixmap;
   if (d->wizard && d->wizard->currentPage() == this) {
      d->wizard->d_func()->updatePixmap(which);
   }
}

QPixmap QWizardPage::pixmap(QWizard::WizardPixmap which) const
{
   Q_D(const QWizardPage);
   Q_ASSERT(uint(which) < QWizard::NPixmaps);

   const QPixmap &pixmap = d->pixmaps[which];
   if (!pixmap.isNull()) {
      return pixmap;
   }

   if (wizard()) {
      return wizard()->pixmap(which);
   }

   return pixmap;
}

void QWizardPage::initializePage()
{
}

void QWizardPage::cleanupPage()
{
   Q_D(QWizardPage);
   if (d->wizard) {
      QVector<QWizardField> &fields = d->wizard->d_func()->fields;
      for (int i = 0; i < fields.count(); ++i) {
         const QWizardField &field = fields.at(i);
         if (field.page == this) {
            field.object->setProperty(field.property, field.initialValue);
         }
      }
   }
}

bool QWizardPage::validatePage()
{
   return true;
}

bool QWizardPage::isComplete() const
{
   Q_D(const QWizardPage);

   if (!d->wizard) {
      return true;
   }

   const QVector<QWizardField> &wizardFields = d->wizard->d_func()->fields;
   for (int i = wizardFields.count() - 1; i >= 0; --i) {
      const QWizardField &field = wizardFields.at(i);
      if (field.page == this && field.mandatory) {
         QVariant value = field.object->property(field.property);
         if (value == field.initialValue) {
            return false;
         }

#ifndef QT_NO_LINEEDIT
         if (QLineEdit *lineEdit = qobject_cast<QLineEdit *>(field.object)) {
            if (!lineEdit->hasAcceptableInput()) {
               return false;
            }
         }
#endif
#ifndef QT_NO_SPINBOX
         if (QAbstractSpinBox *spinBox = qobject_cast<QAbstractSpinBox *>(field.object)) {
            if (!spinBox->hasAcceptableInput()) {
               return false;
            }
         }
#endif
      }
   }
   return true;
}

void QWizardPage::setFinalPage(bool finalPage)
{
   Q_D(QWizardPage);
   d->explicitlyFinal = finalPage;
   QWizard *wizard = this->wizard();
   if (wizard && wizard->currentPage() == this) {
      wizard->d_func()->updateCurrentPage();
   }
}

bool QWizardPage::isFinalPage() const
{
   Q_D(const QWizardPage);
   if (d->explicitlyFinal) {
      return true;
   }

   QWizard *wizard = this->wizard();
   if (wizard && wizard->currentPage() == this) {
      // try to use the QWizard implementation if possible
      return wizard->nextId() == -1;
   } else {
      return nextId() == -1;
   }
}

void QWizardPage::setCommitPage(bool commitPage)
{
   Q_D(QWizardPage);

   d->commit = commitPage;
   QWizard *wizard = this->wizard();

   if (wizard && wizard->currentPage() == this) {
      wizard->d_func()->updateCurrentPage();
   }
}

bool QWizardPage::isCommitPage() const
{
   Q_D(const QWizardPage);
   return d->commit;
}

void QWizardPage::setButtonText(QWizard::WizardButton which, const QString &text)
{
   Q_D(QWizardPage);
   d->buttonCustomTexts.insert(which, text);
   if (wizard() && wizard()->currentPage() == this && wizard()->d_func()->btns[which]) {
      wizard()->d_func()->btns[which]->setText(text);
   }
}

QString QWizardPage::buttonText(QWizard::WizardButton which) const
{
   Q_D(const QWizardPage);

   if (d->buttonCustomTexts.contains(which)) {
      return d->buttonCustomTexts.value(which);
   }

   if (wizard()) {
      return wizard()->buttonText(which);
   }

   return QString();
}

int QWizardPage::nextId() const
{
   Q_D(const QWizardPage);

   if (!d->wizard) {
      return -1;
   }

   bool foundCurrentPage = false;

   const QWizardPrivate::PageMap &pageMap = d->wizard->d_func()->pageMap;
   QWizardPrivate::PageMap::const_iterator i = pageMap.constBegin();
   QWizardPrivate::PageMap::const_iterator end = pageMap.constEnd();

   for (; i != end; ++i) {
      if (i.value() == this) {
         foundCurrentPage = true;
      } else if (foundCurrentPage) {
         return i.key();
      }
   }

   return -1;
}

void QWizardPage::setField(const QString &name, const QVariant &value)
{
   Q_D(QWizardPage);

   if (! d->wizard) {
      return;
   }

   d->wizard->setField(name, value);
}

QVariant QWizardPage::field(const QString &name) const
{
   Q_D(const QWizardPage);

   if (! d->wizard) {
      return QVariant();
   }
   return d->wizard->field(name);
}

void QWizardPage::registerField(const QString &name, QWidget *widget, const QString &property, const QString &changedSignal)
{
   Q_D(QWizardPage);

   QWizardField field(this, name, widget, property, changedSignal);

   if (d->wizard) {
      d->wizard->d_func()->addField(field);
   } else {
      d->pendingFields += field;
   }
}

QWizard *QWizardPage::wizard() const
{
   Q_D(const QWizardPage);
   return d->wizard;
}

void QWizard::_q_emitCustomButtonClicked()
{
   Q_D(QWizard);
   d->_q_emitCustomButtonClicked();
}

void QWizard::_q_updateButtonStates()
{
   Q_D(QWizard);
   d->_q_updateButtonStates();
}

void QWizard::_q_handleFieldObjectDestroyed(QObject *obj)
{
   Q_D(QWizard);
   d->_q_handleFieldObjectDestroyed(obj);
}

void QWizardPage::_q_changedSignal()
{
   Q_D(QWizardPage);
   d->_q_changedSignal();
}

void QWizardPage::_q_updateCachedCompleteState()
{
   Q_D(QWizardPage);
   d->_q_updateCachedCompleteState();
}

#endif // QT_NO_WIZARD
