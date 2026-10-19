// SPDX-License-Identifier: LGPL-2.1-or-later
/***************************************************************************
 *   Copyright (c) 2026 FreeCAD contributors                               *
 *                                                                         *
 *   This file is part of FreeCAD.                                         *
 ***************************************************************************/

#include "RibbonManager.h"

#include <QMainWindow>
#include <QToolBar>

#include <App/Application.h>
#include <Base/Parameter.h>

#include "Application.h"
#include "CompatibilityProfileManager.h"
#include "MainWindow.h"
#include "RibbonCatalog.h"
#include "RibbonWidget.h"
#include "ToolBarManager.h"

using namespace Gui;

namespace
{
constexpr const char* PrefGroup = "User parameter:BaseApp/Preferences/CompatibilityProfiles";
constexpr const char* ShowRibbonKey = "ShowRibbon";
constexpr const char* HideClassicKey = "HideClassicToolbars";
constexpr const char* HostObjectName = "CompatibilityRibbon";
}  // namespace

RibbonManager* RibbonManager::_instance = nullptr;

RibbonManager* RibbonManager::getInstance()
{
    if (!_instance) {
        _instance = new RibbonManager();
    }
    return _instance;
}

void RibbonManager::destruct()
{
    delete _instance;
    _instance = nullptr;
}

RibbonManager::RibbonManager() = default;

RibbonManager::~RibbonManager() = default;

void RibbonManager::install(MainWindow* mainWindow)
{
    if (!mainWindow) {
        return;
    }
    _mainWindow = mainWindow;

    if (!_host) {
        _host = new QToolBar(tr("Compatibility Ribbon"), mainWindow);
        _host->setObjectName(QString::fromLatin1(HostObjectName));
        _host->setMovable(false);
        _host->setFloatable(false);
        _host->setAllowedAreas(Qt::TopToolBarArea);
        _host->toggleViewAction()->setVisible(false);

        _ribbon = new RibbonWidget(_host);
        _host->addWidget(_ribbon);

        mainWindow->addToolBarBreak(Qt::TopToolBarArea);
        mainWindow->addToolBar(Qt::TopToolBarArea, _host);
    }

    if (!_installed) {
        connect(
            mainWindow,
            &MainWindow::workbenchActivated,
            this,
            [this](const QString&) { syncFromActiveProfile(); }
        );
        _installed = true;
    }

    syncFromActiveProfile();
}

bool RibbonManager::isRibbonEnabled() const
{
    auto hGrp = App::GetApplication().GetParameterGroupByPath(PrefGroup);
    // Default on for Creo/SW inspired profiles when the key is unset.
    return hGrp->GetBool(ShowRibbonKey, true);
}

void RibbonManager::setRibbonEnabled(bool enabled)
{
    App::GetApplication().GetParameterGroupByPath(PrefGroup)->SetBool(ShowRibbonKey, enabled);
    syncFromActiveProfile();
}

bool RibbonManager::hideClassicToolbars() const
{
    return App::GetApplication()
        .GetParameterGroupByPath(PrefGroup)
        ->GetBool(HideClassicKey, true);
}

void RibbonManager::setHideClassicToolbars(bool hide)
{
    App::GetApplication().GetParameterGroupByPath(PrefGroup)->SetBool(HideClassicKey, hide);
    syncFromActiveProfile();
}

void RibbonManager::syncFromActiveProfile()
{
    if (!_host || !_ribbon) {
        return;
    }
    rebuild();
    applyToolbarVisibility();
}

void RibbonManager::rebuild()
{
    if (!_ribbon) {
        return;
    }

    auto* profiles = Application::Instance ? Application::Instance->compatibilityProfileManager()
                                           : nullptr;
    const std::string profileId = profiles ? profiles->activeProfileId() : "FreeCADNative";
    const bool show = isRibbonEnabled();

    if (!show) {
        _ribbon->clearLayoutDefinition();
        _host->setVisible(false);
        restoreClassicToolbars();
        return;
    }

    _ribbon->setLayoutDefinition(RibbonCatalog::layoutForProfile(profileId));
    _host->setVisible(true);
}

QStringList RibbonManager::classicToolbarNames() const
{
    // Common StdWorkbench / PartDesign toolbar titles used across workbenches.
    // Force-hiding by name; unknown names are ignored by ToolBarManager.
    return {
        QStringLiteral("File"),
        QStringLiteral("Edit"),
        QStringLiteral("Clipboard"),
        QStringLiteral("Workbench"),
        QStringLiteral("Macro"),
        QStringLiteral("View"),
        QStringLiteral("Structure"),
        QStringLiteral("Help"),
        QStringLiteral("Individual views"),
        QStringLiteral("Part Design Helper"),
        QStringLiteral("Part Design Modeling"),
        QStringLiteral("Sketcher"),
        QStringLiteral("Sketcher geometries"),
        QStringLiteral("Sketcher constraints"),
        QStringLiteral("Sketcher tools"),
        QStringLiteral("Sketcher B-spline tools"),
        QStringLiteral("Sketcher visual"),
        QStringLiteral("Part tools"),
        QStringLiteral("Boolean"),
    };
}

void RibbonManager::applyToolbarVisibility()
{
    if (!_host) {
        return;
    }

    if (!isRibbonEnabled() || !hideClassicToolbars()) {
        restoreClassicToolbars();
        return;
    }

    if (auto* mgr = ToolBarManager::getInstance()) {
        mgr->setState(classicToolbarNames(), ToolBarManager::State::ForceHidden);
    }
    // Keep the ribbon host visible.
    _host->setVisible(true);
}

void RibbonManager::restoreClassicToolbars()
{
    if (auto* mgr = ToolBarManager::getInstance()) {
        mgr->setState(classicToolbarNames(), ToolBarManager::State::RestoreDefault);
        mgr->restoreState();
    }
}
