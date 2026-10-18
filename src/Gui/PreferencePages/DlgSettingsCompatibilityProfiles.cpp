// SPDX-License-Identifier: LGPL-2.1-or-later
/***************************************************************************
 *   Copyright (c) 2026 FreeCAD contributors                               *
 *                                                                         *
 *   This file is part of FreeCAD.                                         *
 ***************************************************************************/

#include "DlgSettingsCompatibilityProfiles.h"

#include <QHBoxLayout>
#include <QComboBox>
#include <QLabel>
#include <QListWidget>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

#include <Base/Exception.h>
#include <Gui/Application.h>
#include <Gui/CompatibilityProfileManager.h>

using namespace Gui::Dialog;

/* TRANSLATOR Gui::Dialog::DlgSettingsCompatibilityProfiles */

DlgSettingsCompatibilityProfiles::DlgSettingsCompatibilityProfiles(QWidget* parent)
    : PreferencePage(parent)
{
    auto* root = new QVBoxLayout(this);

    auto* intro = new QLabel(
        tr("Compatibility Profiles switch theme, navigation, shortcuts, and layout "
           "toward vendor-inspired habits. Geometry still uses the FreeCAD/OCCT kernel. "
           "Names are inspired only and are not affiliated with PTC or Dassault Systemes."),
        this
    );
    intro->setWordWrap(true);
    root->addWidget(intro);

    auto* row = new QHBoxLayout();
    row->addWidget(new QLabel(tr("Profile:"), this));
    profileCombo = new QComboBox(this);
    row->addWidget(profileCombo, 1);
    root->addLayout(row);

    descriptionLabel = new QLabel(this);
    descriptionLabel->setWordWrap(true);
    root->addWidget(descriptionLabel);

    statusLabel = new QLabel(this);
    statusLabel->setWordWrap(true);
    root->addWidget(statusLabel);

    root->addWidget(new QLabel(tr("Command facade (vendor label → FreeCAD command):"), this));
    facadeList = new QListWidget(this);
    facadeList->setMinimumHeight(140);
    root->addWidget(facadeList, 1);

    auto* buttons = new QHBoxLayout();
    applyButton = new QPushButton(tr("Apply profile"), this);
    revertButton = new QPushButton(tr("Revert to baseline"), this);
    buttons->addWidget(applyButton);
    buttons->addWidget(revertButton);
    buttons->addStretch();
    root->addLayout(buttons);

    connect(profileCombo, qOverload<int>(&QComboBox::currentIndexChanged), this, [this](int index) {
        onProfileChanged(index);
    });
    connect(applyButton, &QPushButton::clicked, this, &DlgSettingsCompatibilityProfiles::onApplyClicked);
    connect(revertButton, &QPushButton::clicked, this, &DlgSettingsCompatibilityProfiles::onRevertClicked);
}

DlgSettingsCompatibilityProfiles::~DlgSettingsCompatibilityProfiles() = default;

void DlgSettingsCompatibilityProfiles::saveSettings()
{
    // Apply is explicit via buttons so users can preview the description first.
}

void DlgSettingsCompatibilityProfiles::loadSettings()
{
    auto* manager = Application::Instance->compatibilityProfileManager();
    if (!manager) {
        return;
    }

    const QString activeId = QString::fromStdString(manager->activeProfileId());
    profileCombo->blockSignals(true);
    profileCombo->clear();
    int activeIndex = 0;
    int index = 0;
    for (const auto& profile : manager->profiles()) {
        profileCombo->addItem(
            QString::fromStdString(profile.displayName),
            QString::fromStdString(profile.id)
        );
        if (QString::fromStdString(profile.id) == activeId) {
            activeIndex = index;
        }
        ++index;
    }
    profileCombo->setCurrentIndex(activeIndex);
    profileCombo->blockSignals(false);

    onProfileChanged(profileCombo->currentIndex());
    refreshStatus();
}

void DlgSettingsCompatibilityProfiles::changeEvent(QEvent* e)
{
    if (e->type() == QEvent::LanguageChange) {
        // Labels are rebuilt via loadSettings when the page is shown again.
    }
    PreferencePage::changeEvent(e);
}

void DlgSettingsCompatibilityProfiles::onProfileChanged(int index)
{
    auto* manager = Application::Instance->compatibilityProfileManager();
    if (!manager || index < 0) {
        return;
    }

    const auto profileId = profileCombo->itemData(index).toString().toStdString();
    const auto* profile = manager->findProfile(profileId);
    if (!profile) {
        return;
    }

    descriptionLabel->setText(QString::fromStdString(profile->description));

    facadeList->clear();
    for (const auto& entry : manager->commandFacade(profileId)) {
        facadeList->addItem(
            QStringLiteral("%1 → %2")
                .arg(QString::fromStdString(entry.vendorLabel), QString::fromStdString(entry.freeCadCommand))
        );
    }
}

void DlgSettingsCompatibilityProfiles::refreshStatus()
{
    auto* manager = Application::Instance->compatibilityProfileManager();
    if (!manager) {
        return;
    }

    QString status = tr("Active profile: %1")
                         .arg(QString::fromStdString(manager->activeProfileDisplayName()));
    if (manager->hasSnapshot()) {
        status += QLatin1Char('\n');
        status += tr("A baseline snapshot is stored. Revert restores that snapshot.");
    }
    else {
        status += QLatin1Char('\n');
        status += tr("No baseline snapshot yet. The first Apply will capture one.");
    }
    statusLabel->setText(status);
    revertButton->setEnabled(manager->hasSnapshot());
}

void DlgSettingsCompatibilityProfiles::onApplyClicked()
{
    auto* manager = Application::Instance->compatibilityProfileManager();
    if (!manager) {
        return;
    }

    const auto profileId = profileCombo->currentData().toString().toStdString();
    try {
        if (manager->apply(profileId)) {
            requireRestart();
            QMessageBox::information(
                this,
                tr("Compatibility Profiles"),
                tr("Profile applied. Restart FreeCAD if navigation or theme changes do not "
                   "appear fully.")
            );
        }
    }
    catch (const Base::Exception& e) {
        QMessageBox::critical(this, tr("Compatibility Profiles"), QString::fromUtf8(e.what()));
    }
    refreshStatus();
}

void DlgSettingsCompatibilityProfiles::onRevertClicked()
{
    auto* manager = Application::Instance->compatibilityProfileManager();
    if (!manager) {
        return;
    }

    try {
        if (manager->revert()) {
            requireRestart();
            QMessageBox::information(
                this,
                tr("Compatibility Profiles"),
                tr("Baseline configuration restored.")
            );
        }
        else {
            QMessageBox::information(
                this,
                tr("Compatibility Profiles"),
                tr("No baseline snapshot is available to restore.")
            );
        }
    }
    catch (const Base::Exception& e) {
        QMessageBox::critical(this, tr("Compatibility Profiles"), QString::fromUtf8(e.what()));
    }

    loadSettings();
}

#include "moc_DlgSettingsCompatibilityProfiles.cpp"
