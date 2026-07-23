/***********************************************************************
*
* Copyright (c) 2012-2026 Barbara Geller
* Copyright (c) 2012-2026 Ansel Sermersheim
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

#include <qaccessiblebridgeutils_p.h>

#include <qmath.h>

#ifndef QT_NO_ACCESSIBILITY

namespace QAccessibleBridgeUtils {

static bool performAction(QAccessibleInterface *iface, const QString &actionName)
{
   if (QAccessibleActionInterface *actionIface = iface->actionInterface()) {
      if (actionIface->actionNames().contains(actionName)) {
         actionIface->doAction(actionName);
         return true;
      }
   }

   return false;
}

QStringList effectiveActionNames(QAccessibleInterface *iface)
{
    QStringList retval;

    if (QAccessibleActionInterface *actionIface = iface->actionInterface()) {
        retval = actionIface->actionNames();
    }

    if (iface->valueInterface()) {
        if (! retval.contains(QAccessibleActionInterface::increaseAction())) {
            retval << QAccessibleActionInterface::increaseAction();
        }

        if (! retval.contains(QAccessibleActionInterface::decreaseAction())) {
            retval << QAccessibleActionInterface::decreaseAction();
        }
    }

    return retval;
}

bool performEffectiveAction(QAccessibleInterface *iface, const QString &actionName)
{
    if (! iface) {
       return false;
    }

    if (performAction(iface, actionName)) {
       return true;
    }

    if (actionName != QAccessibleActionInterface::increaseAction() && actionName != QAccessibleActionInterface::decreaseAction()) {
       return false;
    }

    QAccessibleValueInterface *valueIface = iface->valueInterface();

    if (! valueIface) {
        return false;
    }

    bool success;

    const QVariant currentVariant = valueIface->currentValue();
    double stepSize = valueIface->minimumStepSize().toDouble(&success);

    if (! success || qFuzzyIsNull(stepSize)) {
        const double min = valueIface->minimumValue().toDouble(&success);

        if (! success) {
            return false;
        }

        const double max = valueIface->maximumValue().toDouble(&success);

        if (! success) {
            return false;
        }

        stepSize = (max - min) / 10;  // this is pretty arbitrary, we just need to provide something
        const int typ = currentVariant.type();

        if (typ != QVariant::Float && typ != QVariant::Double) {
            // currentValue is an integer. Round it up to ensure stepping in case it was below 1
            stepSize = qCeil(stepSize);
        }
    }

    const double current = currentVariant.toDouble(&success);

    if (! success) {
       return false;
    }

    if (actionName == QAccessibleActionInterface::decreaseAction()) {
        stepSize = -stepSize;
    }

    valueIface->setCurrentValue(current + stepSize);

    return true;
}

}   //namespace

#endif
