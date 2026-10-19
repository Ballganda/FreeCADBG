// SPDX-License-Identifier: LGPL-2.1-or-later
/***************************************************************************
 *   Copyright (c) 2026 FreeCAD contributors                               *
 *                                                                         *
 *   This file is part of FreeCAD.                                         *
 ***************************************************************************/

#include "RibbonCatalog.h"

namespace Gui
{
namespace
{

constexpr const char* NativeId = "FreeCADNative";
constexpr const char* CreoId = "CreoInspired";
constexpr const char* SolidWorksId = "SolidWorksInspired";

RibbonCommandItem cmd(const char* name, const char* label = "")
{
    return {name, label ? label : ""};
}

/**
 * Fixed skeleton: same tabs, same group slotIds, same FreeCAD commands in the
 * same button indices. Only labels differ so Creo/SW users see familiar names
 * while clicks stay in the same places.
 */
RibbonLayout sharedSkeleton(const std::string& profileId, bool solidWorksLabels, bool creoLabels)
{
    auto L = [&](const char* native, const char* creo, const char* sw) -> const char* {
        if (solidWorksLabels) {
            return sw;
        }
        if (creoLabels) {
            return creo;
        }
        return native;
    };

    RibbonLayout layout;
    layout.profileId = profileId;

    RibbonTabDef home;
    home.id = "home";
    home.title = "Home";
    home.groups = {
        {"file",
         "File",
         {
             cmd("Std_New", L("New", "New", "New")),
             cmd("Std_Open", L("Open", "Open", "Open")),
             cmd("Std_Save", L("Save", "Save", "Save")),
         }},
        {"edit",
         "Edit",
         {
             cmd("Std_Undo", L("Undo", "Undo", "Undo")),
             cmd("Std_Redo", L("Redo", "Redo", "Redo")),
             cmd("Std_Delete", L("Delete", "Delete", "Delete")),
         }},
        {"view_quick",
         "View",
         {
             cmd("Std_ViewFitAll", L("Fit All", "Fit", "Zoom to Fit")),
             cmd("Std_ViewIsometric", L("Isometric", "Isometric", "Isometric")),
         }},
        {"model",
         "Model",
         {
             cmd("Sketcher_NewSketch", L("New Sketch", "Sketch", "Sketch")),
             cmd("PartDesign_Body", L("Body", "Body", "Boss/Base (Body)")),
         }},
        {"options",
         "Options",
         {
             cmd("Std_DlgPreferences", L("Preferences", "Preferences", "Options")),
         }},
    };
    layout.tabs.push_back(home);

    RibbonTabDef view;
    view.id = "view";
    view.title = "View";
    view.groups = {
        {"standard_views",
         "Standard Views",
         {
             cmd("Std_ViewFront", L("Front", "Front", "Front")),
             cmd("Std_ViewTop", L("Top", "Top", "Top")),
             cmd("Std_ViewRight", L("Right", "Right", "Right")),
             cmd("Std_ViewIsometric", L("Isometric", "Isometric", "Isometric")),
             cmd("Std_ViewFitAll", L("Fit All", "Fit", "Zoom to Fit")),
         }},
        {"display",
         "Display",
         {
             cmd("Std_DrawStyle", L("Draw Style", "Display Style", "Display Style")),
             cmd("Std_ViewFullscreen", L("Fullscreen", "Fullscreen", "Fullscreen")),
         }},
    };
    layout.tabs.push_back(view);

    RibbonTabDef model;
    model.id = "model";
    model.title = "Model";
    model.groups = {
        {"sketch",
         "Sketch",
         {
             cmd("Sketcher_NewSketch", L("New Sketch", "Sketch", "Sketch")),
             cmd("Sketcher_LeaveSketch", L("Close Sketch", "Close Sketch", "Exit Sketch")),
         }},
        {"features",
         "Features",
         {
             cmd("PartDesign_Pad", L("Pad", "Extrude", "Extruded Boss/Base")),
             cmd("PartDesign_Pocket", L("Pocket", "Cut", "Extruded Cut")),
             cmd("PartDesign_Revolution", L("Revolution", "Revolve", "Revolved Boss/Base")),
             cmd("PartDesign_Fillet", L("Fillet", "Round", "Fillet")),
             cmd("PartDesign_Chamfer", L("Chamfer", "Chamfer", "Chamfer")),
         }},
        {"boolean",
         "Boolean",
         {
             cmd("Part_Fuse", L("Union", "Merge", "Combine")),
             cmd("Part_Cut", L("Cut", "Cut", "Subtract")),
             cmd("Part_Common", L("Common", "Intersect", "Intersect")),
         }},
    };
    layout.tabs.push_back(model);

    return layout;
}

}  // namespace

RibbonLayout RibbonCatalog::layoutForProfile(const std::string& profileId)
{
    if (profileId == CreoId) {
        return sharedSkeleton(profileId, false, true);
    }
    if (profileId == SolidWorksId) {
        return sharedSkeleton(profileId, true, false);
    }
    return sharedSkeleton(NativeId, false, false);
}

}  // namespace Gui
