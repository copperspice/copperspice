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

#ifndef GSTREAMER_PHONONSRC_H
#define GSTREAMER_PHONONSRC_H

#include <sys/types.h>
#include <gst/gst.h>
#include <gst/base/gstbasesrc.h>
#include "streamreader.h"

QT_BEGIN_NAMESPACE

namespace Phonon
{
namespace Gstreamer
{

G_BEGIN_DECLS

#define GST_TYPE_PHONON_SRC \
  (phonon_src_get_type())
#define GST_PHONON_SRC(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_PHONON_SRC,PhononSrc))
#define GST_PHONON_SRC_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_PHONON_SRC,PhononSrcClass))
#define GST_IS_PHONON_SRC(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_PHONON_SRC))
#define GST_IS_PHONON_SRC_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_PHONON_SRC))

typedef struct _PhononSrc PhononSrc;
typedef struct _PhononSrcClass PhononSrcClass;

// PhononSrc:
struct _PhononSrc {
    GstBaseSrc element;
#ifndef QT_NO_PHONON_ABSTRACTMEDIASTREAM
    StreamReader *device;
#endif //QT_NO_PHONON_ABSTRACTMEDIASTREAM
};

struct _PhononSrcClass {
    GstBaseSrcClass parent_class;
};

GType phonon_src_get_type (void);

G_END_DECLS

}
} //namespace Phonon::Gstreamer

QT_END_NAMESPACE


#endif // __PHONON_SRC_H__
