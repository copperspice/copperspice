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

#ifndef QCOLORDIALOG_P_H
#define QCOLORDIALOG_P_H

#include <qdialog_p.h>
#include <qcolordialog.h>

#ifndef QT_NO_COLORDIALOG

QT_BEGIN_NAMESPACE

class QColorLuminancePicker;
class QColorPicker;
class QColorShower;
class QDialogButtonBox;
class QLabel;
class QVBoxLayout;
class QPushButton;
class QWellArray;

class QColorDialogPrivate : public QDialogPrivate
{
   Q_DECLARE_PUBLIC(QColorDialog)

 public:
   void init(const QColor &initial);
   QRgb currentColor() const;
   QColor currentQColor() const;
   void setCurrentColor(QRgb rgb);
   void setCurrentQColor(const QColor &color);
   bool selectColor(const QColor &color);

   int currentAlpha() const;
   void setCurrentAlpha(int a);
   void showAlpha(bool b);
   bool isAlphaVisible() const;
   void retranslateStrings();

   void _q_addCustom();

   void _q_newHsv(int h, int s, int v);
   void _q_newColorTypedIn(const QRgb &rgb);
   void _q_newCustom(int, int);
   void _q_newStandard(int, int);

   QWellArray *custom;
   QWellArray *standard;

   QDialogButtonBox *buttons;
   QVBoxLayout *leftLay;
   QColorPicker *cp;
   QColorLuminancePicker *lp;
   QColorShower *cs;
   QLabel *lblBasicColors;
   QLabel *lblCustomColors;
   QPushButton *ok;
   QPushButton *cancel;
   QPushButton *addCusBt;
   QColor selectedQColor;
   int nextCust;
   bool smallDisplay;
   QColorDialog::ColorDialogOptions opts;
   QPointer<QObject> receiverToDisconnectOnClose;
   QByteArray memberToDisconnectOnClose;
   bool nativeDialogInUse;

#ifdef Q_OS_MAC
   void openCocoaColorPanel(const QColor &initial,
                            QWidget *parent, const QString &title, QColorDialog::ColorDialogOptions options);
   void closeCocoaColorPanel();
   void releaseCocoaColorPanelDelegate();
   void setCocoaPanelColor(const QColor &color);

   inline void done(int result) {
      q_func()->done(result);
   }

   inline QColorDialog *colorDialog() {
      return q_func();
   }

   void *delegate;

   static bool sharedColorPanelAvailable;

   void _q_macRunNativeAppModalPanel();
   void mac_nativeDialogModalHelp();
#endif

};

#endif // QT_NO_COLORDIALOG

QT_END_NAMESPACE

#endif // QCOLORDIALOG_P_H
