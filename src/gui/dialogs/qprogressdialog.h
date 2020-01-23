/***********************************************************************
*
* Copyright (c) 2012-2020 Barbara Geller
* Copyright (c) 2012-2020 Ansel Sermersheim
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

#ifndef QPROGRESSDIALOG_H
#define QPROGRESSDIALOG_H

#include <qdialog.h>
#include <QElapsedTimer>
#include <QShortcut>
#ifndef QT_NO_PROGRESSDIALOG

class QLabel;
class QProgressBar;
class QPushButton;
class QTimer;
class QVBoxLayout;

class Q_GUI_EXPORT QProgressDialog : public QDialog
{
   GUI_CS_OBJECT(QProgressDialog)

   GUI_CS_PROPERTY_READ(wasCanceled, wasCanceled)

   GUI_CS_PROPERTY_READ(minimum, minimum)
   GUI_CS_PROPERTY_WRITE(minimum, setMinimum)

   GUI_CS_PROPERTY_READ(maximum, maximum)
   GUI_CS_PROPERTY_WRITE(maximum, setMaximum)

   GUI_CS_PROPERTY_READ(value, value)
   GUI_CS_PROPERTY_WRITE(value, setValue)

   GUI_CS_PROPERTY_READ(autoReset, autoReset)
   GUI_CS_PROPERTY_WRITE(autoReset, setAutoReset)

   GUI_CS_PROPERTY_READ(autoClose, autoClose)
   GUI_CS_PROPERTY_WRITE(autoClose, setAutoClose)

   GUI_CS_PROPERTY_READ(minimumDuration, minimumDuration)
   GUI_CS_PROPERTY_WRITE(minimumDuration, setMinimumDuration)

   GUI_CS_PROPERTY_READ(labelText, labelText)
   GUI_CS_PROPERTY_WRITE(labelText, setLabelText)

 public:
   explicit QProgressDialog(QWidget *parent = nullptr, Qt::WindowFlags flags = Qt::WindowFlags());

   QProgressDialog(const QString &labelText, const QString &cancelButtonText, int minimum, int maximum,
      QWidget *parent = nullptr, Qt::WindowFlags flags = Qt::WindowFlags());

   ~QProgressDialog();

   QString labelText() const;
   void setLabel(QLabel *label);

   void setBar(QProgressBar *bar);

   void setCancelButton(QPushButton *button);
   void setCancelButtonCentered(bool value = true);

   QSize sizeHint() const override;

   int minimum() const;
   int maximum() const;
   int value() const;
   int minimumDuration() const;

   void setAutoReset(bool reset);
   bool autoReset() const;
   void setAutoClose(bool close);
   bool autoClose() const;
   bool wasCanceled() const;

   using QDialog::open;
   void open(QObject *receiver, const QString &member);

   GUI_CS_SLOT_1(Public, void cancel())
   GUI_CS_SLOT_2(cancel)

   GUI_CS_SLOT_1(Public, void reset())
   GUI_CS_SLOT_2(reset)

   GUI_CS_SLOT_1(Public, void setMaximum(int maximum))
   GUI_CS_SLOT_2(setMaximum)

   GUI_CS_SLOT_1(Public, void setMinimum(int minimum))
   GUI_CS_SLOT_2(setMinimum)

   GUI_CS_SLOT_1(Public, void setRange(int minimum, int maximum))
   GUI_CS_SLOT_2(setRange)

   GUI_CS_SLOT_1(Public, void setValue(int progress))
   GUI_CS_SLOT_2(setValue)

   GUI_CS_SLOT_1(Public, void setLabelText(const QString &text))
   GUI_CS_SLOT_2(setLabelText)

   GUI_CS_SLOT_1(Public, void setCancelButtonText(const QString &text))
   GUI_CS_SLOT_2(setCancelButtonText)

   GUI_CS_SLOT_1(Public, void setMinimumDuration(int ms))
   GUI_CS_SLOT_2(setMinimumDuration)

   GUI_CS_SIGNAL_1(Public, void canceled())
   GUI_CS_SIGNAL_2(canceled)

 protected:
   void closeEvent(QCloseEvent *event) override;
   void changeEvent(QEvent *event) override;
   void showEvent(QShowEvent *event) override;

   GUI_CS_SLOT_1(Protected, void forceShow())
   GUI_CS_SLOT_2(forceShow)

 private:
   Q_DISABLE_COPY(QProgressDialog)

   GUI_CS_SLOT_1(Private, void disconnectOnClose())
   GUI_CS_SLOT_2(disconnectOnClose)

   void init(const QString &labelText, const QString &cancelText, int min, int max);

   void ensureSizeIsAtLeastSizeHint();
   void setCancelButtonAlignment();
   void retranslateStrings();

   QLabel *m_label;
   QProgressBar *m_progressBar;
   QPushButton *m_cancelButton;
   QVBoxLayout *m_layout;

   QTimer *forceTimer;
   QElapsedTimer starttime;

#ifndef QT_NO_CURSOR
   QCursor parentCursor;
#endif

#ifndef QT_NO_SHORTCUT
   QShortcut *escapeShortcut;
#endif
   int  showTime;
   bool shown_once;
   bool cancellation_flag;
   bool setValue_called;
   bool m_autoClose;
   bool m_autoReset;
   bool forceHide;
   bool useDefaultCancelText;
   bool m_centerCancelPB;
   QPointer<QObject> receiverToDisconnectOnClose;
   QString memberToDisconnectOnClose;
};

#endif // QT_NO_PROGRESSDIALOG

#endif
