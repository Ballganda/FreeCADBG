// SPDX-License-Identifier: LGPL-2.1-or-later
/***************************************************************************
 *   Copyright (c) 2026 FreeCAD contributors                               *
 *                                                                         *
 *   This file is part of FreeCAD.                                         *
 ***************************************************************************/

#pragma once

#include <QObject>
#include <QPointer>
#include <QStringList>

#include <FCGlobal.h>

class QToolBar;

namespace Gui
{

class MainWindow;
class RibbonWidget;

/**
 * Owns the compatibility ribbon host toolbar, rebuilds it from the active
 * profile layout, and optionally hides classic workbench toolbars.
 */
class GuiExport RibbonManager: public QObject
{
    Q_OBJECT

public:
    static RibbonManager* getInstance();
    static void destruct();

    void install(MainWindow* mainWindow);
    void syncFromActiveProfile();

    bool isRibbonEnabled() const;
    void setRibbonEnabled(bool enabled);

    /** When true, classic workbench toolbars are force-hidden while the ribbon is shown. */
    bool hideClassicToolbars() const;
    void setHideClassicToolbars(bool hide);

private:
    RibbonManager();
    ~RibbonManager() override;

    void rebuild();
    void applyToolbarVisibility();
    void restoreClassicToolbars();
    QStringList classicToolbarNames() const;

    static RibbonManager* _instance;

    QPointer<MainWindow> _mainWindow;
    QPointer<QToolBar> _host;
    QPointer<RibbonWidget> _ribbon;
    bool _installed {false};
};

}  // namespace Gui
