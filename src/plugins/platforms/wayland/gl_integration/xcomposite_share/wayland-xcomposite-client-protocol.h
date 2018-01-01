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

#ifndef XCOMPOSITE_CLIENT_PROTOCOL_H
#define XCOMPOSITE_CLIENT_PROTOCOL_H

#ifdef  __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>
#include "wayland-util.h"

struct wl_client;

struct wl_xcomposite;

extern const struct wl_interface wl_xcomposite_interface;

struct wl_xcomposite_listener {
	void (*root)(void *data,
		     struct wl_xcomposite *wl_xcomposite,
		     const char *display_name,
		     uint32_t root_window);
};

static inline int
wl_xcomposite_add_listener(struct wl_xcomposite *wl_xcomposite,
			      const struct wl_xcomposite_listener *listener, void *data)
{
	return wl_proxy_add_listener((struct wl_proxy *) wl_xcomposite,
				     (void (**)(void)) listener, data);
}

#define WL_XCOMPOSITE_CREATE_BUFFER	0

static inline struct wl_xcomposite *
wl_xcomposite_create(struct wl_display *display, uint32_t id, uint32_t version)
{
	wl_display_bind(display, id, "wl_xcomposite", version);

	return (struct wl_xcomposite *)
		wl_proxy_create_for_id(display, &wl_xcomposite_interface, id);
}

static inline void
wl_xcomposite_set_user_data(struct wl_xcomposite *wl_xcomposite, void *user_data)
{
	wl_proxy_set_user_data((struct wl_proxy *) wl_xcomposite, user_data);
}

static inline void *
wl_xcomposite_get_user_data(struct wl_xcomposite *wl_xcomposite)
{
	return wl_proxy_get_user_data((struct wl_proxy *) wl_xcomposite);
}

static inline void
wl_xcomposite_destroy(struct wl_xcomposite *wl_xcomposite)
{
	wl_proxy_destroy((struct wl_proxy *) wl_xcomposite);
}

static inline struct wl_buffer *
wl_xcomposite_create_buffer(struct wl_xcomposite *wl_xcomposite, uint32_t x_window, int width, int height, struct wl_visual *visual)
{
	struct wl_proxy *id;

	id = wl_proxy_create((struct wl_proxy *) wl_xcomposite,
			     &wl_buffer_interface);
	if (!id)
		return NULL;

	wl_proxy_marshal((struct wl_proxy *) wl_xcomposite,
			 WL_XCOMPOSITE_CREATE_BUFFER, id, x_window, width, height, visual);

	return (struct wl_buffer *) id;
}

#ifdef  __cplusplus
}
#endif

#endif
