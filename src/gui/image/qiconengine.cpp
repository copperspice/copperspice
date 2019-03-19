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

#include <qiconengine.h>
#include <qpainter.h>

QT_BEGIN_NAMESPACE

QSize QIconEngine::actualSize(const QSize &size, QIcon::Mode /*mode*/, QIcon::State /*state*/)
{
   return size;
}

QIconEngine::~QIconEngine()
{
}

QPixmap QIconEngine::pixmap(const QSize &size, QIcon::Mode mode, QIcon::State state)
{
   QPixmap pm(size);
   {
      QPainter p(&pm);
      paint(&p, QRect(QPoint(0, 0), size), mode, state);
   }
   return pm;
}

/*!
  Called by QIcon::addPixmap(). Adds a specialized \a pixmap for the given
  \a mode and \a state. The default pixmap-based engine stores any supplied
  pixmaps, and it uses them instead of scaled pixmaps if the size of a pixmap
  matches the size of icon requested. Custom icon engines that implement
  scalable vector formats are free to ignores any extra pixmaps.
 */
void QIconEngine::addPixmap(const QPixmap &/*pixmap*/, QIcon::Mode /*mode*/, QIcon::State /*state*/)
{
}


/*!  Called by QIcon::addFile(). Adds a specialized pixmap from the
  file with the given \a fileName, \a size, \a mode and \a state. The
  default pixmap-based engine stores any supplied file names, and it
  loads the pixmaps on demand instead of using scaled pixmaps if the
  size of a pixmap matches the size of icon requested. Custom icon
  engines that implement scalable vector formats are free to ignores
  any extra files.
 */
void QIconEngine::addFile(const QString &/*fileName*/, const QSize &/*size*/, QIcon::Mode /*mode*/,
                          QIcon::State /*state*/)
{
}



// version 2 functions


/*!
    \class QIconEngineV2

    \brief The QIconEngineV2 class provides an abstract base class for QIcon renderers.

    \ingroup painting
    \since 4.3

    An icon engine renders \l{QIcon}s. With icon engines, you can
    customize icons. Qt provides a default engine that makes icons
    adhere to the current style by scaling the icons and providing a
    disabled appearance.

    An engine is installed on an icon either through a QIcon
    constructor or through a QIconEnginePluginV2. The plugins are used
    by Qt if a specific engine is not given when the icon is created.
    See the QIconEngineV2 class description to learn how to create
    icon engine plugins.

    An icon engine provides the rendering functions for a QIcon. Each
    icon has a corresponding icon engine that is responsible for drawing
    the icon with a requested size, mode and state.

    QIconEngineV2 extends the API of QIconEngine to allow streaming of
    the icon engine contents, and should be used instead of QIconEngine
    for implementing new icon engines.

    \sa QIconEnginePluginV2

*/

/*!
    \enum QIconEngineV2::IconEngineHook
    \since 4.5

    These enum values are used for virtual_hook() to allow additional
    queries to icon engine without breaking binary compatibility.

    \value AvailableSizesHook Allows to query the sizes of the
    contained pixmaps for pixmap-based engines. The \a data argument
    of the virtual_hook() function is a AvailableSizesArgument pointer
    that should be filled with icon sizes. Engines that work in terms
    of a scalable, vectorial format normally return an empty list.

    \value IconNameHook Allows to query the name used to create the
    icon, for example when instantiating an icon using
    QIcon::fromTheme().

    \sa virtual_hook()
 */

/*!
    \class QIconEngineV2::AvailableSizesArgument
    \since 4.5

    This struct represents arguments to virtual_hook() function when
    \a id parameter is QIconEngineV2::AvailableSizesHook.

    \sa virtual_hook(), QIconEngineV2::IconEngineHook
 */

/*!
    \variable QIconEngineV2::AvailableSizesArgument::mode
    \brief the requested mode of an image.

    \sa QIcon::Mode
*/

/*!
    \variable QIconEngineV2::AvailableSizesArgument::state
    \brief the requested state of an image.

    \sa QIcon::State
*/

/*!
    \variable QIconEngineV2::AvailableSizesArgument::sizes

    \brief image sizes that are available with specified \a mode and
    \a state. This is an output parameter and is filled after call to
    virtual_hook(). Engines that work in terms of a scalable,
    vectorial format normally return an empty list.
*/


/*!
    Returns a key that identifies this icon engine.
 */
QString QIconEngineV2::key() const
{
   return QString();
}

/*!
    Returns a clone of this icon engine.
 */
QIconEngineV2 *QIconEngineV2::clone() const
{
   return 0;
}

/*!
    Reads icon engine contents from the QDataStream \a in. Returns
    true if the contents were read; otherwise returns false.

    QIconEngineV2's default implementation always return false.
 */
bool QIconEngineV2::read(QDataStream &)
{
   return false;
}

/*!
    Writes the contents of this engine to the QDataStream \a out.
    Returns true if the contents were written; otherwise returns false.

    QIconEngineV2's default implementation always return false.
 */
bool QIconEngineV2::write(QDataStream &) const
{
   return false;
}

/*!
    \since 4.5

    Additional method to allow extending QIconEngineV2 without
    adding new virtual methods (and without breaking binary compatibility).
    The actual action and format of \a data depends on \a id argument
    which is in fact a constant from IconEngineHook enum.

    \sa IconEngineHook
*/
void QIconEngineV2::virtual_hook(int id, void *data)
{
   switch (id) {
      case QIconEngineV2::AvailableSizesHook: {
         QIconEngineV2::AvailableSizesArgument &arg =
            *reinterpret_cast<QIconEngineV2::AvailableSizesArgument *>(data);
         arg.sizes.clear();
         break;
      }
      default:
         break;
   }
}

/*!
    \since 4.5

    Returns sizes of all images that are contained in the engine for the
    specific \a mode and \a state.

    \note This is a helper method and the actual work is done by
    virtual_hook() method, hence this method depends on icon engine support
    and may not work with all icon engines.
 */
QList<QSize> QIconEngineV2::availableSizes(QIcon::Mode mode, QIcon::State state)
{
   AvailableSizesArgument arg;
   arg.mode = mode;
   arg.state = state;
   virtual_hook(QIconEngineV2::AvailableSizesHook, reinterpret_cast<void *>(&arg));
   return arg.sizes;
}

/*!
    \since 4.7

    Returns the name used to create the engine, if available.

    \note This is a helper method and the actual work is done by
    virtual_hook() method, hence this method depends on icon engine support
    and may not work with all icon engines.
 */
QString QIconEngineV2::iconName()
{
   QString name;
   virtual_hook(QIconEngineV2::IconNameHook, reinterpret_cast<void *>(&name));
   return name;
}

QT_END_NAMESPACE
