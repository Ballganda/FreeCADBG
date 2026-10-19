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

/** One command entry (face label and/or split-menu item). */
struct RibbonCommandItem
{
    /** FreeCAD command name (e.g. Std_New). Empty means a reserved empty slot. */
    std::string commandName;
    /** Optional display label override; empty uses the command action text. */
    std::string label;
};

/** Visual size of a ribbon control. Large occupies the same footprint as a 2x2 of Small. */
enum class RibbonControlSize
{
    Large,
    Small,
};

/**
 * A control in a ribbon group.
 * - Button: single command; size Large or Small.
 * - SplitButton: primary click runs the active command; chevron menu picks/changes
 *   the active command (Creo-style). commands[0] is the initial active item.
 */
struct RibbonControlDef
{
    enum class Kind
    {
        Button,
        SplitButton,
    };

    Kind kind {Kind::Button};
    RibbonControlSize size {RibbonControlSize::Large};
    std::vector<RibbonCommandItem> commands;
    /** Stable id for remembering the last-selected split active command. */
    std::string splitId;
};

/** Named group of controls (left-to-right within a tab). */
struct RibbonGroupDef
{
    /** Stable id so the same group stays in the same place across profiles. */
    std::string slotId;
    std::string title;
    std::vector<RibbonControlDef> controls;
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
