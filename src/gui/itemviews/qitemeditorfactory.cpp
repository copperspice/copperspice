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

#include <qplatformdefs.h>

#include <qitemeditorfactory.h>
#include <qitemeditorfactory_p.h>

#ifndef QT_NO_ITEMVIEWS

#include <qalgorithms.h>
#include <qapplication.h>
#include <qcombobox.h>
#include <qdatetimeedit.h>
#include <qdebug.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qspinbox.h>

#include <limits.h>
#include <float.h>

#ifndef QT_NO_COMBOBOX

class QBooleanComboBox : public QComboBox
{
   GUI_CS_OBJECT(QBooleanComboBox)

   GUI_CS_PROPERTY_READ(value, value)
   GUI_CS_PROPERTY_WRITE(value, setValue)
   GUI_CS_PROPERTY_USER(value, true)

 public:
   QBooleanComboBox(QWidget *parent);
   void setValue(bool);
   bool value() const;
};

#endif

#ifndef QT_NO_SPINBOX

class QUIntSpinBox : public QSpinBox
{
   GUI_CS_OBJECT(QUIntSpinBox)

   GUI_CS_PROPERTY_READ(value, uintValue)
   GUI_CS_PROPERTY_WRITE(value, setUIntValue)
   GUI_CS_PROPERTY_NOTIFY(value, uintValueChanged)
   GUI_CS_PROPERTY_USER(value, true)

 public:
   explicit QUIntSpinBox(QWidget *parent = nullptr)
      : QSpinBox(parent)
   {
      connect(this, cs_mp_cast<int>(&QUIntSpinBox::valueChanged), this, &QUIntSpinBox::uintValueChanged);
   }

   uint uintValue() const {
      return value();
   }

   void setUIntValue(uint data) {
      return setValue(data);
   }

   GUI_CS_SIGNAL_1(Public, void uintValueChanged())
   GUI_CS_SIGNAL_2(uintValueChanged)
};
#endif

QWidget *QItemEditorFactory::createEditor(QVariant::Type type, QWidget *parent) const
{
   QItemEditorCreatorBase *creator = creatorMap.value(type, nullptr);

   if (! creator) {
      const QItemEditorFactory *dfactory = defaultFactory();
      return dfactory == this ? nullptr : dfactory->createEditor(type, parent);
   }

   return creator->createWidget(parent);
}

QString QItemEditorFactory::valuePropertyName(QVariant::Type type) const
{
   QItemEditorCreatorBase *creator = creatorMap.value(type, nullptr);

   if (! creator) {
      const QItemEditorFactory *dfactory = defaultFactory();
      return dfactory == this ? QString() : dfactory->valuePropertyName(type);
   }

   return creator->valuePropertyName();
}

QItemEditorFactory::~QItemEditorFactory()
{
   //we make sure we delete all the QItemEditorCreatorBase
   //this has to be done only once, hence the QSet
   QSet<QItemEditorCreatorBase *> set = creatorMap.values().toSet();
   qDeleteAll(set);
}

void QItemEditorFactory::registerEditor(QVariant::Type type, QItemEditorCreatorBase *creator)
{
   QHash<QVariant::Type, QItemEditorCreatorBase *>::iterator it = creatorMap.find(type);

   if (it != creatorMap.end()) {
      QItemEditorCreatorBase *oldCreator = it.value();

      Q_ASSERT(oldCreator);
      creatorMap.erase(it);

      if (!creatorMap.values().contains(oldCreator)) {
         delete oldCreator;   // if it is no more in use we can delete it
      }
   }

   creatorMap[type] = creator;
}

class QDefaultItemEditorFactory : public QItemEditorFactory
{
 public:
   inline QDefaultItemEditorFactory() {}

   QWidget *createEditor(QVariant::Type type, QWidget *parent) const override;
   QString valuePropertyName(QVariant::Type) const override;
};

QWidget *QDefaultItemEditorFactory::createEditor(QVariant::Type type, QWidget *parent) const
{
   switch (type) {
#ifndef QT_NO_COMBOBOX
      case QVariant::Bool: {
         QBooleanComboBox *cb = new QBooleanComboBox(parent);
         cb->setFrame(false);
         return cb;
      }
#endif

#ifndef QT_NO_SPINBOX
      case QVariant::UInt: {
         QSpinBox *sb = new QUIntSpinBox(parent);
         sb->setFrame(false);
         sb->setMinimum(0);
         sb->setMaximum(INT_MAX);
         return sb;
      }

      case QVariant::Int: {
         QSpinBox *sb = new QSpinBox(parent);
         sb->setFrame(false);
         sb->setMinimum(INT_MIN);
         sb->setMaximum(INT_MAX);
         return sb;
      }
#endif

#ifndef QT_NO_DATETIMEEDIT
      case QVariant::Date: {
         QDateTimeEdit *ed = new QDateEdit(parent);
         ed->setFrame(false);
         return ed;
      }

      case QVariant::Time: {
         QDateTimeEdit *ed = new QTimeEdit(parent);
         ed->setFrame(false);
         return ed;
      }

      case QVariant::DateTime: {
         QDateTimeEdit *ed = new QDateTimeEdit(parent);
         ed->setFrame(false);
         return ed;
      }
#endif

      case QVariant::Pixmap:
         return new QLabel(parent);

#ifndef QT_NO_SPINBOX
      case QVariant::Double: {
         QDoubleSpinBox *sb = new QDoubleSpinBox(parent);
         sb->setFrame(false);
         sb->setMinimum(-DBL_MAX);
         sb->setMaximum(DBL_MAX);
         return sb;
      }
#endif

#ifndef QT_NO_LINEEDIT
      case QVariant::String:
      default: {
         // the default editor is a lineedit
         QExpandingLineEdit *le = new QExpandingLineEdit(parent);
         le->setFrame(le->style()->styleHint(QStyle::SH_ItemView_DrawDelegateFrame, nullptr, le));
         if (!le->style()->styleHint(QStyle::SH_ItemView_ShowDecorationSelected, nullptr, le)) {
            le->setWidgetOwnsGeometry(true);
         }
         return le;
      }
#else
      default:
         break;
#endif
   }

   return nullptr;
}

QString QDefaultItemEditorFactory::valuePropertyName(QVariant::Type type) const
{
   switch (type) {

#ifndef QT_NO_COMBOBOX
      case QVariant::Bool:
         return QString("currentIndex");
#endif

#ifndef QT_NO_SPINBOX
      case QVariant::UInt:
      case QVariant::Int:
      case QVariant::Double:
         return QString("value");
#endif

#ifndef QT_NO_DATETIMEEDIT
      case QVariant::Date:
         return QString("date");

      case QVariant::Time:
         return QString("time");

      case QVariant::DateTime:
         return QString("dateTime");
#endif

      case QVariant::String:
      default:
         // the default editor is a lineedit
         return QString("text");
   }
}

static QItemEditorFactory *q_default_factory = nullptr;

struct QDefaultFactoryCleaner {
   inline QDefaultFactoryCleaner() {}

   ~QDefaultFactoryCleaner() {
      delete q_default_factory;
      q_default_factory = nullptr;
   }
};

const QItemEditorFactory *QItemEditorFactory::defaultFactory()
{
   static const QDefaultItemEditorFactory factory;

   if (q_default_factory) {
      return q_default_factory;
   }

   return &factory;
}

void QItemEditorFactory::setDefaultFactory(QItemEditorFactory *factory)
{
   static const QDefaultFactoryCleaner cleaner;
   delete q_default_factory;
   q_default_factory = factory;
}

QItemEditorCreatorBase::~QItemEditorCreatorBase()
{
}

#ifndef QT_NO_LINEEDIT

QExpandingLineEdit::QExpandingLineEdit(QWidget *parent)
   : QLineEdit(parent), originalWidth(-1), widgetOwnsGeometry(false)
{
   connect(this, &QExpandingLineEdit::textChanged, this, &QExpandingLineEdit::resizeToContents);
   updateMinimumWidth();
}

void QExpandingLineEdit::changeEvent(QEvent *e)
{
   switch (e->type()) {
      case QEvent::FontChange:
      case QEvent::StyleChange:
      case QEvent::ContentsRectChange:
         updateMinimumWidth();
         break;

      default:
         break;
   }

   QLineEdit::changeEvent(e);
}

void QExpandingLineEdit::updateMinimumWidth()
{
   int left, right;
   getTextMargins(&left, nullptr, &right, nullptr);
   int width = left + right + 4 /*horizontalMargin in qlineedit.cpp*/;
   getContentsMargins(&left, nullptr, &right, nullptr);
   width += left + right;

   QStyleOptionFrame opt;
   initStyleOption(&opt);

   int minWidth = style()->sizeFromContents(QStyle::CT_LineEdit, &opt, QSize(width, 0).
         expandedTo(QApplication::globalStrut()), this).width();

   setMinimumWidth(minWidth);
}

void QExpandingLineEdit::resizeToContents()
{
   int oldWidth = width();
   if (originalWidth == -1) {
      originalWidth = oldWidth;
   }

   if (QWidget *parent = parentWidget()) {
      QPoint position = pos();

      int hintWidth = minimumWidth() + fontMetrics().width(displayText());
      int parentWidth = parent->width();
      int maxWidth = isRightToLeft() ? position.x() + oldWidth : parentWidth - position.x();
      int newWidth = qBound(originalWidth, hintWidth, maxWidth);

      if (widgetOwnsGeometry) {
         setMaximumWidth(newWidth);
      }

      if (isRightToLeft()) {
         move(position.x() - newWidth + oldWidth, position.y());
      }
      resize(newWidth, height());
   }
}

#endif // QT_NO_LINEEDIT

#ifndef QT_NO_COMBOBOX

QBooleanComboBox::QBooleanComboBox(QWidget *parent)
   : QComboBox(parent)
{
   addItem(QComboBox::tr("False"));
   addItem(QComboBox::tr("True"));
}

void QBooleanComboBox::setValue(bool value)
{
   setCurrentIndex(value ? 1 : 0);
}

bool QBooleanComboBox::value() const
{
   return (currentIndex() == 1);
}

#endif // QT_NO_COMBOBOX


#endif // QT_NO_ITEMVIEWS
