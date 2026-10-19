// SPDX-License-Identifier: LGPL-2.1-or-later
/***************************************************************************
 *   Copyright (c) 2026 FreeCAD contributors                               *
 *                                                                         *
 *   This file is part of FreeCAD.                                         *
 ***************************************************************************/

#pragma once

#include <string>

#include "RibbonDefinition.h"
#include <FCGlobal.h>

namespace Gui
{

/**
 * Builds per-profile ribbon layouts.
 * Creo Inspired and SolidWorks Inspired each use their own tab order and
 * left-to-right group flow (not a shared skeleton).
 */
class GuiExport RibbonCatalog
{
public:
    /** Ribbon definition for the given compatibility profile id. */
    static RibbonLayout layoutForProfile(const std::string& profileId);
};

}  // namespace Gui
