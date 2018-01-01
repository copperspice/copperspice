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

/********************************************************
**  Copyright (C) 2005-2006 Matthias Kretz <kretz@kde.org
********************************************************/

#ifndef PHONON_PHONONDEFS_H
#define PHONON_PHONONDEFS_H

#include <QtCore/QtGlobal>
#include "phonon_export.h"

QT_BEGIN_NAMESPACE

#ifdef PHONON_BACKEND_VERSION_4_4
# ifndef PHONON_BACKEND_VERSION_4_3
#  define PHONON_BACKEND_VERSION_4_3
# endif
#endif
#ifdef PHONON_BACKEND_VERSION_4_3
# ifndef PHONON_BACKEND_VERSION_4_2
#  define PHONON_BACKEND_VERSION_4_2
# endif
#endif

// the following inlines are correct - exclude per line doesn't work for multiline-macros so exclude
// the whole file for inline checks
//krazy:excludeall=inline
#define K_DECLARE_PRIVATE(Class) \
    inline Class##Private* k_func() { return reinterpret_cast<Class##Private *>(k_ptr); } \
    inline const Class##Private* k_func() const { return reinterpret_cast<const Class##Private *>(k_ptr); } \
    friend class Class##Private;

/**
 * \internal
 * Used in class declarations to provide the needed functions. This is used for
 * abstract base classes.
 *
 * \param classname The Name of the class this macro is used for.
 *
 * Example:
 * \code
 * class AbstractEffect : public QObject
 * {
 *   Q _OBJECT
 *   Q_PROPERTY(int propertyA READ propertyA WRITE setPropertyA)
 *   PHONON_ABSTRACTBASE(AbstractEffect)
 *   public:
 *     int propertyA() const;
 *     void setPropertyA(int);
 * };
 * \endcode
 *
 * \see PHONON_OBJECT
 * \see PHONON_HEIR
 */
#define PHONON_ABSTRACTBASE(classname) \
protected: \
    /**
     * \internal
     * Constructor that is called from derived classes.
     *
     * \param parent Standard QObject parent.
     */ \
    classname(classname ## Private &dd, QObject *parent); \
private:

/**
 * \internal
 * Used in class declarations to provide the needed functions. This is used for
 * classes that inherit QObject directly.
 *
 * \param classname The Name of the class this macro is used for.
 *
 * Example:
 * \code
 * class EffectSettings : public QObject
 * {
 *   Q _OBJECT
 *   Q_PROPERTY(int propertyA READ propertyA WRITE setPropertyA)
 *   PHONON_OBJECT(EffectSettings)
 *   public:
 *     int propertyA() const;
 *     void setPropertyA(int);
 * };
 * \endcode
 *
 * \see PHONON_ABSTRACTBASE
 * \see PHONON_HEIR
 */
#define PHONON_OBJECT(classname) \
public: \
    /**
     * Constructs an object with the given \p parent.
     */ \
    classname(QObject *parent = nullptr); \
private:

/**
 * \internal
 * Used in class declarations to provide the needed functions. This is used for
 * classes that inherit another Phonon object.
 *
 * \param classname The Name of the class this macro is used for.
 *
 * Example:
 * \code
 * class ConcreteEffect : public AbstractEffect
 * {
 *   Q _OBJECT
 *   Q_PROPERTY(int propertyB READ propertyB WRITE setPropertyB)
 *   PHONON_HEIR(ConcreteEffect)
 *   public:
 *     int propertyB() const;
 *     void setPropertyB(int);
 * };
 * \endcode
 *
 * \see PHONON_ABSTRACTBASE
 * \see PHONON_OBJECT
 */
#define PHONON_HEIR(classname) \
public: \
    /**
     * Constructs an object with the given \p parent.
     */ \
    classname(QObject *parent = nullptr); \

QT_END_NAMESPACE

#endif // PHONONDEFS_H
