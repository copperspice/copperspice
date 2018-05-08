/*
 * Copyright (C) 2006 Zack Rusin <zack@kde.org>
 * Copyright (C) 2007 Staikos Computing Services Inc. <info@staikos.net>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE COMPUTER, INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE COMPUTER, INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "ContextMenuItem.h"

#include "ContextMenu.h"

namespace WebCore {

ContextMenuItem::ContextMenuItem(ContextMenu* subMenu)
{
    platformDescription().type = SubmenuType;
    platformDescription().action = ContextMenuItemTagNoAction;

    if (subMenu)
        setSubMenu(subMenu);
}

ContextMenuItem::ContextMenuItem(ContextMenuItemType type, ContextMenuAction action,
                                 const String& title, ContextMenu* subMenu)
{
    platformDescription().type = type;
    platformDescription().action = action;
    platformDescription().title = title;

    if (subMenu)
        setSubMenu(subMenu);
}

ContextMenuItem::ContextMenuItem(ContextMenuItemType, ContextMenuAction, const String&, bool, bool)
{
    // FIXME: Implement
}

ContextMenuItem::ContextMenuItem(ContextMenuAction, const String&, bool, bool, Vector<ContextMenuItem>&)
{
    // FIXME: Implement
}

ContextMenuItem::~ContextMenuItem()
{
}

#if PLATFORM(QT)

ContextMenuItem::ContextMenuItem(const ContextMenuItem & other)
{
    m_platformDescription.reset(new PlatformMenuItemDescription(*other.m_platformDescription));
}

ContextMenuItem& ContextMenuItem::operator=(const ContextMenuItem & other)
{
    m_platformDescription.reset(new PlatformMenuItemDescription(*other.m_platformDescription));
   return *this;
}

ContextMenuItem& ContextMenuItem::operator=(ContextMenuItem && other)
{
    m_platformDescription.swap(other.m_platformDescription);
    other.m_platformDescription.reset();
    return *this;
}

#endif

PlatformMenuItemDescription ContextMenuItem::releasePlatformDescription()
{
    return platformDescription();
}

ContextMenuItemType ContextMenuItem::type() const
{
    return platformDescription().type;
}

void ContextMenuItem::setType(ContextMenuItemType type)
{
    platformDescription().type = type;
}

ContextMenuAction ContextMenuItem::action() const
{
    return platformDescription().action;
}

void ContextMenuItem::setAction(ContextMenuAction action)
{
    platformDescription().action = action;
}

String ContextMenuItem::title() const
{
    return platformDescription().title;
}

void ContextMenuItem::setTitle(const String& title)
{
    platformDescription().title = title;
}


PlatformMenuDescription ContextMenuItem::platformSubMenu() const
{
    return &platformDescription().subMenuItems;
}

void ContextMenuItem::setSubMenu(ContextMenu* menu)
{
    platformDescription().subMenuItems = *menu->platformDescription();
}

void ContextMenuItem::setSubMenu(Vector<ContextMenuItem>&)
{
    // FIXME: Implement
}

void ContextMenuItem::setChecked(bool on)
{
    platformDescription().checked = on;
}

bool ContextMenuItem::checked() const
{
    // FIXME - Implement
    return false;
}

void ContextMenuItem::setEnabled(bool on)
{
    platformDescription().enabled = on;
}

bool ContextMenuItem::enabled() const
{
    return platformDescription().enabled;
}

PlatformMenuItemDescription& ContextMenuItem::platformDescription()
{
#if PLATFORM(QT)
    if(!m_platformDescription) {
	m_platformDescription.reset(new PlatformMenuItemDescription());
    }
    return *m_platformDescription;
#else
    return m_platformDescription;
#endif
}

const PlatformMenuItemDescription& ContextMenuItem::platformDescription() const
{
#if PLATFORM(QT)
    if(!m_platformDescription) {
	m_platformDescription.reset(new PlatformMenuItemDescription());
    }
    return *m_platformDescription;
#else
    return m_platformDescription;
#endif
}

}
// vim: ts=4 sw=4 et
