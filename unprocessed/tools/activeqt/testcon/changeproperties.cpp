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

#include "changeproperties.h"

#include <QtGui>
#include <qt_windows.h>
#include <ActiveQt/ActiveQt>

QT_BEGIN_NAMESPACE

ChangeProperties::ChangeProperties(QWidget *parent)
: QDialog(parent), activex(0)
{
    setupUi(this);

    listProperties->setColumnCount(3);
    listProperties->headerItem()->setText(0, QLatin1String("Name"));
    listProperties->headerItem()->setText(1, QLatin1String("Type"));
    listProperties->headerItem()->setText(2, QLatin1String("Value"));

    listEditRequests->setColumnCount(1);
    listEditRequests->headerItem()->setText(0, QLatin1String("Name"));
}

void ChangeProperties::setControl(QAxWidget *ax)
{
    activex = ax;
    updateProperties();
}

void ChangeProperties::on_listProperties_currentItemChanged(QTreeWidgetItem *current)
{
    editValue->setEnabled(current != 0);
    buttonSet->setEnabled(current != 0);
    valueLabel->setEnabled(current != 0);
    
    if (!current)
	return;

    editValue->setText(current->text(2));
    QString prop = current->text(0);
    valueLabel->setText(prop + QLatin1String(" ="));

    const QMetaObject *mo = activex->metaObject();
    const QMetaProperty property = mo->property(mo->indexOfProperty(prop.toLatin1()));

    valueLabel->setEnabled(property.isWritable());
    editValue->setEnabled(property.isWritable());
    buttonSet->setEnabled(property.isWritable());
}

void ChangeProperties::on_buttonSet_clicked()
{
    QTreeWidgetItem *item = listProperties->currentItem();
    if (!item)
	return;
    
    QString prop = item->text(0);
    QVariant value = activex->property(prop.toLatin1());
    QVariant::Type type = value.type();
    if (!value.isValid()) {
	const QMetaObject *mo = activex->metaObject();
	const QMetaProperty property = mo->property(mo->indexOfProperty(prop.toLatin1()));
	type = QVariant::nameToType(property.typeName());
    }
    switch (type) {
    case QVariant::Color:
	{
	    QColor col;
	    col.setNamedColor(editValue->text());
	    if (col.isValid()) {
		value = QVariant::fromValue(col);
	    } else {
		QMessageBox::warning(this, tr("Can't parse input"), 
		                           tr("Failed to create a color from %1\n"
					                "The string has to be a valid color name (e.g. 'red')\n"
							"or a RGB triple of format '#rrggbb'."
							).arg(editValue->text()));
	    }
	}
	break;
    case QVariant::Font:
	{
	    QFont fnt;
	    if (fnt.fromString(editValue->text())) {
		value = QVariant::fromValue(fnt);
	    } else {
		QMessageBox::warning(this, tr("Can't parse input"), 
		                           tr("Failed to create a font from %1\n"
					        "The string has to have a format family,<point size> or\n"
						"family,pointsize,stylehint,weight,italic,underline,strikeout,fixedpitch,rawmode."
							).arg(editValue->text()));
	    }
	}
	break;
    case QVariant::Pixmap:
	{
	    QString fileName = editValue->text();
	    if (fileName.isEmpty())
		fileName = QFileDialog::getOpenFileName(this);
	    QPixmap pm(fileName);
	    if (pm.isNull())
		return;

	    value = QVariant::fromValue(pm);
	}
	break;
    case QVariant::Bool:
	{
	    QString txt = editValue->text().toLower();
	    value = QVariant(txt != QLatin1String("0") && txt != QLatin1String("false"));
	}
	break;
    case QVariant::List:
	{
	    QStringList txtList = editValue->text().split(QRegExp(QLatin1String("[,;]")));
	    QList<QVariant> varList;
	    for (int i = 0; i < txtList.count(); ++i) {
		QVariant svar(txtList.at(i));
		QString str = svar.toString();
		str = str.trimmed();
		bool ok;
		int n = str.toInt(&ok);
		if (ok) {
		    varList << n;
		    continue;
		}
		double d = str.toDouble(&ok);
		if (ok) {
		    varList << d;
		    continue;
		}
		varList << str;
	    }
	    value = varList;
	}
	break;

    default:
	value = editValue->text();
	break;
    }
 
    Q_ASSERT(activex->setProperty(prop.toLatin1(), value));
    updateProperties();
    listProperties->setCurrentItem(listProperties->findItems(prop, Qt::MatchExactly).at(0));
}

void ChangeProperties::on_listEditRequests_itemChanged(QTreeWidgetItem *item)
{
    if (!item)
	return;

    QString property = item->text(0);
    activex->setPropertyWritable(property.toLatin1(), item->checkState(0) == Qt::Checked);
}


void ChangeProperties::updateProperties()
{
    bool hasControl = activex && !activex->isNull();
    tabWidget->setEnabled(hasControl);

    listProperties->clear();
    listEditRequests->clear();
    if (hasControl) {
	const QMetaObject *mo = activex->metaObject();
	const int numprops = mo->propertyCount();
	for (int i = mo->propertyOffset(); i < numprops; ++i) {
	    const QMetaProperty property = mo->property(i);
	    QTreeWidgetItem *item = new QTreeWidgetItem(listProperties);
	    item->setText(0, QString::fromLatin1(property.name()));
	    item->setText(1, QString::fromLatin1(property.typeName()));
            if (!property.isDesignable()) {
                item->setTextColor(0, Qt::gray);
                item->setTextColor(1, Qt::gray);
                item->setTextColor(2, Qt::gray);
            }
	    QVariant var = activex->property(property.name());
	    
	    switch (var.type()) {
	    case QVariant::Color:
		{
		    QColor col = qvariant_cast<QColor>(var);
		    item->setText(2, col.name());
		}
		break;
	    case QVariant::Font:
		{
		    QFont fnt = qvariant_cast<QFont>(var);
		    item->setText(2, fnt.toString());
		}
		break;
	    case QVariant::Bool:
		{
		    item->setText(2, var.toBool() ? QLatin1String("true") : QLatin1String("false"));
		}
		break;
	    case QVariant::Pixmap:
		{
		    QPixmap pm = qvariant_cast<QPixmap>(var);
		    item->setIcon(2, pm);
		}
		break;
	    case QVariant::List:
		{
		    QList<QVariant> varList = var.toList();
		    QStringList strList;
		    for (int i = 0; i < varList.count(); ++i) {
			QVariant var = varList.at(i);
			strList << var.toString();
		    }
		    item->setText(2, strList.join(QLatin1String(", ")));
		}
		break;
	    case QVariant::Int:
		if (property.isEnumType()) {
		    const QMetaEnum enumerator = mo->enumerator(mo->indexOfEnumerator(property.typeName()));
		    item->setText(2, QString::fromLatin1(enumerator.valueToKey(var.toInt())));
		    break;
		}
		//FALLTHROUGH
	    default:
		item->setText(2, var.toString());
		break;
	    }

            bool requesting = false;
#if 0
            {
                void *argv[] = { &requesting };
                activex->qt_metacall(QMetaObject::Call(0x10000000) /*RequestingEdit*/, i, argv);
            }
#endif
            if (requesting) {
		QTreeWidgetItem *check = new QTreeWidgetItem(listEditRequests);
                check->setText(0, QString::fromLatin1(property.name()));
                check->setCheckState(0, activex->propertyWritable(property.name()) ? Qt::Checked : Qt::Unchecked);
	    }
	}
        listProperties->setCurrentItem(listProperties->topLevelItem(0));
    } else {
        editValue->clear();
    }
}

QT_END_NAMESPACE
