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

#include <qplatformdefs.h>
#include <qitemeditorfactory.h>
#include <qitemeditorfactory_p.h>

#ifndef QT_NO_ITEMVIEWS

#include <qcombobox.h>
#include <qdatetimeedit.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qspinbox.h>
#include <limits.h>
#include <float.h>
#include <qapplication.h>
#include <qdebug.h>

QT_BEGIN_NAMESPACE


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

#endif // QT_NO_COMBOBOX

/*!
    \class QItemEditorFactory
    \brief The QItemEditorFactory class provides widgets for editing item data
    in views and delegates.
    \since 4.2
    \ingroup model-view

    When editing data in an item view, editors are created and
    displayed by a delegate. QItemDelegate, which is the delegate by
    default installed on Qt's item views, uses a QItemEditorFactory to
    create editors for it. A default unique instance provided by
    QItemEditorFactory is used by all item delegates.  If you set a
    new default factory with setDefaultFactory(), the new factory will
    be used by existing and new delegates.

    A factory keeps a collection of QItemEditorCreatorBase
    instances, which are specialized editors that produce editors
    for one particular QVariant data type (All Qt models store
    their data in \l{QVariant}s).

    \section1 Standard Editing Widgets

    The standard factory implementation provides editors for a variety of data
    types. These are created whenever a delegate needs to provide an editor for
    data supplied by a model. The following table shows the relationship between
    types and the standard editors provided.

    \table
    \header \o Type \o Editor Widget
    \row    \o bool \o QComboBox
    \row    \o double \o QDoubleSpinBox
    \row    \o int \o{1,2} QSpinBox
    \row    \o unsigned int
    \row    \o QDate \o QDateEdit
    \row    \o QDateTime \o QDateTimeEdit
    \row    \o QPixmap \o QLabel
    \row    \o QString \o QLineEdit
    \row    \o QTime \o QTimeEdit
    \endtable

    Additional editors can be registered with the registerEditor() function.

    \sa QItemDelegate, {Model/View Programming}, {Color Editor Factory Example}
*/

/*!
    \fn QItemEditorFactory::QItemEditorFactory()

    Constructs a new item editor factory.
*/

/*!
    Creates an editor widget with the given \a parent for the specified \a type of data,
    and returns it as a QWidget.

    \sa registerEditor()
*/
QWidget *QItemEditorFactory::createEditor(QVariant::Type type, QWidget *parent) const
{
   QItemEditorCreatorBase *creator = creatorMap.value(type, 0);

   if (!creator) {
      const QItemEditorFactory *dfactory = defaultFactory();
      return dfactory == this ? 0 : dfactory->createEditor(type, parent);
   }
   return creator->createWidget(parent);
}

/*!
    Returns the property name used to access data for the given \a type of data.
*/
QString QItemEditorFactory::valuePropertyName(QVariant::Type type) const
{
   QItemEditorCreatorBase *creator = creatorMap.value(type, 0);

   if (!creator) {
      const QItemEditorFactory *dfactory = defaultFactory();
      return dfactory == this ? QString() : dfactory->valuePropertyName(type);
   }

   return creator->valuePropertyName();
}

/*!
    Destroys the item editor factory.
*/
QItemEditorFactory::~QItemEditorFactory()
{
   //we make sure we delete all the QItemEditorCreatorBase
   //this has to be done only once, hence the QSet
   QSet<QItemEditorCreatorBase *> set = creatorMap.values().toSet();
   qDeleteAll(set);
}

/*!
    Registers an item editor creator specified by \a creator for the given \a type of data.

    \bold{Note:} The factory takes ownership of the item editor creator and will destroy
    it if a new creator for the same type is registered later.

    \sa createEditor()
*/
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
         QSpinBox *sb = new QSpinBox(parent);
         sb->setFrame(false);
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
         le->setFrame(le->style()->styleHint(QStyle::SH_ItemView_DrawDelegateFrame, 0, le));
         if (!le->style()->styleHint(QStyle::SH_ItemView_ShowDecorationSelected, 0, le)) {
            le->setWidgetOwnsGeometry(true);
         }
         return le;
      }
#else
      default:
         break;
#endif
   }
   return 0;
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

static QItemEditorFactory *q_default_factory = 0;

struct QDefaultFactoryCleaner {
   inline QDefaultFactoryCleaner() {}

   ~QDefaultFactoryCleaner() {
      delete q_default_factory;
      q_default_factory = 0;
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

#ifndef QT_NO_LINEEDIT

QExpandingLineEdit::QExpandingLineEdit(QWidget *parent)
   : QLineEdit(parent), originalWidth(-1), widgetOwnsGeometry(false)
{
   connect(this, SIGNAL(textChanged(const QString &)), this, SLOT(resizeToContents()));
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
   getTextMargins(&left, 0, &right, 0);
   int width = left + right + 4 /*horizontalMargin in qlineedit.cpp*/;
   getContentsMargins(&left, 0, &right, 0);
   width += left + right;

   QStyleOptionFrameV2 opt;
   initStyleOption(&opt);

   int minWidth = style()->sizeFromContents(QStyle::CT_LineEdit, &opt, QSize(width, 0)
                  .expandedTo(QApplication::globalStrut()), this).width();

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

QT_END_NAMESPACE

#endif // QT_NO_ITEMVIEWS
