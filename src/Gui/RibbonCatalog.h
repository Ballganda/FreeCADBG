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
 * Builds ribbon layouts where similar commands occupy the same tab/group slots
 * for every compatibility profile (Creo Inspired, SolidWorks Inspired, Native).
 */
class GuiExport RibbonCatalog
{
public:
    /** Shared skeleton filled per profile; slot order is identical across profiles. */
    static RibbonLayout layoutForProfile(const std::string& profileId);
};

}  // namespace Gui
