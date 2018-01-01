/***********************************************************************
*
* Copyright (c) 2012-2018 Barbara Geller
* Copyright (c) 2012-2018 Ansel Sermersheim
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

#ifndef QFONTDIALOG_P_H
#define QFONTDIALOG_P_H

#include <qdialog_p.h>
#include <qfontdatabase.h>
#include <qfontdialog.h>

#ifndef QT_NO_FONTDIALOG

QT_BEGIN_NAMESPACE

class QBoxLayout;
class QCheckBox;
class QComboBox;
class QDialogButtonBox;
class QFontListView;
class QGroupBox;
class QLabel;
class QLineEdit;

class QFontDialogPrivate : public QDialogPrivate
{
   Q_DECLARE_PUBLIC(QFontDialog)

 public:
   inline QFontDialogPrivate()
      : writingSystem(QFontDatabase::Any) {
   }

   void updateFamilies();
   void updateStyles();
   void updateSizes();

   static QFont getFont(bool *ok, const QFont &initial, QWidget *parent, const QString &title, 
                        QFontDialog::FontDialogOptions options);

   void init();
   void _q_sizeChanged(const QString &);
   void _q_familyHighlighted(int);
   void _q_writingSystemHighlighted(int);
   void _q_styleHighlighted(int);
   void _q_sizeHighlighted(int);
   void _q_updateSample();
   void updateSampleFont(const QFont &newFont);
   void retranslateStrings();

   QLabel *familyAccel;
   QLineEdit *familyEdit;
   QFontListView *familyList;

   QLabel *styleAccel;
   QLineEdit *styleEdit;
   QFontListView *styleList;

   QLabel *sizeAccel;
   QLineEdit *sizeEdit;
   QFontListView *sizeList;

   QGroupBox *effects;
   QCheckBox *strikeout;
   QCheckBox *underline;
   QComboBox *color;

   QGroupBox *sample;
   QLineEdit *sampleEdit;

   QLabel *writingSystemAccel;
   QComboBox *writingSystemCombo;

   QBoxLayout *buttonLayout;
   QBoxLayout *effectsLayout;
   QBoxLayout *sampleLayout;
   QBoxLayout *sampleEditLayout;

   QDialogButtonBox *buttonBox;

   QFontDatabase fdb;
   QString family;
   QFontDatabase::WritingSystem writingSystem;
   QString style;
   int size;
   bool smoothScalable;
   QFont selectedFont;
   QFontDialog::FontDialogOptions opts;
   QPointer<QObject> receiverToDisconnectOnClose;
   QByteArray memberToDisconnectOnClose;

#ifdef Q_OS_MAC
   static void setFont(void *delegate, const QFont &font);

   inline void done(int result) {
      q_func()->done(result);
   }

   inline QFontDialog *fontDialog() {
      return q_func();
   }

   void *delegate;
   void closeCocoaFontPanel();
   bool nativeDialogInUse;
   bool canBeNativeDialog();
   bool setVisible_sys(bool visible);
   void createNSFontPanelDelegate();
   void _q_macRunNativeAppModalPanel();
   void mac_nativeDialogModalHelp();
   bool showCocoaFontPanel();
   bool hideCocoaFontPanel();

   static bool sharedFontPanelAvailable;
#endif
};

#endif // QT_NO_FONTDIALOG

QT_END_NAMESPACE

#endif // QFONTDIALOG_P_H
