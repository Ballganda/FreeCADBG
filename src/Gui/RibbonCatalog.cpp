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

RibbonGroupDef group(const char* slotId, const char* title, std::vector<RibbonCommandItem> commands)
{
    return {slotId, title, std::move(commands)};
}

/**
 * Creo-inspired flow (part mode):
 * Model → Analysis → Annotate → Tools → View
 * On Model: Get Data / Datums / Shapes / Editing (Creo-like left-to-right).
 */
RibbonLayout creoLayout()
{
    RibbonLayout layout;
    layout.profileId = CreoId;

    RibbonTabDef model;
    model.id = "creo_model";
    model.title = "Model";
    model.groups = {
        group(
            "creo_get_data",
            "Get Data",
            {
                cmd("Std_New", "New"),
                cmd("Std_Open", "Open"),
                cmd("Std_Save", "Save"),
            }
        ),
        group(
            "creo_datums",
            "Datums",
            {
                cmd("PartDesign_Body", "Body"),
                cmd("PartDesign_Plane", "Plane"),
                cmd("PartDesign_Line", "Axis"),
                cmd("PartDesign_Point", "Point"),
            }
        ),
        group(
            "creo_shapes",
            "Shapes",
            {
                cmd("Sketcher_NewSketch", "Sketch"),
                cmd("PartDesign_Pad", "Extrude"),
                cmd("PartDesign_Revolution", "Revolve"),
                cmd("PartDesign_AdditivePipe", "Sweep"),
                cmd("PartDesign_AdditiveLoft", "Blend"),
            }
        ),
        group(
            "creo_editing",
            "Editing",
            {
                cmd("PartDesign_Pocket", "Cut"),
                cmd("PartDesign_Fillet", "Round"),
                cmd("PartDesign_Chamfer", "Chamfer"),
                cmd("PartDesign_Mirrored", "Mirror"),
            }
        ),
        group(
            "creo_operations",
            "Operations",
            {
                cmd("Part_Fuse", "Merge"),
                cmd("Part_Cut", "Cut"),
                cmd("Part_Common", "Intersect"),
                cmd("Std_Delete", "Delete"),
            }
        ),
    };
    layout.tabs.push_back(model);

    RibbonTabDef analysis;
    analysis.id = "creo_analysis";
    analysis.title = "Analysis";
    analysis.groups = {
        group(
            "creo_measure",
            "Measure",
            {
                cmd("Std_Measure", "Measure"),
                cmd("Part_CheckGeometry", "Check Geometry"),
            }
        ),
    };
    layout.tabs.push_back(analysis);

    RibbonTabDef annotate;
    annotate.id = "creo_annotate";
    annotate.title = "Annotate";
    annotate.groups = {
        group(
            "creo_annotate",
            "Annotations",
            {
                cmd("Part_Section", "Section"),
                cmd("Std_SelBoundingBox", "Show Bound Box"),
            }
        ),
    };
    layout.tabs.push_back(annotate);

    RibbonTabDef tools;
    tools.id = "creo_tools";
    tools.title = "Tools";
    tools.groups = {
        group(
            "creo_edit",
            "Edit",
            {
                cmd("Std_Undo", "Undo"),
                cmd("Std_Redo", "Redo"),
            }
        ),
        group(
            "creo_options",
            "Utilities",
            {
                cmd("Std_DlgPreferences", "Options"),
                cmd("Std_DlgParameter", "Parameters"),
            }
        ),
    };
    layout.tabs.push_back(tools);

    RibbonTabDef view;
    view.id = "creo_view";
    view.title = "View";
    view.groups = {
        group(
            "creo_orient",
            "Orientation",
            {
                cmd("Std_ViewFront", "Front"),
                cmd("Std_ViewTop", "Top"),
                cmd("Std_ViewRight", "Right"),
                cmd("Std_ViewIsometric", "Default Orient"),
            }
        ),
        group(
            "creo_model_display",
            "Model Display",
            {
                cmd("Std_ViewFitAll", "Refit"),
                cmd("Std_DrawStyle", "Display Style"),
                cmd("Std_ViewFullscreen", "Fullscreen"),
            }
        ),
    };
    layout.tabs.push_back(view);

    return layout;
}

/**
 * SolidWorks-inspired flow (part mode):
 * Features (primary) → Sketch → Evaluate → View
 * On Features: Sketch | Features | Reference Geometry (SW left-to-right).
 */
RibbonLayout solidWorksLayout()
{
    RibbonLayout layout;
    layout.profileId = SolidWorksId;

    RibbonTabDef features;
    features.id = "sw_features";
    features.title = "Features";
    features.groups = {
        group(
            "sw_sketch_entry",
            "Sketch",
            {
                cmd("Sketcher_NewSketch", "Sketch"),
                cmd("Sketcher_LeaveSketch", "Exit Sketch"),
            }
        ),
        group(
            "sw_features",
            "Features",
            {
                cmd("PartDesign_Body", "Boss/Base"),
                cmd("PartDesign_Pad", "Extruded Boss/Base"),
                cmd("PartDesign_Pocket", "Extruded Cut"),
                cmd("PartDesign_Revolution", "Revolved Boss/Base"),
                cmd("PartDesign_Fillet", "Fillet"),
                cmd("PartDesign_Chamfer", "Chamfer"),
                cmd("PartDesign_Mirrored", "Mirror"),
                cmd("PartDesign_LinearPattern", "Linear Pattern"),
            }
        ),
        group(
            "sw_reference",
            "Reference Geometry",
            {
                cmd("PartDesign_Plane", "Plane"),
                cmd("PartDesign_Line", "Axis"),
                cmd("PartDesign_Point", "Point"),
            }
        ),
        group(
            "sw_boolean",
            "Combine",
            {
                cmd("Part_Fuse", "Combine"),
                cmd("Part_Cut", "Subtract"),
                cmd("Part_Common", "Common"),
            }
        ),
    };
    layout.tabs.push_back(features);

    RibbonTabDef sketch;
    sketch.id = "sw_sketch";
    sketch.title = "Sketch";
    sketch.groups = {
        group(
            "sw_sketch_tools",
            "Sketch",
            {
                cmd("Sketcher_NewSketch", "Sketch"),
                cmd("Sketcher_LeaveSketch", "Exit Sketch"),
                cmd("Sketcher_ViewSketch", "Normal To"),
            }
        ),
        group(
            "sw_sketch_edit",
            "Edit",
            {
                cmd("Std_Undo", "Undo"),
                cmd("Std_Redo", "Redo"),
                cmd("Std_Delete", "Delete"),
            }
        ),
    };
    layout.tabs.push_back(sketch);

    RibbonTabDef evaluate;
    evaluate.id = "sw_evaluate";
    evaluate.title = "Evaluate";
    evaluate.groups = {
        group(
            "sw_evaluate",
            "Evaluate",
            {
                cmd("Std_Measure", "Measure"),
                cmd("Part_CheckGeometry", "Check"),
            }
        ),
    };
    layout.tabs.push_back(evaluate);

    RibbonTabDef view;
    view.id = "sw_view";
    view.title = "View";
    view.groups = {
        group(
            "sw_standard_views",
            "Standard Views",
            {
                cmd("Std_ViewFront", "Front"),
                cmd("Std_ViewTop", "Top"),
                cmd("Std_ViewRight", "Right"),
                cmd("Std_ViewIsometric", "Isometric"),
                cmd("Std_ViewFitAll", "Zoom to Fit"),
            }
        ),
        group(
            "sw_display",
            "Display",
            {
                cmd("Std_DrawStyle", "Display Style"),
                cmd("Std_ViewFullscreen", "Full Screen"),
            }
        ),
    };
    layout.tabs.push_back(view);

    RibbonTabDef fileTab;
    fileTab.id = "sw_file";
    fileTab.title = "File";
    fileTab.groups = {
        group(
            "sw_file",
            "File",
            {
                cmd("Std_New", "New"),
                cmd("Std_Open", "Open"),
                cmd("Std_Save", "Save"),
            }
        ),
        group(
            "sw_options",
            "Options",
            {
                cmd("Std_DlgPreferences", "Options"),
            }
        ),
    };
    layout.tabs.push_back(fileTab);

    return layout;
}

/** FreeCAD-native ribbon: mirrors common workbench toolbar groups. */
RibbonLayout nativeLayout()
{
    RibbonLayout layout;
    layout.profileId = NativeId;

    RibbonTabDef home;
    home.id = "fc_home";
    home.title = "Home";
    home.groups = {
        group(
            "fc_file",
            "File",
            {
                cmd("Std_New"),
                cmd("Std_Open"),
                cmd("Std_Save"),
            }
        ),
        group(
            "fc_edit",
            "Edit",
            {
                cmd("Std_Undo"),
                cmd("Std_Redo"),
                cmd("Std_Delete"),
            }
        ),
        group(
            "fc_view",
            "View",
            {
                cmd("Std_ViewFitAll"),
                cmd("Std_ViewIsometric"),
            }
        ),
        group(
            "fc_model",
            "Part Design",
            {
                cmd("Sketcher_NewSketch"),
                cmd("PartDesign_Body"),
                cmd("PartDesign_Pad"),
                cmd("PartDesign_Pocket"),
            }
        ),
        group(
            "fc_prefs",
            "Preferences",
            {
                cmd("Std_DlgPreferences"),
            }
        ),
    };
    layout.tabs.push_back(home);

    RibbonTabDef view;
    view.id = "fc_view";
    view.title = "View";
    view.groups = {
        group(
            "fc_standard_views",
            "Standard Views",
            {
                cmd("Std_ViewFront"),
                cmd("Std_ViewTop"),
                cmd("Std_ViewRight"),
                cmd("Std_ViewIsometric"),
                cmd("Std_ViewFitAll"),
            }
        ),
        group(
            "fc_display",
            "Display",
            {
                cmd("Std_DrawStyle"),
                cmd("Std_ViewFullscreen"),
            }
        ),
    };
    layout.tabs.push_back(view);

    return layout;
}

}  // namespace

RibbonLayout RibbonCatalog::layoutForProfile(const std::string& profileId)
{
    if (profileId == CreoId) {
        return creoLayout();
    }
    if (profileId == SolidWorksId) {
        return solidWorksLayout();
    }
    return nativeLayout();
}

}  // namespace Gui
