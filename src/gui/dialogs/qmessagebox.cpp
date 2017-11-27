/***********************************************************************
*
* Copyright (c) 2012-2017 Barbara Geller
* Copyright (c) 2012-2017 Ansel Sermersheim
* Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
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
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#include <QtGui/qmessagebox.h>

#ifndef QT_NO_MESSAGEBOX

#include <qdialogbuttonbox.h>
#include <qlabel_p.h>
#include <qapplication_p.h>
#include <QtCore/qlist.h>
#include <QtCore/qdebug.h>
#include <QtGui/qstyle.h>
#include <QtGui/qstyleoption.h>
#include <QtGui/qgridlayout.h>
#include <QtGui/qdesktopwidget.h>
#include <QtGui/qpushbutton.h>
#include <QtGui/qaccessible.h>
#include <QtGui/qicon.h>
#include <QtGui/qtextdocument.h>
#include <QtGui/qapplication.h>
#include <QtGui/qtextedit.h>
#include <QtGui/qtextbrowser.h>
#include <QtGui/qmenu.h>
#include <qdialog_p.h>
#include <QtGui/qfont.h>
#include <QtGui/qfontmetrics.h>
#include <QtGui/qclipboard.h>

QT_BEGIN_NAMESPACE

enum Button { Old_Ok = 1, Old_Cancel = 2, Old_Yes = 3, Old_No = 4, Old_Abort = 5, Old_Retry = 6,
              Old_Ignore = 7, Old_YesAll = 8, Old_NoAll = 9, Old_ButtonMask = 0xFF,
              NewButtonMask = 0xFFFFFC00
            };

enum DetailButtonLabel { ShowLabel = 0, HideLabel = 1 };

void cs_require_version(int argc, char *argv[], const char *str)
{
   QString cur_version = QString(CS_VERSION_STR);
   QString req_version = QString(str);

   int current = (cur_version.section(QChar::fromLatin1('.'),0,0).toInt() << 16) +
      (cur_version.section(QChar::fromLatin1('.'),1,1).toInt() << 8) + cur_version.section(QChar::fromLatin1('.'),2,2).toInt();

   int required = (req_version.section(QChar::fromLatin1('.'),0,0).toInt() << 16) +
      (req_version.section(QChar::fromLatin1('.'),1,1).toInt() << 8) + req_version.section(QChar::fromLatin1('.'),2,2).toInt();

   if (current < required)   {

      if (! qApp) {
         new QApplication(argc, argv);
      }

      QString errMsg = QApplication::tr("%1 requires CopperSpice version %2\n\nFound CopperSpice version %3\n").
               arg(qAppName()).arg(req_version).arg(cur_version);

      QMessageBox box(QMessageBox::Critical, QApplication::tr("Incompatible Library"), errMsg, QMessageBox::Abort, 0);

      QIcon icon(QLatin1String(":/copperspice/qmessagebox/images/cslogo-64.png"));
      box.setWindowIcon(icon);
      box.exec();

      qFatal("%s", errMsg.toLatin1().data());
   }
}

#ifndef QT_NO_TEXTEDIT

class QMessageBoxDetailsText : public QWidget
{
 public:
   class TextEdit : public QTextEdit
   {
    public:
      TextEdit(QWidget *parent = nullptr) : QTextEdit(parent) { }

      void contextMenuEvent(QContextMenuEvent *e)  override {

#ifndef QT_NO_CONTEXTMENU
         QMenu *menu = createStandardContextMenu();
         menu->setAttribute(Qt::WA_DeleteOnClose);
         menu->popup(e->globalPos());
#else
         Q_UNUSED(e);
#endif
      }
   };

   QMessageBoxDetailsText(QWidget *parent = nullptr)
      : QWidget(parent) {
      QVBoxLayout *layout = new QVBoxLayout;
      layout->setMargin(0);
      QFrame *line = new QFrame(this);
      line->setFrameShape(QFrame::HLine);
      line->setFrameShadow(QFrame::Sunken);
      layout->addWidget(line);
      textEdit = new TextEdit();
      textEdit->setFixedHeight(100);
      textEdit->setFocusPolicy(Qt::NoFocus);
      textEdit->setReadOnly(true);
      layout->addWidget(textEdit);
      setLayout(layout);
   }

   void setText(const QString &text) {
      textEdit->setPlainText(text);
   }

   QString text() const {
      return textEdit->toPlainText();
   }

 private:
   TextEdit *textEdit;
};
#endif // QT_NO_TEXTEDIT

class DetailButton : public QPushButton
{
 public:
   DetailButton(QWidget *parent) : QPushButton(label(ShowLabel), parent) {
      setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
   }

   QString label(DetailButtonLabel label) const {
      return label == ShowLabel ? QMessageBox::tr("Show Details...") : QMessageBox::tr("Hide Details...");
   }

   void setLabel(DetailButtonLabel lbl) {
      setText(label(lbl));
   }

   QSize sizeHint() const override {
      ensurePolished();
      QStyleOptionButton opt;
      initStyleOption(&opt);
      const QFontMetrics fm = fontMetrics();
      opt.text = label(ShowLabel);
      QSize sz = fm.size(Qt::TextShowMnemonic, opt.text);
      QSize ret = style()->sizeFromContents(QStyle::CT_PushButton, &opt, sz, this).expandedTo(QApplication::globalStrut());
      opt.text = label(HideLabel);
      sz = fm.size(Qt::TextShowMnemonic, opt.text);

      ret = ret.expandedTo(style()->sizeFromContents(QStyle::CT_PushButton, &opt, sz, this).
                           expandedTo(QApplication::globalStrut()));
      return ret;
   }
};


class QMessageBoxPrivate : public QDialogPrivate
{
   Q_DECLARE_PUBLIC(QMessageBox)

 public:

   QMessageBoxPrivate() : escapeButton(0), defaultButton(0), clickedButton(0), detailsButton(0),
#ifndef QT_NO_TEXTEDIT
      detailsText(0),
#endif
      compatMode(false), autoAddOkButton(true),
      detectedEscapeButton(0), informativeLabel(0) { }

   void init(const QString &title = QString(), const QString &text = QString());
   void _q_buttonClicked(QAbstractButton *);

   QAbstractButton *findButton(int button0, int button1, int button2, int flags);
   void addOldButtons(int button0, int button1, int button2);

   QAbstractButton *abstractButtonForId(int id) const;
   int execReturnCode(QAbstractButton *button);

   void detectEscapeButton();
   void updateSize();
   int layoutMinimumWidth();
   void retranslateStrings();

   static int showOldMessageBox(QWidget *parent, QMessageBox::Icon icon, const QString &title, const QString &text,
                  int button0, int button1, int button2);

   static int showOldMessageBox(QWidget *parent, QMessageBox::Icon icon, const QString &title, const QString &text,
                  const QString &button0Text, const QString &button1Text, const QString &button2Text,
                  int defaultButtonNumber, int escapeButtonNumber);

   static QMessageBox::StandardButton showNewMessageBox(QWidget *parent,
                  QMessageBox::Icon icon, const QString &title, const QString &text,
                  QMessageBox::StandardButtons buttons, QMessageBox::StandardButton defaultButton);

   static QPixmap standardIcon(QMessageBox::Icon icon, QMessageBox *mb);

   QLabel *label;
   QMessageBox::Icon icon;
   QLabel *iconLabel;
   QDialogButtonBox *buttonBox;
   QList<QAbstractButton *> customButtonList;
   QAbstractButton *escapeButton;
   QPushButton *defaultButton;
   QAbstractButton *clickedButton;
   DetailButton *detailsButton;

#ifndef QT_NO_TEXTEDIT
   QMessageBoxDetailsText *detailsText;
#endif

   bool compatMode;
   bool autoAddOkButton;
   QAbstractButton *detectedEscapeButton;
   QLabel *informativeLabel;
   QPointer<QObject> receiverToDisconnectOnClose;
   QByteArray memberToDisconnectOnClose;
   QByteArray signalToDisconnectOnClose;
};

void QMessageBoxPrivate::init(const QString &title, const QString &text)
{
   Q_Q(QMessageBox);

   label = new QLabel;
   label->setObjectName(QLatin1String("qt_msgbox_label"));
   label->setTextInteractionFlags(Qt::TextInteractionFlags(q->style()->styleHint(QStyle::SH_MessageBox_TextInteractionFlags, 0, q)));

   label->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
   label->setOpenExternalLinks(true);

#if defined(Q_OS_MAC)
   label->setContentsMargins(16, 0, 0, 0);
#elif !defined(Q_WS_QWS)
   label->setContentsMargins(2, 0, 0, 0);
   label->setIndent(9);
#endif

   icon = QMessageBox::NoIcon;
   iconLabel = new QLabel;
   iconLabel->setObjectName(QLatin1String("qt_msgboxex_icon_label"));
   iconLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

   buttonBox = new QDialogButtonBox;
   buttonBox->setObjectName(QLatin1String("qt_msgbox_buttonbox"));
   buttonBox->setCenterButtons(q->style()->styleHint(QStyle::SH_MessageBox_CenterButtons, 0, q));
   QObject::connect(buttonBox, SIGNAL(clicked(QAbstractButton *)), q, SLOT(_q_buttonClicked(QAbstractButton *)));

   QGridLayout *grid = new QGridLayout;

#ifndef Q_OS_MAC
   const int preferredIconColumn = 0;
   const int preferredTextColumn = 1;

   grid->addWidget(iconLabel, 0, preferredIconColumn, 2, 1, Qt::AlignTop);
   grid->addWidget(label, 0, preferredTextColumn, 1, 1);

   // -- leave space for information label
   grid->addWidget(buttonBox, 2, 0, 1, 2);
#else
   grid->setMargin(0);
   grid->setVerticalSpacing(8);
   grid->setHorizontalSpacing(0);
   q->setContentsMargins(24, 15, 24, 20);
   grid->addWidget(iconLabel, 0, 0, 2, 1, Qt::AlignTop | Qt::AlignLeft);
   grid->addWidget(label, 0, 1, 1, 1);

   // -- leave space for information label
   grid->setRowStretch(1, 100);
   grid->setRowMinimumHeight(2, 6);
   grid->addWidget(buttonBox, 3, 1, 1, 1);
#endif

   grid->setSizeConstraint(QLayout::SetNoConstraint);
   q->setLayout(grid);

   if (!title.isEmpty() || !text.isEmpty()) {
      q->setWindowTitle(title);
      q->setText(text);
   }
   q->setModal(true);

#ifdef Q_OS_MAC
   QFont f = q->font();
   f.setBold(true);
   label->setFont(f);
#endif
   retranslateStrings();
}

int QMessageBoxPrivate::layoutMinimumWidth()
{
   layout->activate();
   return layout->totalMinimumSize().width();
}

void QMessageBoxPrivate::updateSize()
{
   Q_Q(QMessageBox);

   if (!q->isVisible()) {
      return;
   }

   QSize screenSize = QApplication::desktop()->availableGeometry(QCursor::pos()).size();

#if defined(Q_WS_QWS)
   // the width of the screen, less the window border.
   int hardLimit = screenSize.width() - (q->frameGeometry().width() - q->geometry().width());

#else
   int hardLimit = screenSize.width() - 480;       // used to have a qMin to set to 1000

   // on small screens allows the messagebox be the same size as the screen
   if (screenSize.width() <= 1024) {
      hardLimit = screenSize.width();
   }
#endif

#ifdef Q_OS_MAC
   int softLimit = qMin(screenSize.width() / 2, 420);
#elif defined(Q_WS_QWS)
   int softLimit = qMin(hardLimit, 500);
#else
   // note: ideally on windows, hard and soft limits but it breaks compat
   int softLimit = qMin(screenSize.width() / 2, 500);
#endif

   if (informativeLabel) {
      informativeLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
   }

   label->setWordWrap(false); // makes the label return min size
   int width = layoutMinimumWidth();

   if (width > softLimit) {
      label->setWordWrap(true);
      width = qMax(softLimit, layoutMinimumWidth());

      if (width > hardLimit) {
         label->d_func()->ensureTextControl();
         if (QTextControl *control = label->d_func()->control) {
            QTextOption opt = control->document()->defaultTextOption();
            opt.setWrapMode(QTextOption::WrapAnywhere);
            control->document()->setDefaultTextOption(opt);
         }
         width = hardLimit;
      }
   }

   if (informativeLabel) {
      label->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
      QSizePolicy policy(QSizePolicy::Minimum, QSizePolicy::Preferred);
      policy.setHeightForWidth(true);
      informativeLabel->setSizePolicy(policy);
      width = qMax(width, layoutMinimumWidth());

      if (width > hardLimit) { // longest word is really big, so wrap anywhere
         informativeLabel->d_func()->ensureTextControl();
         if (QTextControl *control = informativeLabel->d_func()->control) {
            QTextOption opt = control->document()->defaultTextOption();
            opt.setWrapMode(QTextOption::WrapAnywhere);
            control->document()->setDefaultTextOption(opt);
         }
         width = hardLimit;
      }
      policy.setHeightForWidth(label->wordWrap());
      label->setSizePolicy(policy);
   }

   QFontMetrics fm(QApplication::font("QWorkspaceTitleBar"));
   int windowTitleWidth = qMin(fm.width(q->windowTitle()) + 50, hardLimit);

   if (windowTitleWidth > width) {
      width = windowTitleWidth;
   }

   layout->activate();
   int height = (layout->hasHeightForWidth()) ? layout->totalHeightForWidth(width)
                : layout->totalMinimumSize().height();

   q->setFixedSize(width, height);
   QCoreApplication::removePostedEvents(q, QEvent::LayoutRequest);
}

static int oldButton(int button)
{
   switch (button & QMessageBox::ButtonMask) {
      case QMessageBox::Ok:
         return Old_Ok;
      case QMessageBox::Cancel:
         return Old_Cancel;
      case QMessageBox::Yes:
         return Old_Yes;
      case QMessageBox::No:
         return Old_No;
      case QMessageBox::Abort:
         return Old_Abort;
      case QMessageBox::Retry:
         return Old_Retry;
      case QMessageBox::Ignore:
         return Old_Ignore;
      case QMessageBox::YesToAll:
         return Old_YesAll;
      case QMessageBox::NoToAll:
         return Old_NoAll;
      default:
         return 0;
   }
}

int QMessageBoxPrivate::execReturnCode(QAbstractButton *button)
{
   int ret = buttonBox->standardButton(button);

   if (ret == QMessageBox::NoButton) {
      ret = customButtonList.indexOf(button); // if button == 0, correctly sets ret = -1

   } else if (compatMode) {
      ret = oldButton(ret);

   }

   return ret;
}

void QMessageBoxPrivate::_q_buttonClicked(QAbstractButton *button)
{
   Q_Q(QMessageBox);

#ifndef QT_NO_TEXTEDIT
   if (detailsButton && detailsText && button == detailsButton) {
      detailsButton->setLabel(detailsText->isHidden() ? HideLabel : ShowLabel);
      detailsText->setHidden(!detailsText->isHidden());
      updateSize();
   } else
#endif
   {
      clickedButton = button;
      q->done(execReturnCode(button)); // does not trigger closeEvent
      emit q->buttonClicked(button);

      if (receiverToDisconnectOnClose) {
         QObject::disconnect(q, signalToDisconnectOnClose.constData(), receiverToDisconnectOnClose,
                             memberToDisconnectOnClose.constData());

         receiverToDisconnectOnClose = 0;
      }
      signalToDisconnectOnClose.clear();
      memberToDisconnectOnClose.clear();
   }
}

void QMessageBox::_q_buttonClicked(QAbstractButton *un_named_arg1)
{
   Q_D(QMessageBox);
   d->_q_buttonClicked(un_named_arg1);
}

QMessageBox::QMessageBox(QWidget *parent)
                  : QDialog(*new QMessageBoxPrivate, parent, Qt::MSWindowsFixedSizeDialogHint | Qt::WindowTitleHint |
                  Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint)
{
   Q_D(QMessageBox);
   d->init();
}

QMessageBox::QMessageBox(Icon icon, const QString &title, const QString &text, StandardButtons buttons,
                  QWidget *parent, Qt::WindowFlags f)
                  : QDialog(*new QMessageBoxPrivate, parent, f | Qt::MSWindowsFixedSizeDialogHint | Qt::WindowTitleHint |
                  Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint)
{
   Q_D(QMessageBox);

   d->init(title, text);
   setIcon(icon);

   if (buttons != NoButton) {
      setStandardButtons(buttons);
   }
}

QMessageBox::~QMessageBox()
{
}

void QMessageBox::addButton(QAbstractButton *button, ButtonRole role)
{
   Q_D(QMessageBox);
   if (!button) {
      return;
   }
   removeButton(button);
   d->buttonBox->addButton(button, (QDialogButtonBox::ButtonRole)role);
   d->customButtonList.append(button);
   d->autoAddOkButton = false;
}


QPushButton *QMessageBox::addButton(const QString &text, ButtonRole role)
{
   Q_D(QMessageBox);
   QPushButton *pushButton = new QPushButton(text);
   addButton(pushButton, role);
   d->updateSize();
   return pushButton;
}


QPushButton *QMessageBox::addButton(StandardButton button)
{
   Q_D(QMessageBox);
   QPushButton *pushButton = d->buttonBox->addButton((QDialogButtonBox::StandardButton)button);
   if (pushButton) {
      d->autoAddOkButton = false;
   }
   return pushButton;
}

void QMessageBox::removeButton(QAbstractButton *button)
{
   Q_D(QMessageBox);
   d->customButtonList.removeAll(button);
   if (d->escapeButton == button) {
      d->escapeButton = 0;
   }
   if (d->defaultButton == button) {
      d->defaultButton = 0;
   }
   d->buttonBox->removeButton(button);
   d->updateSize();
}

void QMessageBox::setStandardButtons(StandardButtons buttons)
{
   Q_D(QMessageBox);
   d->buttonBox->setStandardButtons(QDialogButtonBox::StandardButtons(int(buttons)));

   QList<QAbstractButton *> buttonList = d->buttonBox->buttons();
   if (!buttonList.contains(d->escapeButton)) {
      d->escapeButton = 0;
   }
   if (!buttonList.contains(d->defaultButton)) {
      d->defaultButton = 0;
   }
   d->autoAddOkButton = false;
   d->updateSize();
}

QMessageBox::StandardButtons QMessageBox::standardButtons() const
{
   Q_D(const QMessageBox);
   return QMessageBox::StandardButtons(int(d->buttonBox->standardButtons()));
}


QMessageBox::StandardButton QMessageBox::standardButton(QAbstractButton *button) const
{
   Q_D(const QMessageBox);
   return (QMessageBox::StandardButton)d->buttonBox->standardButton(button);
}

QAbstractButton *QMessageBox::button(StandardButton which) const
{
   Q_D(const QMessageBox);
   return d->buttonBox->button(QDialogButtonBox::StandardButton(which));
}


QAbstractButton *QMessageBox::escapeButton() const
{
   Q_D(const QMessageBox);
   return d->escapeButton;
}

void QMessageBox::setEscapeButton(QAbstractButton *button)
{
   Q_D(QMessageBox);
   if (d->buttonBox->buttons().contains(button)) {
      d->escapeButton = button;
   }
}

void QMessageBox::setEscapeButton(QMessageBox::StandardButton button)
{
   Q_D(QMessageBox);
   setEscapeButton(d->buttonBox->button(QDialogButtonBox::StandardButton(button)));
}

void QMessageBoxPrivate::detectEscapeButton()
{
   if (escapeButton) { // escape button explicitly set
      detectedEscapeButton = escapeButton;
      return;
   }

   // Cancel button automatically becomes escape button
   detectedEscapeButton = buttonBox->button(QDialogButtonBox::Cancel);
   if (detectedEscapeButton) {
      return;
   }

   // If there is only one button, make it the escape button
   const QList<QAbstractButton *> buttons = buttonBox->buttons();
   if (buttons.count() == 1) {
      detectedEscapeButton = buttons.first();
      return;
   }

   // if the message box has one RejectRole button, make it the escape button
   for (int i = 0; i < buttons.count(); i++) {
      if (buttonBox->buttonRole(buttons.at(i)) == QDialogButtonBox::RejectRole) {
         if (detectedEscapeButton) { // already detected!
            detectedEscapeButton = 0;
            break;
         }
         detectedEscapeButton = buttons.at(i);
      }
   }
   if (detectedEscapeButton) {
      return;
   }

   // if the message box has one NoRole button, make it the escape button
   for (int i = 0; i < buttons.count(); i++) {
      if (buttonBox->buttonRole(buttons.at(i)) == QDialogButtonBox::NoRole) {
         if (detectedEscapeButton) { // already detected!
            detectedEscapeButton = 0;
            break;
         }
         detectedEscapeButton = buttons.at(i);
      }
   }
}

QAbstractButton *QMessageBox::clickedButton() const
{
   Q_D(const QMessageBox);
   return d->clickedButton;
}

QPushButton *QMessageBox::defaultButton() const
{
   Q_D(const QMessageBox);
   return d->defaultButton;
}

/*!
    \since 4.2

    Sets the message box's \l{QPushButton::setDefault()}{default button}
    to \a button.

    \sa addButton(), QPushButton::setDefault()
*/
void QMessageBox::setDefaultButton(QPushButton *button)
{
   Q_D(QMessageBox);
   if (!d->buttonBox->buttons().contains(button)) {
      return;
   }
   d->defaultButton = button;
   button->setDefault(true);
   button->setFocus();
}

void QMessageBox::setDefaultButton(QMessageBox::StandardButton button)
{
   Q_D(QMessageBox);
   setDefaultButton(d->buttonBox->button(QDialogButtonBox::StandardButton(button)));
}

QString QMessageBox::text() const
{
   Q_D(const QMessageBox);
   return d->label->text();
}

void QMessageBox::setText(const QString &text)
{
   Q_D(QMessageBox);

   d->label->setText(text);

   d->label->setWordWrap(d->label->textFormat() == Qt::RichText ||
         (d->label->textFormat() == Qt::AutoText && Qt::mightBeRichText(text)));

   d->updateSize();
}

QMessageBox::Icon QMessageBox::icon() const
{
   Q_D(const QMessageBox);

   return d->icon;
}

void QMessageBox::setIcon(Icon icon)
{
   Q_D(QMessageBox);

   setIconPixmap(QMessageBoxPrivate::standardIcon((QMessageBox::Icon)icon, this));
   d->icon = icon;
}

QPixmap QMessageBox::iconPixmap() const
{
   Q_D(const QMessageBox);

   if (d->iconLabel && d->iconLabel->pixmap()) {
      return *d->iconLabel->pixmap();
   }
   return QPixmap();
}

void QMessageBox::setIconPixmap(const QPixmap &pixmap)
{
   Q_D(QMessageBox);

   d->iconLabel->setPixmap(pixmap);
   d->updateSize();
   d->icon = NoIcon;
}

Qt::TextFormat QMessageBox::textFormat() const
{
   Q_D(const QMessageBox);
   return d->label->textFormat();
}

void QMessageBox::setTextFormat(Qt::TextFormat format)
{
   Q_D(QMessageBox);
   d->label->setTextFormat(format);
   d->label->setWordWrap(format == Qt::RichText ||
                         (format == Qt::AutoText && Qt::mightBeRichText(d->label->text())));
   d->updateSize();
}

/*!
    \reimp
*/
bool QMessageBox::event(QEvent *e)
{
   bool result = QDialog::event(e);

   switch (e->type()) {
      case QEvent::LayoutRequest:
         d_func()->updateSize();
         break;
      case QEvent::LanguageChange:
         d_func()->retranslateStrings();
         break;
      default:
         break;
   }
   return result;
}

/*!
    \reimp
*/
void QMessageBox::resizeEvent(QResizeEvent *event)
{
   QDialog::resizeEvent(event);
}

/*!
    \reimp
*/
void QMessageBox::closeEvent(QCloseEvent *e)
{
   Q_D(QMessageBox);

   if (!d->detectedEscapeButton) {
      e->ignore();
      return;
   }

   QDialog::closeEvent(e);
   d->clickedButton = d->detectedEscapeButton;
   setResult(d->execReturnCode(d->detectedEscapeButton));
}

/*!
    \reimp
*/
void QMessageBox::changeEvent(QEvent *ev)
{
   Q_D(QMessageBox);

   switch (ev->type()) {
      case QEvent::StyleChange: {
         if (d->icon != NoIcon) {
            setIcon(d->icon);
         }
         Qt::TextInteractionFlags flags(style()->styleHint(QStyle::SH_MessageBox_TextInteractionFlags, 0, this));
         d->label->setTextInteractionFlags(flags);
         d->buttonBox->setCenterButtons(style()->styleHint(QStyle::SH_MessageBox_CenterButtons, 0, this));

         if (d->informativeLabel) {
            d->informativeLabel->setTextInteractionFlags(flags);
         }
         // intentional fall through
      }

      case QEvent::FontChange:
      case QEvent::ApplicationFontChange:
#ifdef Q_OS_MAC
      {
         QFont f = font();
         f.setBold(true);
         d->label->setFont(f);
      }
#endif
      default:
         break;
   }
   QDialog::changeEvent(ev);
}

/*!
    \reimp
*/
void QMessageBox::keyPressEvent(QKeyEvent *e)
{
   Q_D(QMessageBox);

   if (e->key() == Qt::Key_Escape

#ifdef Q_OS_MAC
         || (e->modifiers() == Qt::ControlModifier && e->key() == Qt::Key_Period)
#endif

      ) {
      if (d->detectedEscapeButton) {

#ifdef Q_OS_MAC
         d->detectedEscapeButton->animateClick();
#else
         d->detectedEscapeButton->click();
#endif
      }
      return;
   }

#if defined (Q_OS_WIN) && ! defined(QT_NO_CLIPBOARD) && ! defined(QT_NO_SHORTCUT)
   if (e == QKeySequence::Copy) {
      QString separator = QString::fromLatin1("---------------------------\n");
      QString textToCopy = separator;
      separator.prepend(QLatin1Char('\n'));
      textToCopy += windowTitle() + separator; // title
      textToCopy += d->label->text() + separator; // text

      if (d->informativeLabel) {
         textToCopy += d->informativeLabel->text() + separator;
      }

      QString buttonTexts;
      QList<QAbstractButton *> buttons = d->buttonBox->buttons();
      for (int i = 0; i < buttons.count(); i++) {
         buttonTexts += buttons[i]->text() + QLatin1String("   ");
      }
      textToCopy += buttonTexts + separator;

      QApplication::clipboard()->setText(textToCopy);
      return;
   }
#endif

#ifndef QT_NO_SHORTCUT
   if (! (e->modifiers() & Qt::AltModifier)) {
      int key = e->key() & ~(Qt::MODIFIER_MASK | Qt::UNICODE_ACCEL);

      if (key) {
         const QList<QAbstractButton *> buttons = d->buttonBox->buttons();

         for (int i = 0; i < buttons.count(); ++i) {
            QAbstractButton *pb   = buttons.at(i);
            QKeySequence shortcut = pb->shortcut();

            if (! shortcut.isEmpty() && key == (shortcut[0] & ~(Qt::MODIFIER_MASK | Qt::UNICODE_ACCEL))) {
               pb->animateClick();
               return;
            }
         }
      }
   }

#endif
   QDialog::keyPressEvent(e);
}

void QMessageBox::open(QObject *receiver, const char *member)
{
   Q_D(QMessageBox);

   const char *signal = member && strchr(member, '*') ? "buttonClicked(QAbstractButton*)" : "finished(int)";
   connect(this, signal, receiver, member);

   d->signalToDisconnectOnClose = signal;
   d->receiverToDisconnectOnClose = receiver;
   d->memberToDisconnectOnClose = member;
   QDialog::open();
}

QList<QAbstractButton *> QMessageBox::buttons() const
{
   Q_D(const QMessageBox);
   return d->buttonBox->buttons();
}

QMessageBox::ButtonRole QMessageBox::buttonRole(QAbstractButton *button) const
{
   Q_D(const QMessageBox);
   return QMessageBox::ButtonRole(d->buttonBox->buttonRole(button));
}

void QMessageBox::showEvent(QShowEvent *e)
{
   Q_D(QMessageBox);

   if (d->autoAddOkButton) {
      addButton(Ok);
   }

   if (d->detailsButton) {
      addButton(d->detailsButton, QMessageBox::ActionRole);
   }

   d->detectEscapeButton();
   d->updateSize();

#ifndef QT_NO_ACCESSIBILITY
   QAccessible::updateAccessibility(this, 0, QAccessible::Alert);
#endif

#ifdef Q_OS_WIN
   HMENU systemMenu = GetSystemMenu((HWND)winId(), FALSE);
   if (!d->detectedEscapeButton) {
      EnableMenuItem(systemMenu, SC_CLOSE, MF_BYCOMMAND | MF_GRAYED);
   } else {
      EnableMenuItem(systemMenu, SC_CLOSE, MF_BYCOMMAND | MF_ENABLED);
   }
#endif

   QDialog::showEvent(e);
}

static QMessageBox::StandardButton showNewMessageBox(QWidget *parent, QMessageBox::Icon icon, const QString &title,
      const QString &text, QMessageBox::StandardButtons buttons, QMessageBox::StandardButton defaultButton)
{
   // necessary for source compatibility with Qt 4.0 and 4.1
   // handles (Yes, No) and (Yes|Default, No)

   if (defaultButton && !(buttons & defaultButton)) {
      return (QMessageBox::StandardButton)
             QMessageBoxPrivate::showOldMessageBox(parent, icon, title, text, int(buttons), int(defaultButton), 0);
   }

   QMessageBox msgBox(icon, title, text, QMessageBox::NoButton, parent);
   QDialogButtonBox *buttonBox = msgBox.findChild<QDialogButtonBox *>();
   Q_ASSERT(buttonBox != 0);

   uint mask = QMessageBox::FirstButton;
   while (mask <= QMessageBox::LastButton) {
      uint sb = buttons & mask;
      mask <<= 1;

      if (!sb) {
         continue;
      }

      QPushButton *button = msgBox.addButton((QMessageBox::StandardButton)sb);
      // Choose the first accept role as the default
      if (msgBox.defaultButton()) {
         continue;
      }

      if ((defaultButton == QMessageBox::NoButton && buttonBox->buttonRole(button) == QDialogButtonBox::AcceptRole)
            || (defaultButton != QMessageBox::NoButton && sb == uint(defaultButton))) {
         msgBox.setDefaultButton(button);
      }
   }

   if (msgBox.exec() == -1) {
      return QMessageBox::Cancel;
   }

   return msgBox.standardButton(msgBox.clickedButton());
}

QMessageBox::StandardButton QMessageBox::information(QWidget *parent, const QString &title,
      const QString &text, StandardButtons buttons, StandardButton defaultButton)
{
   return showNewMessageBox(parent, Information, title, text, buttons, defaultButton);
}

QMessageBox::StandardButton QMessageBox::question(QWidget *parent, const QString &title,
      const QString &text, StandardButtons buttons, StandardButton defaultButton)
{
   return showNewMessageBox(parent, Question, title, text, buttons, defaultButton);
}

QMessageBox::StandardButton QMessageBox::warning(QWidget *parent, const QString &title,
      const QString &text, StandardButtons buttons,
      StandardButton defaultButton)
{
   return showNewMessageBox(parent, Warning, title, text, buttons, defaultButton);
}

QMessageBox::StandardButton QMessageBox::critical(QWidget *parent, const QString &title,
      const QString &text, StandardButtons buttons, StandardButton defaultButton)
{
   return showNewMessageBox(parent, Critical, title, text, buttons, defaultButton);
}

void QMessageBox::about(QWidget *parent, const QString &title, const QString &text)
{

#ifdef Q_OS_MAC
   static QPointer<QMessageBox> oldMsgBox;

   if (oldMsgBox && oldMsgBox->text() == text) {
      oldMsgBox->show();
      oldMsgBox->raise();
      oldMsgBox->activateWindow();
      return;
   }
#endif

   StandardButtons buttons = QMessageBox::Ok;
   QMessageBox *msgBox = new QMessageBox(QMessageBox::Information, title, text, buttons, parent

#ifdef Q_OS_MAC
      , Qt::WindowTitleHint | Qt::WindowSystemMenuHint);
#else
      );
#endif

   msgBox->setAttribute(Qt::WA_DeleteOnClose);
   QIcon icon = msgBox->windowIcon();
   QSize size = icon.actualSize(QSize(64, 64));
   msgBox->setIconPixmap(icon.pixmap(size));

   // should perhaps be a style hint
#ifdef Q_OS_MAC
   oldMsgBox = msgBox;

   msgBox->d_func()->buttonBox->setCenterButtons(true);
   msgBox->show();

#else
   msgBox->exec();

#endif
}

void QMessageBox::aboutCs(QWidget *parent)
{
   QDialog *aboutBox = new QDialog(parent);
   aboutBox->setAttribute(Qt::WA_DeleteOnClose);
   aboutBox->setWindowTitle(tr("About CopperSpice"));

   QIcon icon(QLatin1String(":/copperspice/qmessagebox/images/cslogo-64.png"));
   aboutBox->setWindowIcon(icon);

   QLabel *msg1 = new QLabel;
   msg1->setText(tr("CopperSpice libraries Version %1").arg(QLatin1String(CS_VERSION_STR)));

   QFont font = msg1->font();
   font.setWeight(QFont::Bold);
   font.setPointSize(10);
   msg1->setFont(font);

   QLabel *msg2 = new QLabel;
   msg2->setText(tr("CopperSpice is a C++ toolkit for cross platform applications on X11, Windows, and OS X\n"
                    "CopperSpice is licensed under the GNU LGPL version 2.1"));

   font = msg2->font();
   font.setPointSize(10);
   msg2->setFont(font);

   QLabel *msg3 = new QLabel;
   msg3->setText(tr("Copyright (C) 2012-2017 Ansel Sermersheim & Barbara Geller\n"
                    "Copyright (C) 2012-2014 Digia Plc and/or its subsidiary(-ies)\n"
                    "Copyright (C) 2008-2012 Nokia Corporation and/or its subsidiary(-ies)"));

   font = msg3->font();
   font.setPointSize(10);
   msg3->setFont(font);

   QLabel *csImage = 0;
   QPixmap pm(QLatin1String(":/copperspice/qmessagebox/images/cslogo-64.png"));

   if (! pm.isNull()) {
      csImage = new QLabel;
      csImage->setPixmap(pm);
   }

   QGridLayout *layout = new QGridLayout;
   if (csImage)  {
      layout->addWidget(csImage, 0, 0, 3, 1);
   }
   layout->addWidget(msg1, 0, 1);
   layout->addWidget(msg2, 1, 1);
   layout->addWidget(msg3, 2, 1);
   layout->setSpacing(15);
   layout->setContentsMargins(9, 9, 15, 20);
   layout->setSizeConstraint(QLayout::SetFixedSize);

   aboutBox->setLayout(layout);
   aboutBox->exec();
}

void QMessageBox::aboutQt(QWidget *parent)
{
   QMessageBox::aboutCs(parent);
}

QSize QMessageBox::sizeHint() const
{
   // ### Qt5 remove
   return QDialog::sizeHint();
}

static QMessageBox::StandardButton newButton(int button)
{
   if (button == QMessageBox::NoButton || (button & NewButtonMask)) {
      return QMessageBox::StandardButton(button & QMessageBox::ButtonMask);
   }

   return QMessageBox::NoButton;
}

static bool detectedCompat(int button0, int button1, int button2)
{
   if (button0 != 0 && !(button0 & NewButtonMask)) {
      return true;
   }

   if (button1 != 0 && !(button1 & NewButtonMask)) {
      return true;
   }

   if (button2 != 0 && !(button2 & NewButtonMask)) {
      return true;
   }

   return false;
}

QAbstractButton *QMessageBoxPrivate::findButton(int button0, int button1, int button2, int flags)
{
   Q_Q(QMessageBox);
   int button = 0;

   if (button0 & flags) {
      button = button0;

   } else if (button1 & flags) {
      button = button1;

   } else if (button2 & flags) {
      button = button2;
   }

   return q->button(newButton(button));
}

void QMessageBoxPrivate::addOldButtons(int button0, int button1, int button2)
{
   Q_Q(QMessageBox);
   q->addButton(newButton(button0));
   q->addButton(newButton(button1));
   q->addButton(newButton(button2));

   q->setDefaultButton(static_cast<QPushButton *>(findButton(button0, button1, button2, QMessageBox::Default)));
   q->setEscapeButton(findButton(button0, button1, button2, QMessageBox::Escape));

   compatMode = detectedCompat(button0, button1, button2);
}

QAbstractButton *QMessageBoxPrivate::abstractButtonForId(int id) const
{
   Q_Q(const QMessageBox);
   QAbstractButton *result = customButtonList.value(id);

   if (result) {
      return result;
   }

   if (id & QMessageBox::FlagMask) {
      // for compatibility with Qt 4.0/4.1 (even if it is silly) Qt5
      return 0;
   }

   return q->button(newButton(id));
}

int QMessageBoxPrivate::showOldMessageBox(QWidget *parent, QMessageBox::Icon icon,
      const QString &title, const QString &text, int button0, int button1, int button2)
{
   QMessageBox messageBox(icon, title, text, QMessageBox::NoButton, parent);
   messageBox.d_func()->addOldButtons(button0, button1, button2);
   return messageBox.exec();
}

int QMessageBoxPrivate::showOldMessageBox(QWidget *parent, QMessageBox::Icon icon,
      const QString &title, const QString &text, const QString &button0Text,
      const QString &button1Text, const QString &button2Text, int defaultButtonNumber,
      int escapeButtonNumber)
{
   QMessageBox messageBox(icon, title, text, QMessageBox::NoButton, parent);
   QString myButton0Text = button0Text;

   if (myButton0Text.isEmpty()) {
      myButton0Text = QDialogButtonBox::tr("OK");
   }

   messageBox.addButton(myButton0Text, QMessageBox::ActionRole);
   if (!button1Text.isEmpty()) {
      messageBox.addButton(button1Text, QMessageBox::ActionRole);
   }

   if (!button2Text.isEmpty()) {
      messageBox.addButton(button2Text, QMessageBox::ActionRole);
   }

   const QList<QAbstractButton *> &buttonList = messageBox.d_func()->customButtonList;
   messageBox.setDefaultButton(static_cast<QPushButton *>(buttonList.value(defaultButtonNumber)));
   messageBox.setEscapeButton(buttonList.value(escapeButtonNumber));

   return messageBox.exec();
}

void QMessageBoxPrivate::retranslateStrings()
{
#ifndef QT_NO_TEXTEDIT
   if (detailsButton) {
      detailsButton->setLabel(detailsText->isHidden() ? ShowLabel : HideLabel);
   }
#endif
}

// obsolete
int QMessageBox::information(QWidget *parent, const QString &title, const QString &text,
                  int button0, int button1, int button2)
{
   return QMessageBoxPrivate::showOldMessageBox(parent, Information, title, text,
                  button0, button1, button2);
}

// obsolete
int QMessageBox::information(QWidget *parent, const QString &title, const QString &text,
                  const QString &button0Text, const QString &button1Text,
                  const QString &button2Text, int defaultButtonNumber, int escapeButtonNumber)
{
   return QMessageBoxPrivate::showOldMessageBox(parent, Information, title, text,
                  button0Text, button1Text, button2Text, defaultButtonNumber, escapeButtonNumber);
}

// obsolete
int QMessageBox::question(QWidget *parent, const QString &title, const QString &text,
                  int button0, int button1, int button2)
{
   return QMessageBoxPrivate::showOldMessageBox(parent, Question, title, text,
          button0, button1, button2);
}

// obsolete
int QMessageBox::question(QWidget *parent, const QString &title, const QString &text,
                  const QString &button0Text, const QString &button1Text,
                  const QString &button2Text, int defaultButtonNumber, int escapeButtonNumber)
{
   return QMessageBoxPrivate::showOldMessageBox(parent, Question, title, text,
          button0Text, button1Text, button2Text, defaultButtonNumber, escapeButtonNumber);
}

// obsolete
int QMessageBox::warning(QWidget *parent, const QString &title, const QString &text,
                  int button0, int button1, int button2)
{
   return QMessageBoxPrivate::showOldMessageBox(parent, Warning, title, text,
          button0, button1, button2);
}

// obsolete
int QMessageBox::warning(QWidget *parent, const QString &title, const QString &text,
                         const QString &button0Text, const QString &button1Text, const QString &button2Text,
                         int defaultButtonNumber, int escapeButtonNumber)
{
   return QMessageBoxPrivate::showOldMessageBox(parent, Warning, title, text,
          button0Text, button1Text, button2Text, defaultButtonNumber, escapeButtonNumber);
}

// obsolete
int QMessageBox::critical(QWidget *parent, const QString &title, const QString &text,
                          int button0, int button1, int button2)
{
   return QMessageBoxPrivate::showOldMessageBox(parent, Critical, title, text, button0, button1, button2);
}

// obsolete
int QMessageBox::critical(QWidget *parent, const QString &title, const QString &text,
                          const QString &button0Text, const QString &button1Text,
                          const QString &button2Text, int defaultButtonNumber, int escapeButtonNumber)
{
   return QMessageBoxPrivate::showOldMessageBox(parent, Critical, title, text,
          button0Text, button1Text, button2Text, defaultButtonNumber, escapeButtonNumber);
}

#ifndef QT_NO_TEXTEDIT

QString QMessageBox::detailedText() const
{
   Q_D(const QMessageBox);
   return d->detailsText ? d->detailsText->text() : QString();
}

void QMessageBox::setDetailedText(const QString &text)
{
   Q_D(QMessageBox);

   if (text.isEmpty()) {
      delete d->detailsText;
      d->detailsText = 0;
      removeButton(d->detailsButton);
      delete d->detailsButton;
      d->detailsButton = 0;
      return;
   }

   if (!d->detailsText) {
      d->detailsText = new QMessageBoxDetailsText(this);
      QGridLayout *grid = qobject_cast<QGridLayout *>(layout());
      if (grid) {
         grid->addWidget(d->detailsText, grid->rowCount(), 0, 1, grid->columnCount());
      }
      d->detailsText->hide();
   }
   if (!d->detailsButton) {
      d->detailsButton = new DetailButton(this);
   }
   d->detailsText->setText(text);
}
#endif // QT_NO_TEXTEDIT

/*!
  \property QMessageBox::informativeText

  \brief the informative text that provides a fuller description for
  the message

  \since 4.2

  Infromative text can be used to expand upon the text() to give more
  information to the user. On the Mac, this text appears in small
  system font below the text().  On other platforms, it is simply
  appended to the existing text.

  By default, this property contains an empty string.

  \sa QMessageBox::text, QMessageBox::detailedText
*/
QString QMessageBox::informativeText() const
{
   Q_D(const QMessageBox);
   return d->informativeLabel ? d->informativeLabel->text() : QString();
}

void QMessageBox::setInformativeText(const QString &text)
{
   Q_D(QMessageBox);
   if (text.isEmpty()) {
      layout()->removeWidget(d->informativeLabel);
      delete d->informativeLabel;
      d->informativeLabel = 0;
#ifndef Q_OS_MAC
      d->label->setContentsMargins(2, 0, 0, 0);
#endif
      d->updateSize();
      return;
   }

   if (!d->informativeLabel) {
      QLabel *label = new QLabel(this);
      label->setObjectName(QLatin1String("qt_msgbox_informativelabel"));
      label->setTextInteractionFlags(Qt::TextInteractionFlags(style()->styleHint(QStyle::SH_MessageBox_TextInteractionFlags,
                                     0, this)));
      label->setAlignment(Qt::AlignTop | Qt::AlignLeft);
      label->setOpenExternalLinks(true);
      label->setWordWrap(true);
#ifndef Q_OS_MAC
      d->label->setContentsMargins(2, 0, 0, 0);
      label->setContentsMargins(2, 0, 0, 6);
      label->setIndent(9);
#else
      label->setContentsMargins(16, 0, 0, 0);
      // apply a smaller font the information label on the mac
      label->setFont(qt_app_fonts_hash()->value("QTipLabel"));
#endif
      label->setWordWrap(true);
      QGridLayout *grid = static_cast<QGridLayout *>(layout());

      grid->addWidget(label, 1, 1, 1, 1);
      d->informativeLabel = label;
   }

   d->informativeLabel->setText(text);
   d->updateSize();
}

void QMessageBox::setWindowTitle(const QString &title)
{
   // Message boxes on the mac do not have a title
#ifndef Q_OS_MAC
   QDialog::setWindowTitle(title);
#else
   Q_UNUSED(title);
#endif
}


/*!
    \since 4.2

    This function shadows QWidget::setWindowModality().

    Sets the modality of the message box to \a windowModality.

    On Mac OS X, if the modality is set to Qt::WindowModal and the message box
    has a parent, then the message box will be a Qt::Sheet, otherwise the
    message box will be a standard dialog.
*/
void QMessageBox::setWindowModality(Qt::WindowModality windowModality)
{
   QDialog::setWindowModality(windowModality);

   if (parentWidget() && windowModality == Qt::WindowModal) {
      setParent(parentWidget(), Qt::Sheet);
   } else {
      setParent(parentWidget(), Qt::Dialog);
   }
   setDefaultButton(d_func()->defaultButton);
}

QPixmap QMessageBoxPrivate::standardIcon(QMessageBox::Icon icon, QMessageBox *mb)
{
   QStyle *style = mb ? mb->style() : QApplication::style();
   int iconSize = style->pixelMetric(QStyle::PM_MessageBoxIconSize, 0, mb);

   QIcon tmpIcon;

   switch (icon) {
      case QMessageBox::Information:
         tmpIcon = style->standardIcon(QStyle::SP_MessageBoxInformation, 0, mb);
         break;
      case QMessageBox::Warning:
         tmpIcon = style->standardIcon(QStyle::SP_MessageBoxWarning, 0, mb);
         break;
      case QMessageBox::Critical:
         tmpIcon = style->standardIcon(QStyle::SP_MessageBoxCritical, 0, mb);
         break;
      case QMessageBox::Question:
         tmpIcon = style->standardIcon(QStyle::SP_MessageBoxQuestion, 0, mb);
      default:
         break;
   }

   if (!tmpIcon.isNull()) {
      return tmpIcon.pixmap(iconSize, iconSize);
   }
   return QPixmap();
}

/*!
    \obsolete

    Returns the pixmap used for a standard icon. This allows the
    pixmaps to be used in more complex message boxes. \a icon
    specifies the required icon, e.g. QMessageBox::Question,
    QMessageBox::Information, QMessageBox::Warning or
    QMessageBox::Critical.

    Call QStyle::standardIcon() with QStyle::SP_MessageBoxInformation etc.
    instead.
*/

QPixmap QMessageBox::standardIcon(Icon icon)
{
   return QMessageBoxPrivate::standardIcon(icon, 0);
}

QT_END_NAMESPACE

#endif // QT_NO_MESSAGEBOX
