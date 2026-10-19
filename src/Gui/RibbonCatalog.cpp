// SPDX-License-Identifier: LGPL-2.1-or-later
/***************************************************************************
 *   Copyright (c) 2026 FreeCAD contributors                               *
 *                                                                         *
 *   This file is part of FreeCAD.                                         *
 ***************************************************************************/

#include "RibbonCatalog.h"

#include <initializer_list>
#include <utility>
#include <vector>

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

RibbonControlDef largeBtn(const char* name, const char* label = "")
{
    RibbonControlDef control;
    control.kind = RibbonControlDef::Kind::Button;
    control.size = RibbonControlSize::Large;
    control.commands = {cmd(name, label)};
    return control;
}

RibbonControlDef smallBtn(const char* name, const char* label = "")
{
    RibbonControlDef control;
    control.kind = RibbonControlDef::Kind::Button;
    control.size = RibbonControlSize::Small;
    control.commands = {cmd(name, label)};
    return control;
}

RibbonControlDef splitLarge(
    const char* splitId,
    std::initializer_list<RibbonCommandItem> items
)
{
    RibbonControlDef control;
    control.kind = RibbonControlDef::Kind::SplitButton;
    control.size = RibbonControlSize::Large;
    control.splitId = splitId;
    control.commands.assign(items.begin(), items.end());
    return control;
}

RibbonControlDef splitSmall(
    const char* splitId,
    std::initializer_list<RibbonCommandItem> items
)
{
    RibbonControlDef control;
    control.kind = RibbonControlDef::Kind::SplitButton;
    control.size = RibbonControlSize::Small;
    control.splitId = splitId;
    control.commands.assign(items.begin(), items.end());
    return control;
}

RibbonGroupDef group(
    const char* slotId,
    const char* title,
    std::vector<RibbonControlDef> controls
)
{
    return {slotId, title, std::move(controls)};
}

/**
 * Creo-inspired flow with large / 2x2 small / split controls.
 * Model → Analysis → Annotate → Tools → View
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
                splitLarge(
                    "creo_get_data",
                    {
                        cmd("Std_Open", "Open"),
                        cmd("Std_New", "New"),
                        cmd("Std_Save", "Save"),
                    }
                ),
            }
        ),
        group(
            "creo_datums",
            "Datums",
            {
                largeBtn("PartDesign_Body", "Body"),
                // 2x2 cell beside the large Body button
                smallBtn("PartDesign_Plane", "Plane"),
                smallBtn("PartDesign_Line", "Axis"),
                smallBtn("PartDesign_Point", "Point"),
                smallBtn("Sketcher_NewSketch", "Sketch"),
            }
        ),
        group(
            "creo_shapes",
            "Shapes",
            {
                splitLarge(
                    "creo_shapes",
                    {
                        cmd("PartDesign_Pad", "Extrude"),
                        cmd("PartDesign_Revolution", "Revolve"),
                        cmd("PartDesign_AdditivePipe", "Sweep"),
                        cmd("PartDesign_AdditiveLoft", "Blend"),
                    }
                ),
                largeBtn("Sketcher_NewSketch", "Sketch"),
            }
        ),
        group(
            "creo_editing",
            "Editing",
            {
                splitLarge(
                    "creo_editing",
                    {
                        cmd("PartDesign_Fillet", "Round"),
                        cmd("PartDesign_Chamfer", "Chamfer"),
                        cmd("PartDesign_Pocket", "Cut"),
                        cmd("PartDesign_Mirrored", "Mirror"),
                    }
                ),
            }
        ),
        group(
            "creo_operations",
            "Operations",
            {
                smallBtn("Part_Fuse", "Merge"),
                smallBtn("Part_Cut", "Cut"),
                smallBtn("Part_Common", "Intersect"),
                smallBtn("Std_Delete", "Delete"),
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
                largeBtn("Std_Measure", "Measure"),
                largeBtn("Part_CheckGeometry", "Check Geometry"),
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
                smallBtn("Part_Section", "Section"),
                smallBtn("Std_SelBoundingBox", "Bound Box"),
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
                smallBtn("Std_Undo", "Undo"),
                smallBtn("Std_Redo", "Redo"),
            }
        ),
        group(
            "creo_options",
            "Utilities",
            {
                largeBtn("Std_DlgPreferences", "Options"),
                smallBtn("Std_DlgParameter", "Parameters"),
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
                splitLarge(
                    "creo_orient",
                    {
                        cmd("Std_ViewIsometric", "Default Orient"),
                        cmd("Std_ViewFront", "Front"),
                        cmd("Std_ViewTop", "Top"),
                        cmd("Std_ViewRight", "Right"),
                    }
                ),
                smallBtn("Std_ViewFront", "Front"),
                smallBtn("Std_ViewTop", "Top"),
                smallBtn("Std_ViewRight", "Right"),
                smallBtn("Std_ViewFitAll", "Refit"),
            }
        ),
        group(
            "creo_model_display",
            "Model Display",
            {
                largeBtn("Std_DrawStyle", "Display Style"),
                smallBtn("Std_ViewFullscreen", "Fullscreen"),
            }
        ),
    };
    layout.tabs.push_back(view);

    return layout;
}

/**
 * SolidWorks-inspired flow with large / 2x2 / split controls.
 * Features → Sketch → Evaluate → View → File
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
                largeBtn("Sketcher_NewSketch", "Sketch"),
                smallBtn("Sketcher_LeaveSketch", "Exit Sketch"),
            }
        ),
        group(
            "sw_features",
            "Features",
            {
                splitLarge(
                    "sw_features_boss",
                    {
                        cmd("PartDesign_Pad", "Extruded Boss/Base"),
                        cmd("PartDesign_Revolution", "Revolved Boss/Base"),
                        cmd("PartDesign_Body", "Boss/Base"),
                    }
                ),
                splitLarge(
                    "sw_features_cut",
                    {
                        cmd("PartDesign_Pocket", "Extruded Cut"),
                        cmd("PartDesign_Fillet", "Fillet"),
                        cmd("PartDesign_Chamfer", "Chamfer"),
                    }
                ),
                smallBtn("PartDesign_Mirrored", "Mirror"),
                smallBtn("PartDesign_LinearPattern", "L Pattern"),
                smallBtn("PartDesign_Plane", "Plane"),
                smallBtn("PartDesign_Line", "Axis"),
            }
        ),
        group(
            "sw_reference",
            "Reference Geometry",
            {
                smallBtn("PartDesign_Plane", "Plane"),
                smallBtn("PartDesign_Line", "Axis"),
                smallBtn("PartDesign_Point", "Point"),
            }
        ),
        group(
            "sw_boolean",
            "Combine",
            {
                splitSmall(
                    "sw_boolean",
                    {
                        cmd("Part_Fuse", "Combine"),
                        cmd("Part_Cut", "Subtract"),
                        cmd("Part_Common", "Common"),
                    }
                ),
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
                largeBtn("Sketcher_NewSketch", "Sketch"),
                smallBtn("Sketcher_LeaveSketch", "Exit Sketch"),
                smallBtn("Sketcher_ViewSketch", "Normal To"),
            }
        ),
        group(
            "sw_sketch_edit",
            "Edit",
            {
                smallBtn("Std_Undo", "Undo"),
                smallBtn("Std_Redo", "Redo"),
                smallBtn("Std_Delete", "Delete"),
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
                largeBtn("Std_Measure", "Measure"),
                largeBtn("Part_CheckGeometry", "Check"),
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
                largeBtn("Std_ViewIsometric", "Isometric"),
                smallBtn("Std_ViewFront", "Front"),
                smallBtn("Std_ViewTop", "Top"),
                smallBtn("Std_ViewRight", "Right"),
                smallBtn("Std_ViewFitAll", "Zoom to Fit"),
            }
        ),
        group(
            "sw_display",
            "Display",
            {
                largeBtn("Std_DrawStyle", "Display Style"),
                smallBtn("Std_ViewFullscreen", "Full Screen"),
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
                smallBtn("Std_New", "New"),
                smallBtn("Std_Open", "Open"),
                smallBtn("Std_Save", "Save"),
            }
        ),
        group(
            "sw_options",
            "Options",
            {
                largeBtn("Std_DlgPreferences", "Options"),
            }
        ),
    };
    layout.tabs.push_back(fileTab);

    return layout;
}

/** FreeCAD-native ribbon with mixed large / small packing. */
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
                largeBtn("Std_New"),
                smallBtn("Std_Open"),
                smallBtn("Std_Save"),
            }
        ),
        group(
            "fc_edit",
            "Edit",
            {
                smallBtn("Std_Undo"),
                smallBtn("Std_Redo"),
                smallBtn("Std_Delete"),
            }
        ),
        group(
            "fc_view",
            "View",
            {
                largeBtn("Std_ViewFitAll"),
                largeBtn("Std_ViewIsometric"),
            }
        ),
        group(
            "fc_model",
            "Part Design",
            {
                splitLarge(
                    "fc_model_features",
                    {
                        cmd("PartDesign_Pad", "Pad"),
                        cmd("PartDesign_Pocket", "Pocket"),
                        cmd("PartDesign_Body", "Body"),
                    }
                ),
                largeBtn("Sketcher_NewSketch"),
            }
        ),
        group(
            "fc_prefs",
            "Preferences",
            {
                largeBtn("Std_DlgPreferences"),
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
                smallBtn("Std_ViewFront"),
                smallBtn("Std_ViewTop"),
                smallBtn("Std_ViewRight"),
                smallBtn("Std_ViewIsometric"),
            }
        ),
        group(
            "fc_display",
            "Display",
            {
                largeBtn("Std_DrawStyle"),
                smallBtn("Std_ViewFullscreen"),
                smallBtn("Std_ViewFitAll"),
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
