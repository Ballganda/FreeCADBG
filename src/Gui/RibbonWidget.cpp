// SPDX-License-Identifier: LGPL-2.1-or-later
/***************************************************************************
 *   Copyright (c) 2026 FreeCAD contributors                               *
 *                                                                         *
 *   This file is part of FreeCAD.                                         *
 ***************************************************************************/

#include "RibbonWidget.h"

#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QTabBar>
#include <QToolButton>
#include <QVBoxLayout>
#include <QStackedWidget>

#include "Action.h"
#include "Application.h"
#include "Command.h"

using namespace Gui;

RibbonWidget::RibbonWidget(QWidget* parent)
    : QWidget(parent)
{
    setObjectName(QStringLiteral("CompatibilityRibbonWidget"));
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    _tabs = new QTabBar(this);
    _tabs->setDocumentMode(true);
    _tabs->setExpanding(false);
    _tabs->setDrawBase(true);
    root->addWidget(_tabs);

    _pages = new QStackedWidget(this);
    _pages->setMinimumHeight(72);
    root->addWidget(_pages);

    connect(_tabs, &QTabBar::currentChanged, _pages, &QStackedWidget::setCurrentIndex);
}

void RibbonWidget::setLayoutDefinition(const RibbonLayout& layout)
{
    _layout = layout;
    rebuildPages();
}

void RibbonWidget::clearLayoutDefinition()
{
    _layout = RibbonLayout {};
    rebuildPages();
}

void RibbonWidget::rebuildPages()
{
    while (_pages->count() > 0) {
        QWidget* page = _pages->widget(0);
        _pages->removeWidget(page);
        delete page;
    }
    while (_tabs->count() > 0) {
        _tabs->removeTab(0);
    }

    for (const auto& tab : _layout.tabs) {
        _tabs->addTab(QString::fromStdString(tab.title));
        _pages->addWidget(buildTabPage(tab));
    }

    if (_tabs->count() > 0) {
        _tabs->setCurrentIndex(0);
        _pages->setCurrentIndex(0);
    }
}

QWidget* RibbonWidget::buildTabPage(const RibbonTabDef& tab)
{
    auto* page = new QWidget(this);
    auto* row = new QHBoxLayout(page);
    row->setContentsMargins(6, 4, 6, 4);
    row->setSpacing(8);

    auto* cmdMgr = Application::Instance ? &Application::Instance->commandManager() : nullptr;

    for (const auto& group : tab.groups) {
        auto* groupFrame = new QFrame(page);
        groupFrame->setObjectName(QStringLiteral("RibbonGroup"));
        groupFrame->setFrameShape(QFrame::StyledPanel);
        groupFrame->setProperty("ribbonSlot", QString::fromStdString(group.slotId));

        auto* groupLayout = new QVBoxLayout(groupFrame);
        groupLayout->setContentsMargins(6, 4, 6, 2);
        groupLayout->setSpacing(2);

        auto* buttons = new QHBoxLayout();
        buttons->setSpacing(4);

        for (const auto& item : group.commands) {
            auto* button = new QToolButton(groupFrame);
            button->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
            button->setAutoRaise(true);
            button->setMinimumWidth(64);
            button->setIconSize(QSize(24, 24));
            button->setProperty("ribbonCommand", QString::fromStdString(item.commandName));

            QAction* action = nullptr;
            if (cmdMgr && !item.commandName.empty()) {
                if (Command* cmd = cmdMgr->getCommandByName(item.commandName.c_str())) {
                    cmd->getAction();  // ensure created
                    if (auto* guiAction = cmd->getAction()) {
                        action = guiAction->action();
                    }
                }
            }

            if (action) {
                button->setDefaultAction(action);
                if (!item.label.empty()) {
                    button->setText(QString::fromStdString(item.label));
                }
            }
            else {
                button->setEnabled(false);
                button->setText(
                    item.label.empty() ? QString::fromStdString(item.commandName)
                                       : QString::fromStdString(item.label)
                );
                button->setToolTip(
                    tr("Command '%1' is not available in the current workbench")
                        .arg(QString::fromStdString(item.commandName))
                );
            }
            buttons->addWidget(button);
        }

        groupLayout->addLayout(buttons);

        auto* title = new QLabel(QString::fromStdString(group.title), groupFrame);
        title->setAlignment(Qt::AlignHCenter);
        title->setStyleSheet(QStringLiteral("color: palette(mid); font-size: 11px;"));
        groupLayout->addWidget(title);

        row->addWidget(groupFrame);
    }

    row->addStretch(1);
    return page;
}

#include "moc_RibbonWidget.cpp"
