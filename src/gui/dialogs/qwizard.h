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

#ifndef QWIZARD_H
#define QWIZARD_H

#include <qdialog.h>

#ifndef QT_NO_WIZARD

class QAbstractButton;
class QWizardPage;
class QWizardPrivate;
class QWizardPagePrivate;

class Q_GUI_EXPORT QWizard : public QDialog
{
   GUI_CS_OBJECT(QWizard)

   GUI_CS_ENUM(WizardStyle)
   GUI_CS_ENUM(WizardOption)
   GUI_CS_FLAG(WizardOption, WizardOptions)

   GUI_CS_PROPERTY_READ(wizardStyle, wizardStyle)
   GUI_CS_PROPERTY_WRITE(wizardStyle, setWizardStyle)

   GUI_CS_PROPERTY_READ(options, options)
   GUI_CS_PROPERTY_WRITE(options, setOptions)

   GUI_CS_PROPERTY_READ(titleFormat, titleFormat)
   GUI_CS_PROPERTY_WRITE(titleFormat, setTitleFormat)

   GUI_CS_PROPERTY_READ(subTitleFormat, subTitleFormat)
   GUI_CS_PROPERTY_WRITE(subTitleFormat, setSubTitleFormat)

   GUI_CS_PROPERTY_READ(startId, startId)
   GUI_CS_PROPERTY_WRITE(startId, setStartId)

   GUI_CS_PROPERTY_READ(currentId, currentId)
   GUI_CS_PROPERTY_NOTIFY(currentId, currentIdChanged)

 public:
   enum WizardButton {
      BackButton,
      NextButton,
      CommitButton,
      FinishButton,
      CancelButton,
      HelpButton,
      CustomButton1,
      CustomButton2,
      CustomButton3,
      Stretch,

      NoButton = -1,
      NStandardButtons = 6,
      NButtons = 9
   };

   enum WizardPixmap {
      WatermarkPixmap,
      LogoPixmap,
      BannerPixmap,
      BackgroundPixmap,
      NPixmaps
   };

   enum WizardStyle {
      ClassicStyle,
      ModernStyle,
      MacStyle,
      AeroStyle,
      NStyles
   };

   enum WizardOption {
      IndependentPages                = 0x00000001,
      IgnoreSubTitles                 = 0x00000002,
      ExtendedWatermarkPixmap         = 0x00000004,
      NoDefaultButton                 = 0x00000008,
      NoBackButtonOnStartPage         = 0x00000010,
      NoBackButtonOnLastPage          = 0x00000020,
      DisabledBackButtonOnLastPage    = 0x00000040,
      HaveNextButtonOnLastPage        = 0x00000080,
      HaveFinishButtonOnEarlyPages    = 0x00000100,
      NoCancelButton                  = 0x00000200,
      CancelButtonOnLeft              = 0x00000400,
      HaveHelpButton                  = 0x00000800,
      HelpButtonOnRight               = 0x00001000,
      HaveCustomButton1               = 0x00002000,
      HaveCustomButton2               = 0x00004000,
      HaveCustomButton3               = 0x00008000,
      NoCancelButtonOnLastPage        = 0x00010000
   };

   using WizardOptions = QFlags<WizardOption>;

   explicit QWizard(QWidget *parent = nullptr, Qt::WindowFlags flags = Qt::EmptyFlag);

   QWizard(const QWizard &) = delete;
   QWizard &operator=(const QWizard &) = delete;

   ~QWizard();

   int addPage(QWizardPage *page);
   void setPage(int id, QWizardPage *page);
   void removePage(int id);
   QWizardPage *page(int id) const;
   bool hasVisitedPage(int id) const;
   QList<int> visitedPages() const;    // ### visitedIds()?
   QList<int> pageIds() const;
   void setStartId(int id);
   int startId() const;
   QWizardPage *currentPage() const;
   int currentId() const;

   virtual bool validateCurrentPage();
   virtual int nextId() const;

   void setField(const QString &name, const QVariant &value);
   QVariant field(const QString &name) const;

   void setWizardStyle(WizardStyle style);
   WizardStyle wizardStyle() const;

   void setOption(WizardOption option, bool on = true);
   bool testOption(WizardOption option) const;
   void setOptions(WizardOptions options);
   WizardOptions options() const;

   void setButtonText(WizardButton which, const QString &text);
   QString buttonText(WizardButton which) const;
   void setButtonLayout(const QList<WizardButton> &layout);
   void setButton(WizardButton which, QAbstractButton *button);
   QAbstractButton *button(WizardButton which) const;

   void setTitleFormat(Qt::TextFormat format);
   Qt::TextFormat titleFormat() const;
   void setSubTitleFormat(Qt::TextFormat format);
   Qt::TextFormat subTitleFormat() const;
   void setPixmap(WizardPixmap which, const QPixmap &pixmap);
   QPixmap pixmap(WizardPixmap which) const;

   void setSideWidget(QWidget *widget);
   QWidget *sideWidget() const;

   void setDefaultProperty(const QString &className, const QString &property, const QString &changedSignal);

   void setVisible(bool visible) override;
   QSize sizeHint() const override;

   GUI_CS_SIGNAL_1(Public, void currentIdChanged(int id))
   GUI_CS_SIGNAL_2(currentIdChanged, id)

   GUI_CS_SIGNAL_1(Public, void helpRequested())
   GUI_CS_SIGNAL_2(helpRequested)

   GUI_CS_SIGNAL_1(Public, void customButtonClicked(int which))
   GUI_CS_SIGNAL_2(customButtonClicked, which)

   GUI_CS_SIGNAL_1(Public, void pageAdded(int id))
   GUI_CS_SIGNAL_2(pageAdded, id)

   GUI_CS_SIGNAL_1(Public, void pageRemoved(int id))
   GUI_CS_SIGNAL_2(pageRemoved, id)

   GUI_CS_SLOT_1(Public, void back())
   GUI_CS_SLOT_2(back)

   GUI_CS_SLOT_1(Public, void next())
   GUI_CS_SLOT_2(next)

   GUI_CS_SLOT_1(Public, void restart())
   GUI_CS_SLOT_2(restart)

 protected:
   bool event(QEvent *event) override;
   void resizeEvent(QResizeEvent *event) override;
   void paintEvent(QPaintEvent *event) override;

#if defined(Q_OS_WIN)
   bool nativeEvent(const QByteArray &eventType, void *message, long *result) override;
#endif

   void done(int result) override;
   virtual void initializePage(int id);
   virtual void cleanupPage(int id);

 private:
   Q_DECLARE_PRIVATE(QWizard)

   GUI_CS_SLOT_1(Private, void _q_emitCustomButtonClicked())
   GUI_CS_SLOT_2(_q_emitCustomButtonClicked)

   GUI_CS_SLOT_1(Private, void _q_updateButtonStates())
   GUI_CS_SLOT_2(_q_updateButtonStates)

   void _q_handleFieldObjectDestroyed(QObject *obj);

   friend class QWizardPage;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QWizard::WizardOptions)

class Q_GUI_EXPORT QWizardPage : public QWidget
{
   GUI_CS_OBJECT(QWizardPage)

   GUI_CS_PROPERTY_READ(title, title)
   GUI_CS_PROPERTY_WRITE(title, setTitle)

   GUI_CS_PROPERTY_READ(subTitle, subTitle)
   GUI_CS_PROPERTY_WRITE(subTitle, setSubTitle)

 public:
   explicit QWizardPage(QWidget *parent = nullptr);

   QWizardPage(const QWizardPage &) = delete;
   QWizardPage &operator=(const QWizardPage &) = delete;

   ~QWizardPage();

   void setTitle(const QString &title);
   QString title() const;
   void setSubTitle(const QString &subTitle);
   QString subTitle() const;
   void setPixmap(QWizard::WizardPixmap which, const QPixmap &pixmap);
   QPixmap pixmap(QWizard::WizardPixmap which) const;
   void setFinalPage(bool finalPage);
   bool isFinalPage() const;
   void setCommitPage(bool commitPage);
   bool isCommitPage() const;
   void setButtonText(QWizard::WizardButton which, const QString &text);
   QString buttonText(QWizard::WizardButton which) const;

   virtual void initializePage();
   virtual void cleanupPage();
   virtual bool validatePage();
   virtual bool isComplete() const;
   virtual int nextId() const;

   GUI_CS_SIGNAL_1(Public, void completeChanged())
   GUI_CS_SIGNAL_2(completeChanged)

 protected:
   void setField(const QString &name, const QVariant &value);
   QVariant field(const QString &name) const;
   void registerField(const QString &name, QWidget *widget, const QString &property = QString(),
         const QString &changedSignal = QString());
   QWizard *wizard() const;

 private:
   Q_DECLARE_PRIVATE(QWizardPage)

   GUI_CS_SLOT_1(Private, void _q_changedSignal())
   GUI_CS_SLOT_2(_q_changedSignal)

   GUI_CS_SLOT_1(Private, void _q_updateCachedCompleteState())
   GUI_CS_SLOT_2(_q_updateCachedCompleteState)

   friend class QWizard;
   friend class QWizardPrivate;
};

#endif // QT_NO_WIZARD

#endif
