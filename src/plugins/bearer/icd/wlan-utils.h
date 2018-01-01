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

#ifndef WLAN_UTILS_H
#define WLAN_UTILS_H

/** Originally taken from: libicd-network-wlan-dev.h*/

#include <glib.h>
#include <dbus/dbus.h>
#include <wlancond.h>
#include <icd/network_api_defines.h>

/* capability bits inside network attributes var */
#define NWATTR_WPS_MASK       0x0000F000
#define NWATTR_ALGORITHM_MASK 0x00000F00
#define NWATTR_WPA2_MASK      0x00000080
#define NWATTR_METHOD_MASK    0x00000078
#define NWATTR_MODE_MASK      0x00000007

#define CAP_LOCALMASK         0x0FFFE008

/* how much to shift between capability and network attributes var */
#define CAP_SHIFT_WPS        3
#define CAP_SHIFT_ALGORITHM 20
#define CAP_SHIFT_WPA2       1
#define CAP_SHIFT_METHOD     1
#define CAP_SHIFT_MODE       0
#define CAP_SHIFT_ALWAYS_ONLINE 26

/* ------------------------------------------------------------------------- */
/* From combined to capability */
static inline dbus_uint32_t nwattr2cap(guint nwattrs, dbus_uint32_t *cap)
{
	guint oldval = *cap;

	*cap &= CAP_LOCALMASK; /* clear old capabilities */
	*cap |=
		((nwattrs & ICD_NW_ATTR_ALWAYS_ONLINE) >> CAP_SHIFT_ALWAYS_ONLINE) |
		((nwattrs & NWATTR_WPS_MASK) >> CAP_SHIFT_WPS) |
		((nwattrs & NWATTR_ALGORITHM_MASK) << CAP_SHIFT_ALGORITHM) |
		((nwattrs & NWATTR_WPA2_MASK) << CAP_SHIFT_WPA2) |
		((nwattrs & NWATTR_METHOD_MASK) << CAP_SHIFT_METHOD) |
		(nwattrs & NWATTR_MODE_MASK);

	return oldval;
}


/* ------------------------------------------------------------------------- */
/* From capability to combined */
static inline guint cap2nwattr(dbus_uint32_t cap, guint *nwattrs)
{
	guint oldval = *nwattrs;

	*nwattrs &= ~ICD_NW_ATTR_LOCALMASK; /* clear old capabilities */
        *nwattrs |=
#ifdef WLANCOND_WPS_MASK
		((cap & WLANCOND_WPS_MASK) << CAP_SHIFT_WPS) |
#endif
		((cap & (WLANCOND_ENCRYPT_ALG_MASK |
			 WLANCOND_ENCRYPT_GROUP_ALG_MASK)) >> CAP_SHIFT_ALGORITHM)|
		((cap & WLANCOND_ENCRYPT_WPA2_MASK) >> CAP_SHIFT_WPA2) |
		((cap & WLANCOND_ENCRYPT_METHOD_MASK) >> CAP_SHIFT_METHOD) |
		(cap & WLANCOND_MODE_MASK);

	return oldval;
}


#endif
