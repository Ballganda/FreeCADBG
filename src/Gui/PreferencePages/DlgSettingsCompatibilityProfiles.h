// SPDX-License-Identifier: LGPL-2.1-or-later
/***************************************************************************
 *   Copyright (c) 2026 FreeCAD contributors                               *
 *                                                                         *
 *   This file is part of FreeCAD.                                         *
 ***************************************************************************/

#pragma once

#include <Gui/PropertyPage.h>
#include <QString>

class QComboBox;
class QLabel;
class QListWidget;
class QPushButton;

namespace Gui
{
namespace Dialog
{

/**
 * Preference page to apply or revert Compatibility Profiles
 * (FreeCAD Native / Creo Inspired / SolidWorks Inspired).
 */
class DlgSettingsCompatibilityProfiles: public PreferencePage
{
    Q_OBJECT

public:
    explicit DlgSettingsCompatibilityProfiles(QWidget* parent = nullptr);
    ~DlgSettingsCompatibilityProfiles() override;

    void saveSettings() override;
    void loadSettings() override;

protected:
    void changeEvent(QEvent* e) override;

private:
    void refreshStatus();
    void onApplyClicked();
    void onRevertClicked();
    void onProfileChanged(int index);

private:
    QComboBox* profileCombo {nullptr};
    QLabel* descriptionLabel {nullptr};
    QLabel* statusLabel {nullptr};
    QListWidget* facadeList {nullptr};
    QPushButton* applyButton {nullptr};
    QPushButton* revertButton {nullptr};
};

}  // namespace Dialog
}  // namespace Gui
