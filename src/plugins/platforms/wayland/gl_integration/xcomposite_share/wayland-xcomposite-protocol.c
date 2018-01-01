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

#include <stdlib.h>
#include <stdint.h>
#include "wayland-util.h"

static const struct wl_message wl_xcomposite_requests[] = {
	{ "create_buffer", "nuiio" },
};

static const struct wl_message wl_xcomposite_events[] = {
	{ "root", "su" },
};

WL_EXPORT const struct wl_interface wl_xcomposite_interface = {
	"wl_xcomposite", 1,
	ARRAY_LENGTH(wl_xcomposite_requests), wl_xcomposite_requests,
	ARRAY_LENGTH(wl_xcomposite_events), wl_xcomposite_events,
};

