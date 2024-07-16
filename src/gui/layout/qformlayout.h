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

#ifndef QFORMLAYOUT_H
#define QFORMLAYOUT_H

#include <QLayout>

class QFormLayoutPrivate;

class Q_GUI_EXPORT QFormLayout : public QLayout
{
   GUI_CS_OBJECT(QFormLayout)

   Q_DECLARE_PRIVATE(QFormLayout)

   GUI_CS_ENUM(FieldGrowthPolicy)
   GUI_CS_ENUM(RowWrapPolicy)
   GUI_CS_ENUM(ItemRole)

   GUI_CS_PROPERTY_READ(fieldGrowthPolicy, fieldGrowthPolicy)
   GUI_CS_PROPERTY_WRITE(fieldGrowthPolicy, setFieldGrowthPolicy)
   GUI_CS_PROPERTY_RESET(fieldGrowthPolicy, resetFieldGrowthPolicy)

   GUI_CS_PROPERTY_READ(rowWrapPolicy, rowWrapPolicy)
   GUI_CS_PROPERTY_WRITE(rowWrapPolicy, setRowWrapPolicy)
   GUI_CS_PROPERTY_RESET(rowWrapPolicy, resetRowWrapPolicy)

   GUI_CS_PROPERTY_READ(labelAlignment, labelAlignment)
   GUI_CS_PROPERTY_WRITE(labelAlignment, setLabelAlignment)
   GUI_CS_PROPERTY_RESET(labelAlignment, resetLabelAlignment)

   GUI_CS_PROPERTY_READ(formAlignment, formAlignment)
   GUI_CS_PROPERTY_WRITE(formAlignment, setFormAlignment)
   GUI_CS_PROPERTY_RESET(formAlignment, resetFormAlignment)

   GUI_CS_PROPERTY_READ(horizontalSpacing, horizontalSpacing)
   GUI_CS_PROPERTY_WRITE(horizontalSpacing, setHorizontalSpacing)

   GUI_CS_PROPERTY_READ(verticalSpacing, verticalSpacing)
   GUI_CS_PROPERTY_WRITE(verticalSpacing, setVerticalSpacing)

 public:

   GUI_CS_REGISTER_ENUM(
      enum FieldGrowthPolicy {
         FieldsStayAtSizeHint,
         ExpandingFieldsGrow,
         AllNonFixedFieldsGrow
      };
   )

   GUI_CS_REGISTER_ENUM(
      enum RowWrapPolicy {
         DontWrapRows,
         WrapLongRows,
         WrapAllRows
      };
   )

   enum ItemRole {
      LabelRole    = 0,
      FieldRole    = 1,
      SpanningRole = 2
   };

   explicit QFormLayout(QWidget *parent = nullptr);
   ~QFormLayout();

   void setFieldGrowthPolicy(FieldGrowthPolicy policy);
   FieldGrowthPolicy fieldGrowthPolicy() const;
   void setRowWrapPolicy(RowWrapPolicy policy);
   RowWrapPolicy rowWrapPolicy() const;
   void setLabelAlignment(Qt::Alignment alignment);
   Qt::Alignment labelAlignment() const;
   void setFormAlignment(Qt::Alignment alignment);
   Qt::Alignment formAlignment() const;

   void setHorizontalSpacing(int spacing);
   int horizontalSpacing() const;
   void setVerticalSpacing(int spacing);
   int verticalSpacing() const;

   int spacing() const;
   void setSpacing(int spacing);

   void addRow(QWidget *label, QWidget *field);
   void addRow(QWidget *label, QLayout *field);
   void addRow(const QString &labelText, QWidget *field);
   void addRow(const QString &labelText, QLayout *field);
   void addRow(QWidget *widget);
   void addRow(QLayout *layout);

   void insertRow(int row, QWidget *label, QWidget *field);
   void insertRow(int row, QWidget *label, QLayout *field);
   void insertRow(int row, const QString &labelText, QWidget *field);
   void insertRow(int row, const QString &labelText, QLayout *field);
   void insertRow(int row, QWidget *widget);
   void insertRow(int row, QLayout *layout);

   void setItem(int row, ItemRole role, QLayoutItem *item);
   void setWidget(int row, ItemRole role, QWidget *widget);
   void setLayout(int row, ItemRole role, QLayout *layout);

   QLayoutItem *itemAt(int row, ItemRole role) const;
   void getItemPosition(int index, int *rowPtr, ItemRole *rolePtr) const;
   void getWidgetPosition(QWidget *widget, int *rowPtr, ItemRole *rolePtr) const;
   void getLayoutPosition(QLayout *layout, int *rowPtr, ItemRole *rolePtr) const;
   QWidget *labelForField(QWidget *field) const;
   QWidget *labelForField(QLayout *field) const;

   void addItem(QLayoutItem *item) override;
   QLayoutItem *itemAt(int index) const override;
   QLayoutItem *takeAt(int index) override;

   void setGeometry(const QRect &rect) override;
   QSize minimumSize() const override;
   QSize sizeHint() const override;
   void invalidate() override;

   bool hasHeightForWidth() const override;
   int heightForWidth(int width) const override;
   Qt::Orientations expandingDirections() const override;
   int count() const override;

   int rowCount() const;

 private:
   void resetFieldGrowthPolicy();
   void resetRowWrapPolicy();
   void resetLabelAlignment();
   void resetFormAlignment();
};

#endif
