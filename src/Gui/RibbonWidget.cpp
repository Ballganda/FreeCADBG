// SPDX-License-Identifier: LGPL-2.1-or-later
/***************************************************************************
 *   Copyright (c) 2026 FreeCAD contributors                               *
 *                                                                         *
 *   This file is part of FreeCAD.                                         *
 ***************************************************************************/

#include "RibbonWidget.h"

#include <QAction>
#include <QCoreApplication>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMenu>
#include <QStackedWidget>
#include <QTabBar>
#include <QToolButton>
#include <QVBoxLayout>

#include <App/Application.h>
#include <Base/Parameter.h>

#include "Action.h"
#include "Application.h"
#include "Command.h"

using namespace Gui;

namespace
{

constexpr int LargeMinWidth = 72;
constexpr int LargeIcon = 32;
constexpr int SmallMinWidth = 56;
constexpr int SmallIcon = 16;
constexpr const char* SplitPrefPath =
    "User parameter:BaseApp/Preferences/CompatibilityProfiles/RibbonSplitActive";

QAction* resolveCommandAction(CommandManager* cmdMgr, const std::string& commandName)
{
    if (!cmdMgr || commandName.empty()) {
        return nullptr;
    }
    Command* cmd = cmdMgr->getCommandByName(commandName.c_str());
    if (!cmd) {
        return nullptr;
    }
    cmd->getAction();
    if (auto* guiAction = cmd->getAction()) {
        return guiAction->action();
    }
    return nullptr;
}

QString itemLabel(const RibbonCommandItem& item, QAction* action)
{
    if (!item.label.empty()) {
        return QString::fromStdString(item.label);
    }
    if (action) {
        return action->text();
    }
    return QString::fromStdString(item.commandName);
}

void styleToolButton(QToolButton* button, RibbonControlSize size)
{
    button->setAutoRaise(true);
    if (size == RibbonControlSize::Large) {
        button->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
        button->setIconSize(QSize(LargeIcon, LargeIcon));
        button->setMinimumWidth(LargeMinWidth);
        button->setMinimumHeight(56);
    }
    else {
        button->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        button->setIconSize(QSize(SmallIcon, SmallIcon));
        button->setMinimumWidth(SmallMinWidth);
        button->setMaximumHeight(28);
    }
}

QToolButton* makeCommandButton(
    QWidget* parent,
    CommandManager* cmdMgr,
    const RibbonCommandItem& item,
    RibbonControlSize size
)
{
    auto* button = new QToolButton(parent);
    styleToolButton(button, size);
    button->setProperty("ribbonCommand", QString::fromStdString(item.commandName));

    QAction* action = resolveCommandAction(cmdMgr, item.commandName);
    if (action) {
        button->setDefaultAction(action);
        const QString label = itemLabel(item, action);
        if (!label.isEmpty()) {
            button->setText(label);
        }
    }
    else {
        button->setEnabled(false);
        button->setText(itemLabel(item, nullptr));
        button->setToolTip(
            QCoreApplication::translate(
                "Gui::RibbonWidget",
                "Command '%1' is not available in the current workbench"
            )
                .arg(QString::fromStdString(item.commandName))
        );
    }
    return button;
}

/** Creo-style split: face runs active command; chevron changes active (+ runs). */
class RibbonSplitButton: public QWidget
{
public:
    RibbonSplitButton(QWidget* parent, CommandManager* cmdMgr, const RibbonControlDef& def)
        : QWidget(parent)
        , _cmdMgr(cmdMgr)
        , _def(def)
    {
        setObjectName(QStringLiteral("RibbonSplitButton"));
        auto* row = new QHBoxLayout(this);
        row->setContentsMargins(0, 0, 0, 0);
        row->setSpacing(0);

        _face = new QToolButton(this);
        styleToolButton(_face, def.size);
        _face->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);

        _chevron = new QToolButton(this);
        _chevron->setAutoRaise(true);
        _chevron->setArrowType(Qt::DownArrow);
        _chevron->setToolButtonStyle(Qt::ToolButtonIconOnly);
        _chevron->setPopupMode(QToolButton::InstantPopup);
        _chevron->setMinimumWidth(18);
        _chevron->setMinimumHeight(def.size == RibbonControlSize::Large ? 56 : 0);
        if (def.size == RibbonControlSize::Small) {
            _chevron->setMaximumHeight(28);
        }

        _menu = new QMenu(this);
        _chevron->setMenu(_menu);

        row->addWidget(_face, 1);
        row->addWidget(_chevron);

        _face->setProperty("splitRole", QLatin1String("main"));
        _chevron->setProperty("splitRole", QLatin1String("menu"));

        rebuildMenu();
        applyActiveFromPrefs();
        syncFace();

        connect(_face, &QToolButton::clicked, this, [this]() { runActive(); });
        connect(_menu, &QMenu::triggered, this, [this](QAction* action) { onMenuTriggered(action); });
    }

private:
    int findCommandIndex(const std::string& name) const
    {
        for (int i = 0; i < static_cast<int>(_def.commands.size()); ++i) {
            if (_def.commands[static_cast<size_t>(i)].commandName == name) {
                return i;
            }
        }
        return -1;
    }

    void applyActiveFromPrefs()
    {
        _activeIndex = 0;
        if (_def.splitId.empty() || _def.commands.empty()) {
            return;
        }
        auto hGrp = App::GetApplication().GetParameterGroupByPath(SplitPrefPath);
        const std::string saved = hGrp->GetASCII(_def.splitId.c_str(), "");
        const int idx = findCommandIndex(saved);
        if (idx >= 0) {
            _activeIndex = idx;
        }
    }

    void persistActive() const
    {
        if (_def.splitId.empty() || _def.commands.empty()) {
            return;
        }
        auto hGrp = App::GetApplication().GetParameterGroupByPath(SplitPrefPath);
        hGrp->SetASCII(
            _def.splitId.c_str(),
            _def.commands[static_cast<size_t>(_activeIndex)].commandName.c_str()
        );
    }

    void rebuildMenu()
    {
        _menu->clear();
        for (size_t i = 0; i < _def.commands.size(); ++i) {
            const auto& item = _def.commands[i];
            QAction* cmdAction = resolveCommandAction(_cmdMgr, item.commandName);
            auto* menuAction = _menu->addAction(itemLabel(item, cmdAction));
            if (cmdAction) {
                menuAction->setIcon(cmdAction->icon());
            }
            menuAction->setData(static_cast<int>(i));
            menuAction->setEnabled(cmdAction != nullptr);
        }
    }

    void syncFace()
    {
        if (_def.commands.empty()) {
            _face->setEnabled(false);
            return;
        }
        const auto& item = _def.commands[static_cast<size_t>(_activeIndex)];
        QAction* action = resolveCommandAction(_cmdMgr, item.commandName);
        if (action) {
            _face->setEnabled(action->isEnabled());
            _face->setIcon(action->icon());
            _face->setToolTip(action->toolTip());
        }
        else {
            _face->setEnabled(false);
            _face->setIcon(QIcon());
            _face->setToolTip(
                QCoreApplication::translate("Gui::RibbonWidget", "Command '%1' is not available")
                    .arg(QString::fromStdString(item.commandName))
            );
        }
        _face->setText(itemLabel(item, action));
        _face->setProperty("ribbonCommand", QString::fromStdString(item.commandName));
    }

    void runActive()
    {
        if (_def.commands.empty() || !_cmdMgr) {
            return;
        }
        const auto& name = _def.commands[static_cast<size_t>(_activeIndex)].commandName;
        if (Command* cmd = _cmdMgr->getCommandByName(name.c_str())) {
            cmd->invoke(0);
        }
    }

    void onMenuTriggered(QAction* menuAction)
    {
        if (!menuAction) {
            return;
        }
        const int idx = menuAction->data().toInt();
        if (idx < 0 || idx >= static_cast<int>(_def.commands.size())) {
            return;
        }
        _activeIndex = idx;
        persistActive();
        syncFace();
        runActive();
    }

    CommandManager* _cmdMgr {nullptr};
    RibbonControlDef _def;
    QToolButton* _face {nullptr};
    QToolButton* _chevron {nullptr};
    QMenu* _menu {nullptr};
    int _activeIndex {0};
};

QWidget* makeControlWidget(QWidget* parent, CommandManager* cmdMgr, const RibbonControlDef& control)
{
    if (control.kind == RibbonControlDef::Kind::SplitButton) {
        if (control.commands.empty()) {
            return new QWidget(parent);
        }
        return new RibbonSplitButton(parent, cmdMgr, control);
    }

    if (control.commands.empty()) {
        auto* placeholder = new QWidget(parent);
        placeholder->setMinimumWidth(
            control.size == RibbonControlSize::Large ? LargeMinWidth : SmallMinWidth
        );
        return placeholder;
    }
    return makeCommandButton(parent, cmdMgr, control.commands.front(), control.size);
}

}  // namespace

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
    _pages->setMinimumHeight(88);
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

        size_t i = 0;
        while (i < group.controls.size()) {
            const auto& control = group.controls[i];
            if (control.size != RibbonControlSize::Small) {
                buttons->addWidget(makeControlWidget(groupFrame, cmdMgr, control));
                ++i;
                continue;
            }

            // Consecutive Small controls pack into a 2x2 cell (same space as one Large).
            auto* cell = new QWidget(groupFrame);
            auto* grid = new QGridLayout(cell);
            grid->setContentsMargins(0, 0, 0, 0);
            grid->setHorizontalSpacing(2);
            grid->setVerticalSpacing(2);

            int packed = 0;
            while (i < group.controls.size() && packed < 4
                   && group.controls[i].size == RibbonControlSize::Small) {
                grid->addWidget(
                    makeControlWidget(cell, cmdMgr, group.controls[i]),
                    packed / 2,
                    packed % 2
                );
                ++packed;
                ++i;
            }
            buttons->addWidget(cell);
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
