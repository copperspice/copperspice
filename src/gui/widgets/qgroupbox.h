/***********************************************************************
*
* Copyright (c) 2012-2019 Barbara Geller
* Copyright (c) 2012-2019 Ansel Sermersheim
*
* Copyright (C) 2015 The Qt Company Ltd.
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

#ifndef QGROUPBOX_H
#define QGROUPBOX_H

#include <QtGui/qframe.h>

QT_BEGIN_NAMESPACE

#ifndef QT_NO_GROUPBOX

class QGroupBoxPrivate;
class QStyleOptionGroupBox;

class Q_GUI_EXPORT QGroupBox : public QWidget
{
   GUI_CS_OBJECT(QGroupBox)

   GUI_CS_PROPERTY_READ(title, title)
   GUI_CS_PROPERTY_WRITE(title, setTitle)
   GUI_CS_PROPERTY_READ(alignment, alignment)
   GUI_CS_PROPERTY_WRITE(alignment, setAlignment)
   GUI_CS_PROPERTY_READ(flat, isFlat)
   GUI_CS_PROPERTY_WRITE(flat, setFlat)

   GUI_CS_PROPERTY_READ(checkable, isCheckable)
   GUI_CS_PROPERTY_WRITE(checkable, setCheckable)
   GUI_CS_PROPERTY_READ(checked, isChecked)
   GUI_CS_PROPERTY_WRITE(checked, setChecked)
   GUI_CS_PROPERTY_DESIGNABLE_NONSTATIC(checked, isCheckable())
   GUI_CS_PROPERTY_NOTIFY(checked, toggled)
   GUI_CS_PROPERTY_USER(checked, true)

 public:
   explicit QGroupBox(QWidget *parent = nullptr);
   explicit QGroupBox(const QString &title, QWidget *parent = nullptr);
   ~QGroupBox();

   QString title() const;
   void setTitle(const QString &title);

   Qt::Alignment alignment() const;
   void setAlignment(int alignment);

   QSize minimumSizeHint() const override;

   bool isFlat() const;
   void setFlat(bool flat);
   bool isCheckable() const;
   void setCheckable(bool checkable);
   bool isChecked() const;

   GUI_CS_SLOT_1(Public, void setChecked(bool checked))
   GUI_CS_SLOT_2(setChecked)

   GUI_CS_SIGNAL_1(Public, void clicked(bool checked = false))
   GUI_CS_SIGNAL_2(clicked, checked)
   GUI_CS_SIGNAL_1(Public, void toggled(bool un_named_arg1))
   GUI_CS_SIGNAL_2(toggled, un_named_arg1)

 protected:
   bool event(QEvent *event) override;
   void childEvent(QChildEvent *event) override;
   void resizeEvent(QResizeEvent *event) override;
   void paintEvent(QPaintEvent *event) override;
   void focusInEvent(QFocusEvent *event) override;
   void changeEvent(QEvent *event) override;
   void mousePressEvent(QMouseEvent *event) override;
   void mouseMoveEvent(QMouseEvent *event) override;
   void mouseReleaseEvent(QMouseEvent *event) override;
   void initStyleOption(QStyleOptionGroupBox *option) const;

 private:
   Q_DISABLE_COPY(QGroupBox)
   Q_DECLARE_PRIVATE(QGroupBox)

   GUI_CS_SLOT_1(Private, void _q_setChildrenEnabled(bool b))
   GUI_CS_SLOT_2(_q_setChildrenEnabled)
};

#endif // QT_NO_GROUPBOX

QT_END_NAMESPACE

#endif // QGROUPBOX_H
