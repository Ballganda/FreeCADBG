// SPDX-License-Identifier: LGPL-2.1-or-later
/***************************************************************************
 *   Copyright (c) 2026 FreeCAD contributors                               *
 *                                                                         *
 *   This file is part of FreeCAD.                                         *
 ***************************************************************************/

#pragma once

#include <map>
#include <string>
#include <vector>
#include <filesystem>

#include <Base/Parameter.h>
#include <FCGlobal.h>

namespace Gui
{

/**
 * Named presentation stacks (theme + navigation + shortcuts + layout) that can
 * be applied and reverted without permanently overwriting the user baseline.
 *
 * Vendor names in profile labels are "inspired" branding only.
 */
class GuiExport CompatibilityProfileManager
{
public:
    struct Profile
    {
        std::string id;
        std::string displayName;
        std::string preferencePackName;
        std::string description;
    };

    struct CommandFacadeEntry
    {
        std::string vendorLabel;
        std::string freeCadCommand;
        std::string notes;
    };

    CompatibilityProfileManager();
    ~CompatibilityProfileManager() = default;

    std::vector<Profile> profiles() const;
    const Profile* findProfile(const std::string& profileId) const;

    /** Apply a named profile. Snapshots the user baseline on first use. */
    bool apply(const std::string& profileId);

    /** Restore the pre-profile baseline snapshot and clear the active profile. */
    bool revert();

    bool hasSnapshot() const;
    std::string activeProfileId() const;
    std::string activeProfileDisplayName() const;

    /** Resolve a vendor-facing label to a FreeCAD command name for the active profile. */
    std::string resolveCommand(const std::string& vendorLabel) const;

    /** Curated facade map for the given profile (may be empty). */
    std::vector<CommandFacadeEntry> commandFacade(const std::string& profileId) const;

    /** Run a FreeCAD command by facade label or by FreeCAD command name. */
    bool runCommand(const std::string& vendorLabelOrCommand) const;

private:
    void ensureProfiles();
    void setActiveProfile(const std::string& profileId);
    std::filesystem::path snapshotPath() const;
    void saveSnapshot() const;
    void restoreSnapshot() const;
    void reloadUiState() const;
    static ParameterGrp::handle profileParameterGroup();

    std::vector<Profile> _profiles;
    std::map<std::string, std::vector<CommandFacadeEntry>> _facades;
};

}  // namespace Gui
