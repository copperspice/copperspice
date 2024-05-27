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

#include <qmessagebox.h>

#ifndef QT_NO_MESSAGEBOX

#include <qdialogbuttonbox.h>
#include <qlist.h>
#include <qdebug.h>
#include <qstyle.h>
#include <qstyleoption.h>
#include <qgridlayout.h>
#include <qdesktopwidget.h>
#include <qpushbutton.h>
#include <qcheckbox.h>
#include <qaccessible.h>
#include <qfont.h>
#include <qfontmetrics.h>
#include <qclipboard.h>
#include <qicon.h>
#include <qtextdocument.h>
#include <qapplication.h>
#include <qtextedit.h>
#include <qtextbrowser.h>
#include <qmenu.h>

#include <qlabel_p.h>
#include <qapplication_p.h>
#include <qdialog_p.h>

#ifdef Q_OS_WIN
#include <qt_windows.h>
#include <qplatform_nativeinterface.h>
#endif

#if defined(Q_OS_WIN)
HMENU qt_getWindowsSystemMenu(const QWidget *w)
{
   if (QWindow *window = QApplicationPrivate::windowForWidget(w))
      if (void *handle = QGuiApplication::platformNativeInterface()->nativeResourceForWindow("handle", window)) {
         return GetSystemMenu(reinterpret_cast<HWND>(handle), false);
      }
   return nullptr;
}
#endif

enum Button {
   Old_Ok         = 1,
   Old_Cancel     = 2,
   Old_Yes        = 3,
   Old_No         = 4,
   Old_Abort      = 5,
   Old_Retry      = 6,
   Old_Ignore     = 7,
   Old_YesAll     = 8,
   Old_NoAll      = 9,
   Old_ButtonMask = 0xFF,
   NewButtonMask  = 0xFFFFFC00
};

enum DetailButtonLabel {
   ShowLabel = 0,
   HideLabel = 1
};

void cs_require_version(int argc, char *argv[], const char *str)
{
   QString cur_version = QString(CS_VERSION_STR);
   QString req_version = QString::fromLatin1(str);

   int current = (cur_version.section('.', 0, 0).toInteger<int>() << 16) +
      (cur_version.section('.', 1, 1).toInteger<int>() << 8) + cur_version.section('.', 2, 2).toInteger<int>();

   int required = (req_version.section('.', 0, 0).toInteger<int>() << 16) +
      (req_version.section('.', 1, 1).toInteger<int>() << 8) + req_version.section('.', 2, 2).toInteger<int>();

   if (current < required)   {

      if (! qApp) {
         new QApplication(argc, argv);
      }

      QString errMsg = QApplication::tr("%1 requires CopperSpice version %2\n\nFound CopperSpice version %3\n")
               .formatArg(qAppName()).formatArg(req_version).formatArg(cur_version);

      QMessageBox box(QMessageBox::Critical, QApplication::tr("Incompatible Library"),
               errMsg, QMessageBox::Abort, nullptr);

      QIcon icon(":/copperspice/dialogs/images/cslogo-64.png");
      box.setWindowIcon(icon);
      box.exec();

      qFatal("%s", csPrintable(errMsg));
   }
}

#ifndef QT_NO_TEXTEDIT

class QMessageBoxDetailsText : public QWidget
{
   GUI_CS_OBJECT(QMessageBoxDetailsText)

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
         (void) e;
#endif
      }
   };

   QMessageBoxDetailsText(QWidget *parent = nullptr)
      : QWidget(parent), copyAvailable(false) {
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

      connect(textEdit, &TextEdit::copyAvailable, this, &QMessageBoxDetailsText::textCopyAvailable);
   }

   void setText(const QString &text) {
      textEdit->setPlainText(text);
   }

   QString text() const {
      return textEdit->toPlainText();
   }

   bool copy() {
#ifdef QT_NO_CLIPBOARD
      return false;
#else
      if (! copyAvailable) {
         return false;
      }

      textEdit->copy();
      return true;
#endif
   }

   void selectAll() {
      textEdit->selectAll();
   }

 private:
   GUI_CS_SLOT_1(Private, void textCopyAvailable(bool status) { copyAvailable = status; } )
   GUI_CS_SLOT_2(textCopyAvailable)

   bool copyAvailable;
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
   QMessageBoxPrivate()
      : escapeButton(nullptr), defaultButton(nullptr), checkbox(nullptr),
        clickedButton(nullptr), detailsButton(nullptr),
#ifndef QT_NO_TEXTEDIT
        detailsText(nullptr),
#endif
        compatMode(false), autoAddOkButton(true), detectedEscapeButton(nullptr), informativeLabel(nullptr),
        options(new QMessageDialogOptions)
   {
   }

   void init(const QString &title = QString(), const QString &text = QString());
   void setupLayout();
   void _q_buttonClicked(QAbstractButton *);
   void _q_clicked(QPlatformDialogHelper::StandardButton button, QPlatformDialogHelper::ButtonRole role);

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
   QCheckBox *checkbox;
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
   QString memberToDisconnectOnClose;
   QString signalToDisconnectOnClose;

   QSharedPointer<QMessageDialogOptions> options;

 private:
   void initHelper(QPlatformDialogHelper *) override;
   void helperPrepareShow(QPlatformDialogHelper *) override;
   void helperDone(QDialog::DialogCode, QPlatformDialogHelper *) override;
};

void QMessageBoxPrivate::init(const QString &title, const QString &text)
{
   Q_Q(QMessageBox);

   label = new QLabel;
   label->setObjectName("qt_msgbox_label");
   label->setTextInteractionFlags(Qt::TextInteractionFlags(q->style()->styleHint(QStyle::SH_MessageBox_TextInteractionFlags, nullptr, q)));

   label->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
   label->setOpenExternalLinks(true);

   iconLabel = new QLabel(q);
   iconLabel->setObjectName("qt_msgboxex_icon_label");
   iconLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

   buttonBox = new QDialogButtonBox;
   buttonBox->setObjectName("qt_msgbox_buttonbox");
   buttonBox->setCenterButtons(q->style()->styleHint(QStyle::SH_MessageBox_CenterButtons, nullptr, q));

   QObject::connect(buttonBox, &QDialogButtonBox::clicked, q, &QMessageBox::_q_buttonClicked);

   setupLayout();
   if (!title.isEmpty() || !text.isEmpty()) {
      q->setWindowTitle(title);
      q->setText(text);
   }

   q->setModal(true);

#ifdef Q_OS_DARWIN
   QFont f = q->font();
   f.setBold(true);
   label->setFont(f);
#endif

   icon = QMessageBox::NoIcon;
}

void QMessageBoxPrivate::setupLayout()
{
   Q_Q(QMessageBox);
   delete q->layout();

   QGridLayout *grid = new QGridLayout;
   bool hasIcon = iconLabel->pixmap() && !iconLabel->pixmap()->isNull();

   if (hasIcon) {
      grid->addWidget(iconLabel, 0, 0, 2, 1, Qt::AlignTop);
   }

   iconLabel->setVisible(hasIcon);

#ifdef Q_OS_DARWIN
   QSpacerItem *indentSpacer = new QSpacerItem(14, 1, QSizePolicy::Fixed, QSizePolicy::Fixed);
#else
   QSpacerItem *indentSpacer = new QSpacerItem(hasIcon ? 7 : 15, 1, QSizePolicy::Fixed, QSizePolicy::Fixed);
#endif

   grid->addItem(indentSpacer, 0, hasIcon ? 1 : 0, 2, 1);
   grid->addWidget(label, 0, hasIcon ? 2 : 1, 1, 1);

   if (informativeLabel) {
#ifndef Q_OS_DARWIN
      informativeLabel->setContentsMargins(0, 7, 0, 7);
#endif
      grid->addWidget(informativeLabel, 1, hasIcon ? 2 : 1, 1, 1);
   }
   if (checkbox) {
      grid->addWidget(checkbox, informativeLabel ? 2 : 1, hasIcon ? 2 : 1, 1, 1, Qt::AlignLeft);
#ifdef Q_OS_DARWIN
      grid->addItem(new QSpacerItem(1, 15, QSizePolicy::Fixed, QSizePolicy::Fixed), grid->rowCount(), 0);
#else
      grid->addItem(new QSpacerItem(1, 7, QSizePolicy::Fixed, QSizePolicy::Fixed), grid->rowCount(), 0);
#endif
   }
#ifdef Q_OS_DARWIN
   grid->addWidget(buttonBox, grid->rowCount(), hasIcon ? 2 : 1, 1, 1);
   grid->setMargin(0);
   grid->setVerticalSpacing(8);
   grid->setHorizontalSpacing(0);
   q->setContentsMargins(24, 15, 24, 20);

   // -- leave space for information label
   grid->setRowStretch(1, 100);
   grid->setRowMinimumHeight(2, 6);
#else
   grid->addWidget(buttonBox, grid->rowCount(), 0, 1, grid->columnCount());
#endif
   if (detailsText) {
      grid->addWidget(detailsText, grid->rowCount(), 0, 1, grid->columnCount());
   }
   grid->setSizeConstraint(QLayout::SetNoConstraint);
   q->setLayout(grid);
   retranslateStrings();
   updateSize();
}

int QMessageBoxPrivate::layoutMinimumWidth()
{
   layout->activate();
   return layout->totalMinimumSize().width();
}

void QMessageBoxPrivate::updateSize()
{
   Q_Q(QMessageBox);

   if (! q->isVisible()) {
      return;
   }

   QSize screenSize = QApplication::desktop()->availableGeometry(QCursor::pos()).size();

   int hardLimit = qMin(screenSize.width() - 480, 1000); // can never get bigger than this

   // on small screens allows the messagebox be the same size as the screen
   if (screenSize.width() <= 1024) {
      hardLimit = screenSize.width();
   }




#ifdef Q_OS_DARWIN
   int softLimit = qMin(screenSize.width() / 2, 420);

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

   QFontMetrics fm(QApplication::font("QMdiSubWindowTitleBar"));
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
      ret = customButtonList.indexOf(button);    // if button == nullptr, sets to InvalidRole

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
         QObject::disconnect(q, signalToDisconnectOnClose, receiverToDisconnectOnClose, memberToDisconnectOnClose);

         receiverToDisconnectOnClose = nullptr;
      }
      signalToDisconnectOnClose.clear();
      memberToDisconnectOnClose.clear();
   }
}

void QMessageBoxPrivate::_q_clicked(QPlatformDialogHelper::StandardButton button, QPlatformDialogHelper::ButtonRole role)
{
   (void) role;

   Q_Q(QMessageBox);
   q->done(button);
}

void QMessageBox::_q_clicked(QPlatformDialogHelper::StandardButton button, QPlatformDialogHelper::ButtonRole role)
{
   Q_D(QMessageBox);
   d->_q_clicked(button, role);
}

void QMessageBox::_q_buttonClicked(QAbstractButton *button)
{
   Q_D(QMessageBox);
   d->_q_buttonClicked(button);
}

QMessageBox::QMessageBox(QWidget *parent)
   : QDialog(*new QMessageBoxPrivate, parent, Qt::MSWindowsFixedSizeDialogHint | Qt::WindowTitleHint |
        Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint)
{
   Q_D(QMessageBox);
   d->init();
}

QMessageBox::QMessageBox(Icon icon, const QString &title, const QString &text, StandardButtons buttons,
   QWidget *parent, Qt::WindowFlags flags)

   : QDialog(*new QMessageBoxPrivate, parent, flags | Qt::MSWindowsFixedSizeDialogHint | Qt::WindowTitleHint |
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

   if (! button) {
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
      d->escapeButton = nullptr;
   }

   if (d->defaultButton == button) {
      d->defaultButton = nullptr;
   }

   d->buttonBox->removeButton(button);
   d->updateSize();
}

void QMessageBox::setStandardButtons(StandardButtons buttons)
{
   Q_D(QMessageBox);
   d->buttonBox->setStandardButtons(QDialogButtonBox::StandardButtons(int(buttons)));

   QList<QAbstractButton *> buttonList = d->buttonBox->buttons();
   if (! buttonList.contains(d->escapeButton)) {
      d->escapeButton = nullptr;
   }

   if (!buttonList.contains(d->defaultButton)) {
      d->defaultButton = nullptr;
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

         if (detectedEscapeButton) {
            // already detected
            detectedEscapeButton = nullptr;
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

         if (detectedEscapeButton) {
            // already detected
            detectedEscapeButton = nullptr;
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

void QMessageBox::setCheckBox(QCheckBox *cb)
{
   Q_D(QMessageBox);

   if (cb == d->checkbox) {
      return;
   }

   if (d->checkbox) {
      d->checkbox->hide();
      layout()->removeWidget(d->checkbox);
      if (d->checkbox->parentWidget() == this) {
         d->checkbox->setParent(nullptr);
         d->checkbox->deleteLater();
      }
   }
   d->checkbox = cb;
   if (d->checkbox) {
      QSizePolicy sp = d->checkbox->sizePolicy();
      sp.setHorizontalPolicy(QSizePolicy::MinimumExpanding);
      d->checkbox->setSizePolicy(sp);
   }
   d->setupLayout();
}
QCheckBox *QMessageBox::checkBox() const
{
   Q_D(const QMessageBox);
   return d->checkbox;
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
   d->icon = NoIcon;
   d->setupLayout();
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

Qt::TextInteractionFlags QMessageBox::textInteractionFlags() const
{
    Q_D(const QMessageBox);
    return d->label->textInteractionFlags();
}

void QMessageBox::setTextInteractionFlags(Qt::TextInteractionFlags flags)
{
    Q_D(QMessageBox);
    d->label->setTextInteractionFlags(flags);
}

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

void QMessageBox::resizeEvent(QResizeEvent *event)
{
   QDialog::resizeEvent(event);
}


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

void QMessageBox::changeEvent(QEvent *ev)
{
   Q_D(QMessageBox);

   switch (ev->type()) {
      case QEvent::StyleChange: {
         if (d->icon != NoIcon) {
            setIcon(d->icon);
         }
         Qt::TextInteractionFlags flags(style()->styleHint(QStyle::SH_MessageBox_TextInteractionFlags, nullptr, this));
         d->label->setTextInteractionFlags(flags);
         d->buttonBox->setCenterButtons(style()->styleHint(QStyle::SH_MessageBox_CenterButtons, nullptr, this));

         if (d->informativeLabel) {
            d->informativeLabel->setTextInteractionFlags(flags);
         }
      }
      [[fallthrough]];

      case QEvent::FontChange:
      case QEvent::ApplicationFontChange:
#ifdef Q_OS_DARWIN
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


void QMessageBox::keyPressEvent(QKeyEvent *e)
{
   Q_D(QMessageBox);

   if (e->matches(QKeySequence::Cancel)) {


      if (d->detectedEscapeButton) {

#ifdef Q_OS_DARWIN
         d->detectedEscapeButton->animateClick();
#else
         d->detectedEscapeButton->click();
#endif
      }
      return;
   }

#if !defined(QT_NO_CLIPBOARD) && !defined(QT_NO_SHORTCUT)
#if !defined(QT_NO_TEXTEDIT)
   if (e == QKeySequence::Copy) {
      if (d->detailsText && d->detailsText->isVisible() && d->detailsText->copy()) {
         e->setAccepted(true);
         return;
      }
   } else if (e == QKeySequence::SelectAll && d->detailsText && d->detailsText->isVisible()) {
      d->detailsText->selectAll();
      e->setAccepted(true);
      return;
   }
#endif // !QT_NO_TEXTEDIT
#if defined(Q_OS_WIN)
   if (e == QKeySequence::Copy) {
      QString separator = QString::fromLatin1("---------------------------\n");
      QString textToCopy = separator;

      separator.prepend('\n');
      textToCopy += windowTitle() + separator; // title
      textToCopy += d->label->text() + separator; // text

      if (d->informativeLabel) {
         textToCopy += d->informativeLabel->text() + separator;
      }

      QString buttonTexts;
      QList<QAbstractButton *> buttons = d->buttonBox->buttons();

      for (int i = 0; i < buttons.count(); i++) {
         buttonTexts += buttons[i]->text() + "   ";
      }
      textToCopy += buttonTexts + separator;

#ifndef QT_NO_TEXTEDIT
      if (d->detailsText) {
         textToCopy += d->detailsText->text() + separator;
      }
#endif

      QApplication::clipboard()->setText(textToCopy);
      return;
   }
#endif

#endif // !QT_NO_CLIPBOARD && !QT_NO_SHORTCUT
#ifndef QT_NO_SHORTCUT
   if (!(e->modifiers() & (Qt::AltModifier | Qt::ControlModifier | Qt::MetaModifier))) {
      int key = e->key() & ~Qt::KeyboardModifierMask;

      if (key) {
         const QList<QAbstractButton *> buttons = d->buttonBox->buttons();

         for (int i = 0; i < buttons.count(); ++i) {
            QAbstractButton *pb   = buttons.at(i);
            QKeySequence shortcut = pb->shortcut();

            if (!shortcut.isEmpty() && key == int(shortcut[0] & ~Qt::KeyboardModifierMask)) {
               pb->animateClick();
               return;
            }
         }
      }
   }

#endif
   QDialog::keyPressEvent(e);
}

void QMessageBox::open(QObject *receiver, const QString &member)
{
   Q_D(QMessageBox);

   const QString &signal = ! member.isEmpty() && member.contains('*') ?
      QString("buttonClicked(QAbstractButton*)") : QString("finished(int)");

   connect(this, signal, receiver, member);

   d->signalToDisconnectOnClose   = signal;
   d->receiverToDisconnectOnClose = receiver;
   d->memberToDisconnectOnClose   = member;

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
   QAccessibleEvent event(this, QAccessible::Alert);
   QAccessible::updateAccessibility(&event);
#endif

#ifdef Q_OS_WIN

   if (const HMENU systemMenu = qt_getWindowsSystemMenu(this)) {
      EnableMenuItem(systemMenu, SC_CLOSE, d->detectedEscapeButton ?
         MF_BYCOMMAND | MF_ENABLED : MF_BYCOMMAND | MF_GRAYED);
   }
#endif

   QDialog::showEvent(e);
}

static QMessageBox::StandardButton showNewMessageBox(QWidget *parent, QMessageBox::Icon icon, const QString &title,
   const QString &text, QMessageBox::StandardButtons buttons, QMessageBox::StandardButton defaultButton)
{
   // necessary for backwards compatibility
   // handles (Yes, No) and (Yes|Default, No)

   if (defaultButton && !(buttons & defaultButton)) {
      return (QMessageBox::StandardButton)
         QMessageBoxPrivate::showOldMessageBox(parent, icon, title, text, int(buttons), int(defaultButton), 0);
   }

   QMessageBox msgBox(icon, title, text, QMessageBox::NoButton, parent);
   QDialogButtonBox *buttonBox = msgBox.findChild<QDialogButtonBox *>();
   Q_ASSERT(buttonBox != nullptr);

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

#ifdef Q_OS_DARWIN
   static QPointer<QMessageBox> oldMsgBox;

   if (oldMsgBox && oldMsgBox->text() == text) {
      oldMsgBox->show();
      oldMsgBox->raise();
      oldMsgBox->activateWindow();
      return;
   }
#endif

   QMessageBox *msgBox = new QMessageBox(title, text, Information, 0, 0, 0, parent

#ifdef Q_OS_DARWIN
      , Qt::WindowTitleHint | Qt::WindowSystemMenuHint);
#else
      );
#endif

   msgBox->setAttribute(Qt::WA_DeleteOnClose);
   QIcon icon = msgBox->windowIcon();
   QSize size = icon.actualSize(QSize(64, 64));
   msgBox->setIconPixmap(icon.pixmap(size));

   // should perhaps be a style hint
#ifdef Q_OS_DARWIN
   oldMsgBox = msgBox;

   msgBox->d_func()->buttonBox->setCenterButtons(true);
   msgBox->show();

#else
   msgBox->exec();

#endif
}

void QMessageBox::aboutCs(QWidget *parent, const QString &title)
{
   QDialog *aboutBox = new QDialog(parent);
   aboutBox->setAttribute(Qt::WA_DeleteOnClose);

   if (title.isEmpty()) {
      aboutBox->setWindowTitle(tr("About CopperSpice"));

   } else {
      aboutBox->setWindowTitle(title);
   }

   QIcon icon(":/copperspice/dialogs/images/cslogo-64.png");
   aboutBox->setWindowIcon(icon);

   QLabel *msg1 = new QLabel;
   msg1->setText(tr("CopperSpice Version %1").formatArg(CS_VERSION_STR));

   QFont font = msg1->font();
   font.setWeight(QFont::Bold);
   font.setPointSize(10);
   msg1->setFont(font);

   QLabel *msg2 = new QLabel;
   msg2->setText(tr("CopperSpice is a set of C++ libraries for developing cross platform applications <br>"
                     "on X11, Windows, and Mac OS X\n"));

   font = msg2->font();
   font.setPointSize(10);
   msg2->setFont(font);

   QLabel *msg3 = new QLabel;

   msg3->setText("Copyright (c) 2012-2024 Ansel Sermersheim & Barbara Geller\n"
         "CopperSpice is released under the terms of the GNU LGPL version 2.1\n"
         "\n"
         "Copyright (c) 2015 The Qt Company Ltd\n"
         "Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies)\n"
         "Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies)");

   QLabel *csImage = nullptr;
   QPixmap pm(":/copperspice/dialogs/images/cslogo-64.png");

   if (! pm.isNull()) {
      csImage = new QLabel;
      csImage->setPixmap(pm);
   }

   QGridLayout *layout = new QGridLayout;

   if (csImage)  {
      layout->addWidget(csImage, 0, 0, 2, 1);
   }

   layout->addWidget(msg1, 0, 1);
   layout->addWidget(msg2, 1, 1);
   layout->addWidget(msg3, 2, 0, 1, 2);

   layout->setSpacing(15);
   layout->setContentsMargins(15, 9, 15, 20);
   layout->setSizeConstraint(QLayout::SetFixedSize);

   aboutBox->setLayout(layout);
   aboutBox->exec();
}

void QMessageBox::aboutQt(QWidget *parent, const QString &title)
{
   QMessageBox::aboutCs(parent, title);
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
      // for compatibility with older versions
      return nullptr;
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

QMessageBox::QMessageBox(const QString &title, const QString &text, Icon icon,
            int button0, int button1, int button2, QWidget *parent, Qt::WindowFlags flags)
    : QDialog(*new QMessageBoxPrivate, parent, flags
              | Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint)
{
    Q_D(QMessageBox);
    d->init(title, text);
    setIcon(icon);
    d->addOldButtons(button0, button1, button2);
}

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
      if (d->detailsText) {
         d->detailsText->hide();
         d->detailsText->deleteLater();
      }
      d->detailsText = nullptr;
      removeButton(d->detailsButton);
      if (d->detailsButton) {
         d->detailsButton->hide();
         d->detailsButton->deleteLater();
      }
      d->detailsButton = nullptr;

   } else {

      if (!d->detailsText) {
         d->detailsText = new QMessageBoxDetailsText(this);

         d->detailsText->hide();
      }
      if (!d->detailsButton) {
         const bool autoAddOkButton = d->autoAddOkButton; // QTBUG-39334, addButton() clears the flag.
         d->detailsButton = new DetailButton(this);
         addButton(d->detailsButton, QMessageBox::ActionRole);
         d->autoAddOkButton = autoAddOkButton;
      }
      d->detailsText->setText(text);
   }
   d->setupLayout();
}
#endif // QT_NO_TEXTEDIT

QString QMessageBox::informativeText() const
{
   Q_D(const QMessageBox);
   return d->informativeLabel ? d->informativeLabel->text() : QString();
}

void QMessageBox::setInformativeText(const QString &text)
{
   Q_D(QMessageBox);

   if (text.isEmpty()) {
      if (d->informativeLabel) {
         d->informativeLabel->hide();
         d->informativeLabel->deleteLater();
      }

      d->informativeLabel = nullptr;

   } else {
      if (!d->informativeLabel) {
         QLabel *label = new QLabel;
         label->setObjectName("qt_msgbox_informativelabel");
         label->setTextInteractionFlags(Qt::TextInteractionFlags(style()->styleHint(QStyle::SH_MessageBox_TextInteractionFlags, nullptr, this)));
         label->setAlignment(Qt::AlignTop | Qt::AlignLeft);
         label->setOpenExternalLinks(true);
         label->setWordWrap(true);
#ifdef Q_OS_DARWIN
         // apply a smaller font the information label on the mac
         label->setFont(cs_app_fonts_hash()->value("QTipLabel"));
#endif
         label->setWordWrap(true);
         d->informativeLabel = label;
      }
      d->informativeLabel->setText(text);
   }
   d->setupLayout();
}

void QMessageBox::setWindowTitle(const QString &title)
{
   // Message boxes on the mac do not have a title
#ifndef Q_OS_DARWIN
   QDialog::setWindowTitle(title);
#else
   (void) title;
#endif
}

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
   int iconSize = style->pixelMetric(QStyle::PM_MessageBoxIconSize, nullptr, mb);

   QIcon tmpIcon;

   switch (icon) {
      case QMessageBox::Information:
         tmpIcon = style->standardIcon(QStyle::SP_MessageBoxInformation, nullptr, mb);
         break;
      case QMessageBox::Warning:
         tmpIcon = style->standardIcon(QStyle::SP_MessageBoxWarning, nullptr, mb);
         break;
      case QMessageBox::Critical:
         tmpIcon = style->standardIcon(QStyle::SP_MessageBoxCritical, nullptr, mb);
         break;
      case QMessageBox::Question:
         tmpIcon = style->standardIcon(QStyle::SP_MessageBoxQuestion, nullptr, mb);
      default:
         break;
   }

   if (! tmpIcon.isNull()) {
      QWindow *window = nullptr;
      if (mb) {
         window = mb->windowHandle();

         if (! window) {
            if (const QWidget *nativeParent = mb->nativeParentWidget()) {
               window = nativeParent->windowHandle();
            }
         }
      }

      return tmpIcon.pixmap(window, QSize(iconSize, iconSize));
   }

   return QPixmap();
}

void QMessageBoxPrivate::initHelper(QPlatformDialogHelper *obj)
{
   Q_Q(QMessageBox);

   QPlatformMessageDialogHelper *tmp = dynamic_cast<QPlatformMessageDialogHelper *>(obj);

   if (tmp != nullptr) {
      QObject::connect(tmp, &QPlatformMessageDialogHelper::clicked, q, &QMessageBox::_q_clicked);

      tmp->setOptions(options);
   }
}

static QMessageDialogOptions::Icon helperIcon(QMessageBox::Icon icon)
{
   switch (icon) {
      case QMessageBox::NoIcon:
         return QMessageDialogOptions::NoIcon;

      case QMessageBox::Information:
         return QMessageDialogOptions::Information;

      case QMessageBox::Warning:
         return QMessageDialogOptions::Warning;

      case QMessageBox::Critical:
         return QMessageDialogOptions::Critical;

      case QMessageBox::Question:
         return QMessageDialogOptions::Question;
   }

   return QMessageDialogOptions::NoIcon;
}

static QPlatformDialogHelper::StandardButtons helperStandardButtons(QMessageBox *q)
{
   QPlatformDialogHelper::StandardButtons buttons(int(q->standardButtons()));
   return buttons;
}

void QMessageBoxPrivate::helperPrepareShow(QPlatformDialogHelper *)
{
   Q_Q(QMessageBox);
   options->setWindowTitle(q->windowTitle());
   options->setText(q->text());
   options->setInformativeText(q->informativeText());
   options->setDetailedText(q->detailedText());
   options->setIcon(helperIcon(q->icon()));
   options->setStandardButtons(helperStandardButtons(q));
}

void QMessageBoxPrivate::helperDone(QDialog::DialogCode code, QPlatformDialogHelper *)
{
   Q_Q(QMessageBox);
   clickedButton = q->button(QMessageBox::StandardButton(code));
}

// obsolete
QPixmap QMessageBox::standardIcon(Icon icon)
{
   return QMessageBoxPrivate::standardIcon(icon, nullptr);
}

#endif // QT_NO_MESSAGEBOX
