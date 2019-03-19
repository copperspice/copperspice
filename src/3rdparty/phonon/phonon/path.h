/***********************************************************************
*
* Copyright (c) 2012-2019 Barbara Geller
* Copyright (c) 2012-2019 Ansel Sermersheim
*
* Copyright (C) 2005-2006 Matthias Kretz <kretz@kde.org>
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

#ifndef PHONON_PATH_H
#define PHONON_PATH_H

#include "phonon_export.h"
#include "objectdescription.h"
#include <QtCore/QExplicitlySharedDataPointer>

QT_BEGIN_NAMESPACE

template<class T> class QList;

namespace Phonon
{

class PathPrivate;
class Effect;
class MediaNode;

class PHONON_EXPORT Path
{
    friend class FactoryPrivate;
    public:
        /*
         * Destroys this reference to the Path. If the path was valid the connection is not broken
         * as both the source and the sink MediaNodes still keep a reference to the Path.
         *
         * \see disconnect
         */
        ~Path();

        /*
         * Creates an invalid path.
         *
         * You can still make it a valid path by calling reconnect. To create a path you should use
         * createPath, though.
         *
         * \see createPath
         * \see isValid
         */
        Path();

        /*
         * Constructs a copy of the given path.
         *
         * This constructor is fast thanks to explicit sharing.
         */
        Path(const Path &);

        /*
         * Returns whether the path object connects two MediaNodes or not.
         *
         * \return \p true when the path connects two MediaNodes
         * \return \p false when the path is disconnected
         */
        bool isValid() const;
        //MediaStreamTypes mediaStreamTypes() const;

#ifndef QT_NO_PHONON_EFFECT
        /*
         * Creates and inserts an effect into the path.
         *
         * You may insert effects of the same class as often as you like,
         * but if you insert the same object, the call will fail.
         *
         * \param desc The EffectDescription object for the effect to be inserted.
         *
         * \param insertBefore If you already inserted an effect you can
         * tell with this parameter in which order the data gets
         * processed. If this is \c 0 the effect is appended at the end of
         * the processing list. If the effect has not been inserted before
         * the method will do nothing and return \c false.
         *
         * \return Returns a pointer to the effect object if it could be inserted
         * at the specified position. If \c 0 is returned the effect was not
         * inserted.
         *
         * \see removeEffect
         * \see effects
         */
        Effect *insertEffect(const EffectDescription &desc, Effect *insertBefore = 0);

        /*
         * Inserts an effect into the path.
         *
         * You may insert effects of the same class as often as you like,
         * but if you insert the same object, the call will fail.
         *
         * \param newEffect An Effect object.
         *
         * \param insertBefore If you already inserted an effect you can
         * tell with this parameter in which order the data gets
         * processed. If this is \c 0 the effect is appended at the end of
         * the processing list. If the effect has not been inserted before
         * the method will do nothing and return \c false.
         *
         * \return Returns whether the effect could be inserted at the
         * specified position. If \c false is returned the effect was not
         * inserted.
         *
         * \see removeEffect
         * \see effects
         */
        bool insertEffect(Effect *newEffect, Effect *insertBefore = 0);

        /*
         * Removes an effect from the path.
         *
         * If the effect gets deleted while it is still connected the effect
         * will be removed automatically.
         *
         * \param effect The effect to be removed.
         *
         * \return Returns whether the call was successful. If it returns
         * \c false the effect could not be found in the path, meaning it
         * has not been inserted before.
         *
         * \see insertEffect
         * \see effects
         */
        bool removeEffect(Effect *effect);

        /*
         * Returns a list of Effect objects that are currently
         * used as effects. The order in the list determines the order the
         * signal is sent through the effects.
         *
         * \return A list with all current effects.
         *
         * \see insertEffect
         * \see removeEffect
         */
        QList<Effect *> effects() const;
#endif //QT_NO_PHONON_EFFECT

        /*
         * Tries to change the MediaNodes the path is connected to.
         *
         * If reconnect fails the old connection is kept.
         */
        bool reconnect(MediaNode *source, MediaNode *sink);

        /*
         * Disconnects the path from the MediaNodes it was connected to. This invalidates the path
         * (isValid returns \p false then).
         */
        bool disconnect();

        /*
         * Assigns \p p to this Path and returns a reference to this Path.
         *
         * This operation is fast thanks to explicit sharing.
         */
        Path &operator=(const Path &p);

        /*
         * Returns \p true if this Path is equal to \p p; otherwise returns \p false;
         */
        bool operator==(const Path &p) const;

        /*
         * Returns \p true if this Path is not equal to \p p; otherwise returns \p false;
         */
        bool operator!=(const Path &p) const;

        /*
         * Returns the source MediaNode used by the path.
         */
        MediaNode *source() const;

        /*
         * Returns the sink MediaNode used by the path.
         */
        MediaNode *sink() const;


    protected:
        friend class PathPrivate;
        QExplicitlySharedDataPointer<PathPrivate> d;
};

/*
 * \relates Path
 * Creates a new Path connecting two MediaNodes.
 *
 * The implementation will automatically select the right format and media type. E.g. connecting a
 * MediaObject and AudioOutput will create a Path object connecting the audio. This might be
 * represented as PCM or perhaps even AC3 depending on the AudioOutput object.
 *
 * \param source The MediaNode to connect an output from
 * \param sink The MediaNode to connect to.
 */
PHONON_EXPORT Path createPath(MediaNode *source, MediaNode *sink);

} // namespace Phonon

QT_END_NAMESPACE

#endif // PHONON_PATH_H
