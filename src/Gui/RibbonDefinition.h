// SPDX-License-Identifier: LGPL-2.1-or-later
/***************************************************************************
 *   Copyright (c) 2026 FreeCAD contributors                               *
 *                                                                         *
 *   This file is part of FreeCAD.                                         *
 ***************************************************************************/

#pragma once

#include <string>
#include <vector>

namespace Gui
{

/** One command button in a ribbon group. */
struct RibbonCommandItem
{
    /** FreeCAD command name (e.g. Std_New). Empty means a reserved empty slot. */
    std::string commandName;
    /** Optional display label override; empty uses the command action text. */
    std::string label;
};

/** Named group of commands (left-to-right within a tab). */
struct RibbonGroupDef
{
    /** Stable id so the same group stays in the same place across profiles. */
    std::string slotId;
    std::string title;
    std::vector<RibbonCommandItem> commands;
};

/** One ribbon tab. */
struct RibbonTabDef
{
    std::string id;
    std::string title;
    std::vector<RibbonGroupDef> groups;
};

/** Full ribbon definition for a compatibility profile. */
struct RibbonLayout
{
    std::string profileId;
    std::vector<RibbonTabDef> tabs;
};

}  // namespace Gui
