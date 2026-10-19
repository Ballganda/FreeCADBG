// SPDX-License-Identifier: LGPL-2.1-or-later
/***************************************************************************
 *   Copyright (c) 2026 FreeCAD contributors                               *
 *                                                                         *
 *   This file is part of FreeCAD.                                         *
 ***************************************************************************/

#pragma once

#include <QWidget>

#include "RibbonDefinition.h"

class QTabBar;
class QStackedWidget;

namespace Gui
{

/**
 * Ribbon: tab bar + pages of command groups.
 * Supports large buttons, 2x2 small packing, and Creo-style split buttons
 * (click active face / dropdown to change active command).
 */
class RibbonWidget: public QWidget
{
    Q_OBJECT

public:
    explicit RibbonWidget(QWidget* parent = nullptr);
    ~RibbonWidget() override = default;

    void setLayoutDefinition(const RibbonLayout& layout);
    void clearLayoutDefinition();

private:
    void rebuildPages();
    QWidget* buildTabPage(const RibbonTabDef& tab);

    QTabBar* _tabs {nullptr};
    QStackedWidget* _pages {nullptr};
    RibbonLayout _layout;
};

}  // namespace Gui
