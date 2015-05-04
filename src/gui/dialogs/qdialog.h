/***********************************************************************
*
* Copyright (c) 2012-2015 Barbara Geller
* Copyright (c) 2012-2015 Ansel Sermersheim
* Copyright (c) 2012-2014 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
*
* This file is part of CopperSpice.
*
* CopperSpice is free software: you can redistribute it and/or 
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 as published by the Free Software Foundation.
*
* CopperSpice is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with CopperSpice.  If not, see 
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#ifndef QDIALOG_H
#define QDIALOG_H

#include <QtGui/qwidget.h>

QT_BEGIN_NAMESPACE

class QPushButton;
class QDialogPrivate;

class Q_GUI_EXPORT QDialog : public QWidget
{
   CS_OBJECT(QDialog)
   friend class QPushButton;

   GUI_CS_PROPERTY_READ(sizeGripEnabled, isSizeGripEnabled)
   GUI_CS_PROPERTY_WRITE(sizeGripEnabled, setSizeGripEnabled)
   GUI_CS_PROPERTY_READ(modal, isModal)
   GUI_CS_PROPERTY_WRITE(modal, setModal)

 public:
   explicit QDialog(QWidget *parent = 0, Qt::WindowFlags f = 0);
   ~QDialog();

   enum DialogCode { Rejected, Accepted };

   int result() const;

   void setVisible(bool visible);

   void setOrientation(Qt::Orientation orientation);
   Qt::Orientation orientation() const;

   void setExtension(QWidget *extension);
   QWidget *extension() const;

   QSize sizeHint() const;
   QSize minimumSizeHint() const;

   void setSizeGripEnabled(bool);
   bool isSizeGripEnabled() const;

   void setModal(bool modal);
   void setResult(int r);

   GUI_CS_SIGNAL_1(Public, void finished(int result))
   GUI_CS_SIGNAL_2(finished, result)

   GUI_CS_SIGNAL_1(Public, void accepted())
   GUI_CS_SIGNAL_2(accepted)

   GUI_CS_SIGNAL_1(Public, void rejected())
   GUI_CS_SIGNAL_2(rejected)

   GUI_CS_SLOT_1(Public, void open())
   GUI_CS_SLOT_2(open)

   GUI_CS_SLOT_1(Public, int exec())
   GUI_CS_SLOT_2(exec)

   GUI_CS_SLOT_1(Public, virtual void done(int un_named_arg1))
   GUI_CS_SLOT_2(done)

   GUI_CS_SLOT_1(Public, virtual void accept())
   GUI_CS_SLOT_2(accept)

   GUI_CS_SLOT_1(Public, virtual void reject())
   GUI_CS_SLOT_2(reject)

   GUI_CS_SLOT_1(Public, void showExtension(bool un_named_arg1))
   GUI_CS_SLOT_2(showExtension)

 protected:
   QDialog(QDialogPrivate &, QWidget *parent, Qt::WindowFlags f = 0);

   void keyPressEvent(QKeyEvent *);
   void closeEvent(QCloseEvent *);
   void showEvent(QShowEvent *);
   void resizeEvent(QResizeEvent *);

#ifndef QT_NO_CONTEXTMENU
   void contextMenuEvent(QContextMenuEvent *);
#endif

   bool eventFilter(QObject *, QEvent *);
   void adjustPosition(QWidget *);

 private:
   Q_DECLARE_PRIVATE(QDialog)
   Q_DISABLE_COPY(QDialog)

};

QT_END_NAMESPACE

#endif // QDIALOG_H
