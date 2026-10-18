// SPDX-License-Identifier: LGPL-2.1-or-later
/***************************************************************************
 *   Copyright (c) 2026 FreeCAD contributors                               *
 *                                                                         *
 *   This file is part of FreeCAD.                                         *
 ***************************************************************************/

#include "CompatibilityProfileManager.h"

#include <Base/Console.h>
#include <Base/Exception.h>
#include <Base/FileInfo.h>
#include <Base/Parameter.h>
#include <App/Application.h>

#include "Application.h"
#include "Command.h"
#include "DockWindowManager.h"
#include "PreferencePackManager.h"
#include "ToolBarManager.h"

using namespace Gui;
namespace fs = std::filesystem;

namespace
{
constexpr const char* ActiveProfileKey = "ActiveProfile";
constexpr const char* SnapshotFileName = "compatibility_profile_baseline.cfg";
constexpr const char* NativeId = "FreeCADNative";
constexpr const char* CreoId = "CreoInspired";
constexpr const char* SolidWorksId = "SolidWorksInspired";
}  // namespace

CompatibilityProfileManager::CompatibilityProfileManager()
{
    ensureProfiles();
}

void CompatibilityProfileManager::ensureProfiles()
{
    if (!_profiles.empty()) {
        return;
    }

    _profiles = {
        {NativeId,
         "FreeCAD Native",
         "",  // Restore baseline only; optional FreeCAD Native pack remains in Preference Packs
         "Restore the FreeCAD baseline captured before Compatibility Profiles were applied. "
         "Does not force a theme when no snapshot exists."},
        {CreoId,
         "Creo Inspired",
         "Creo Inspired",
         "Creo-inspired theme, navigation, shortcuts, and layout. Not affiliated with PTC."},
        {SolidWorksId,
         "SolidWorks Inspired",
         "SolidWorks Inspired",
         "SolidWorks-inspired theme, SolidWorks navigation, shortcuts, and layout. "
         "Not affiliated with Dassault Systemes."},
    };

    _facades[CreoId] = {
        {"New", "Std_New", "File new"},
        {"Open", "Std_Open", "File open"},
        {"Save", "Std_Save", "File save"},
        {"Undo", "Std_Undo", "Edit undo"},
        {"Redo", "Std_Redo", "Edit redo"},
        {"Delete", "Std_Delete", "Delete selection"},
        {"Fit", "Std_ViewFitAll", "Zoom to fit"},
        {"Isometric", "Std_ViewIsometric", "Isometric view"},
        {"Sketch", "Sketcher_NewSketch", "Create sketch"},
        {"Body", "PartDesign_Body", "Create PartDesign Body"},
        {"Preferences", "Std_DlgPreferences", "Open preferences"},
    };

    _facades[SolidWorksId] = {
        {"New", "Std_New", "File new"},
        {"Open", "Std_Open", "File open"},
        {"Save", "Std_Save", "File save"},
        {"Undo", "Std_Undo", "Edit undo"},
        {"Redo", "Std_Redo", "Edit redo (Ctrl+Y in this profile)"},
        {"Delete", "Std_Delete", "Delete selection"},
        {"Zoom to Fit", "Std_ViewFitAll", "Zoom to fit"},
        {"Isometric", "Std_ViewIsometric", "Isometric view"},
        {"Front", "Std_ViewFront", "Front view"},
        {"Top", "Std_ViewTop", "Top view"},
        {"Right", "Std_ViewRight", "Right view"},
        {"Sketch", "Sketcher_NewSketch", "Create sketch"},
        {"Boss/Base (Body)", "PartDesign_Body", "Create PartDesign Body"},
        {"Options", "Std_DlgPreferences", "Open preferences"},
    };

    _facades[NativeId] = {
        {"New", "Std_New", ""},
        {"Open", "Std_Open", ""},
        {"Save", "Std_Save", ""},
        {"Undo", "Std_Undo", ""},
        {"Redo", "Std_Redo", ""},
        {"Fit All", "Std_ViewFitAll", ""},
        {"Preferences", "Std_DlgPreferences", ""},
    };
}

std::vector<CompatibilityProfileManager::Profile> CompatibilityProfileManager::profiles() const
{
    return _profiles;
}

const CompatibilityProfileManager::Profile* CompatibilityProfileManager::findProfile(
    const std::string& profileId
) const
{
    for (const auto& profile : _profiles) {
        if (profile.id == profileId) {
            return &profile;
        }
    }
    return nullptr;
}

ParameterGrp::handle CompatibilityProfileManager::profileParameterGroup()
{
    return App::GetApplication().GetParameterGroupByPath(
        "User parameter:BaseApp/Preferences/CompatibilityProfiles"
    );
}

std::filesystem::path CompatibilityProfileManager::snapshotPath() const
{
    return fs::path(App::Application::getUserAppDataDir()) / "CompatibilityProfiles"
        / SnapshotFileName;
}

bool CompatibilityProfileManager::hasSnapshot() const
{
    return fs::exists(snapshotPath());
}

void CompatibilityProfileManager::saveSnapshot() const
{
    auto path = snapshotPath();
    fs::create_directories(path.parent_path());
    App::GetApplication().GetUserParameter().SaveDocument(
        Base::FileInfo::pathToString(path).c_str()
    );
}

void CompatibilityProfileManager::restoreSnapshot() const
{
    auto path = snapshotPath();
    if (!fs::exists(path)) {
        throw Base::FileException(
            "Compatibility profile baseline snapshot is missing",
            Base::FileInfo::pathToString(path)
        );
    }
    App::GetApplication().GetUserParameter().LoadDocument(
        Base::FileInfo::pathToString(path).c_str()
    );
}

void CompatibilityProfileManager::reloadUiState() const
{
    if (auto* app = Application::Instance) {
        app->reloadStyleSheet();
    }
    if (auto* docks = DockWindowManager::instance()) {
        docks->loadState();
    }
    if (auto* toolbars = ToolBarManager::getInstance()) {
        toolbars->restoreState();
    }
}

void CompatibilityProfileManager::setActiveProfile(const std::string& profileId)
{
    profileParameterGroup()->SetASCII(ActiveProfileKey, profileId.c_str());
}

std::string CompatibilityProfileManager::activeProfileId() const
{
    return profileParameterGroup()->GetASCII(ActiveProfileKey, NativeId);
}

std::string CompatibilityProfileManager::activeProfileDisplayName() const
{
    if (const auto* profile = findProfile(activeProfileId())) {
        return profile->displayName;
    }
    return "FreeCAD Native";
}

bool CompatibilityProfileManager::apply(const std::string& profileId)
{
    const Profile* profile = findProfile(profileId);
    if (!profile) {
        throw Base::ValueError("Unknown compatibility profile: " + profileId);
    }

    // Capture the user baseline once, then always re-apply from that baseline
    // so switching profiles does not stack one-way pack merges.
    if (!hasSnapshot()) {
        saveSnapshot();
    }
    else {
        restoreSnapshot();
        reloadUiState();
    }

    if (!profile->preferencePackName.empty()) {
        auto* packs = Application::Instance->prefPackManager();
        if (!packs) {
            throw Base::RuntimeError("Preference pack manager is not available");
        }
        if (!packs->apply(profile->preferencePackName)) {
            Base::Console().warning(
                "CompatibilityProfiles",
                "Preference pack '{}' was not applied\n",
                profile->preferencePackName
            );
            return false;
        }
    }

    setActiveProfile(profileId);
    reloadUiState();

    Base::Console().message(
        "CompatibilityProfiles",
        "Applied profile '{}'\n",
        profile->displayName
    );
    return true;
}

bool CompatibilityProfileManager::revert()
{
    if (!hasSnapshot()) {
        setActiveProfile(NativeId);
        return false;
    }

    restoreSnapshot();
    setActiveProfile(NativeId);
    reloadUiState();

    Base::Console().message(
        "CompatibilityProfiles",
        "Reverted to the baseline configuration captured before profile apply\n"
    );
    return true;
}

std::vector<CompatibilityProfileManager::CommandFacadeEntry>
CompatibilityProfileManager::commandFacade(const std::string& profileId) const
{
    auto it = _facades.find(profileId);
    if (it == _facades.end()) {
        return {};
    }
    return it->second;
}

std::string CompatibilityProfileManager::resolveCommand(const std::string& vendorLabel) const
{
    const auto entries = commandFacade(activeProfileId());
    for (const auto& entry : entries) {
        if (entry.vendorLabel == vendorLabel) {
            return entry.freeCadCommand;
        }
    }
    // Allow callers to pass FreeCAD command names directly.
    return vendorLabel;
}

bool CompatibilityProfileManager::invokeFacadeCommand(const std::string& vendorLabelOrCommand) const
{
    const std::string commandName = resolveCommand(vendorLabelOrCommand);
    CommandManager& mgr = Application::Instance->commandManager();
    Command* command = mgr.getCommandByName(commandName.c_str());
    if (!command) {
        Base::Console().warning(
            "CompatibilityProfiles",
            "Command '{}' is not registered\n",
            commandName
        );
        return false;
    }
    command->invoke(0);
    return true;
}
