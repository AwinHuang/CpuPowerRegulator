#include "CpuPowerRegulator.h"
#include <windows.h>
#include <iostream>
#include <format>
#include <fstream>
#include <filesystem>
#include <QCloseEvent>
#include <QMenu>
#include <QAction>
#include "json.hpp"
#include "AwSystem.h"


CpuPowerRegulator::CpuPowerRegulator(QWidget *parent) : 
    QDialog(parent),
    cpuLoading()
{
    ui.setupUi(this);
    this->loadPreference();
    this->pollingTimer.start(1000);
    this->checkBox_autoAdjustPowerIndex_stateChanged(this->ui.checkBox_autoAdjustPowerIndex->checkState());

    connect(&this->pollingTimer, SIGNAL(timeout()), this, SLOT(cpuPolling()));
    connect(this->ui.checkBox_autoAdjustPowerIndex, SIGNAL(stateChanged(int)), this, SLOT(checkBox_autoAdjustPowerIndex_stateChanged(int)));
    connect(this->ui.horizontalSlider_manualPowerIndex, SIGNAL(valueChanged(int)), this, SLOT(horizontalSlider_manualPowerIndex_valueChanged(int)));
    connect(this->ui.pushButton_information, SIGNAL(clicked()), this, SLOT(showInfoDialog()));
}

CpuPowerRegulator::~CpuPowerRegulator()
{
    this->pollingTimer.stop();
    this->savePerference();
}

void CpuPowerRegulator::cpuPolling() {
    auto powerIndex = AwSystem::getCpuPowerIndex();
    auto cpuLoadingValue = cpuLoading.getValue();
    auto cpuFrequency = AwSystem::getCpuFrequency();

    this->ui.lcdNumber_currentPowerIndex->display(int(powerIndex));
    this->ui.lcdNumber_currentCpuFrequency->display(int(cpuFrequency));
    this->ui.lcdNumber_currentCpuLoading->display(int(cpuLoadingValue));

    auto cpuLoadingThreshold = this->ui.spinBox_cpuLoadingThreshold->value();
    auto cpuLoadingUpStep = this->ui.spinBox_cpuLoadingUpStep->value();
    auto cpuLoadingDownStep = this->ui.spinBox_cpuLoadingDownStep->value();
    auto powerIndexMin = ui.spinBox_cpuPowerIndexMin->value();
    auto powerIndexMax = ui.spinBox_cpuPowerIndexMax->value();

    if (cpuLoadingValue > cpuLoadingThreshold) {
        powerIndex = min(powerIndex + cpuLoadingUpStep, 100);
    } else {
        powerIndex = max(powerIndex - cpuLoadingDownStep, 1);
    }

    if (powerIndex > powerIndexMax) {
        powerIndex = powerIndexMax;
    }
    if (powerIndex < powerIndexMin) {
        powerIndex = powerIndexMin;
    }

    if (this->ui.checkBox_autoAdjustPowerIndex->checkState() == Qt::Checked) {
        AwSystem::setCpuPowerIndex(powerIndex);
        this->ui.horizontalSlider_manualPowerIndex->setSliderPosition(powerIndex);
    }
}

void CpuPowerRegulator::checkBox_autoAdjustPowerIndex_stateChanged(int state) {
    switch (state) {
    case Qt::Checked:
        this->ui.horizontalSlider_manualPowerIndex->setEnabled(false);
        this->ui.label_manualPowerIndexValue->setEnabled(false);

        this->ui.spinBox_cpuLoadingThreshold->setEnabled(true);
        this->ui.spinBox_cpuLoadingUpStep->setEnabled(true);
        this->ui.spinBox_cpuLoadingDownStep->setEnabled(true);
        this->ui.spinBox_cpuPowerIndexMin->setEnabled(true);
        this->ui.spinBox_cpuPowerIndexMax->setEnabled(true);
        break;

    case Qt::Unchecked:
        this->ui.horizontalSlider_manualPowerIndex->setEnabled(true);
        this->ui.label_manualPowerIndexValue->setEnabled(true);

        this->ui.spinBox_cpuLoadingThreshold->setEnabled(false);
        this->ui.spinBox_cpuLoadingUpStep->setEnabled(false);
        this->ui.spinBox_cpuLoadingDownStep->setEnabled(false);
        this->ui.spinBox_cpuPowerIndexMin->setEnabled(false);
        this->ui.spinBox_cpuPowerIndexMax->setEnabled(false);
        break;
    }
}

void CpuPowerRegulator::horizontalSlider_manualPowerIndex_valueChanged(int value) {
    if (value < 1 || value > 100) {
        return;
    }

    this->ui.lcdNumber_currentPowerIndex->display(value);
    AwSystem::setCpuPowerIndex(value);
}

void CpuPowerRegulator::showDialog() {
    this->hideTrayIcon();
    this->show();
}

void CpuPowerRegulator::exitApp() {
    this->exitAppFlag = true;
    this->close();
}

void CpuPowerRegulator::showInfoDialog() {
    if (infoDlg == nullptr) {
		infoDlg = new infoDialog(this);
	}
    if (infoDlg) {
        infoDlg->show();
    }
}

void CpuPowerRegulator::closeEvent(QCloseEvent* event) {
    if (this->exitAppFlag) {
		event->accept();
		return;
	}

    this->showTrayIcon();
    this->hide();
    event->ignore();
}

void CpuPowerRegulator::loadPreference() {
    nlohmann::json data;

    if (!std::filesystem::exists("pref.json")) {
        data = nlohmann::json::object();
    } else {
        std::ifstream f("pref.json");
        data = nlohmann::json::parse(f);
    }

    bool autoAdjust = data.contains("autoAdjust") ? data["autoAdjust"].get<bool>() : true;
    Qt::CheckState checkState = autoAdjust ? Qt::Checked : Qt::Unchecked;
    this->ui.checkBox_autoAdjustPowerIndex->setCheckState(checkState);
    this->checkBox_autoAdjustPowerIndex_stateChanged(checkState);

    int cpuLoadingThreshold = data.contains("cpuLoadingThreshold") ? data["cpuLoadingThreshold"].get<int>() : 10;
    this->ui.spinBox_cpuLoadingThreshold->setValue(cpuLoadingThreshold);

    int cpuLoadingUpStep = data.contains("cpuLoadingUpStep") ? data["cpuLoadingUpStep"].get<int>() : 10;
    this->ui.spinBox_cpuLoadingUpStep->setValue(cpuLoadingUpStep);

    int cpuLoadingDownStep = data.contains("cpuLoadingDownStep") ? data["cpuLoadingDownStep"].get<int>() : 20;
    this->ui.spinBox_cpuLoadingDownStep->setValue(cpuLoadingDownStep);

    int cpuPowerIndexMin = data.contains("cpuPowerIndexMin") ? data["cpuPowerIndexMin"].get<int>() : 50;
    this->ui.spinBox_cpuPowerIndexMin->setValue(cpuPowerIndexMin);

    int cpuPowerIndexMax = data.contains("cpuPowerIndexMax") ? data["cpuPowerIndexMax"].get<int>() : 100;
    this->ui.spinBox_cpuPowerIndexMax->setValue(cpuPowerIndexMax);
}

void CpuPowerRegulator::savePerference() {
    nlohmann::json j;

    j["autoAdjust"] = this->ui.checkBox_autoAdjustPowerIndex->checkState() == Qt::Checked ? true : false;
    j["cpuLoadingThreshold"] = this->ui.spinBox_cpuLoadingThreshold->value();
    j["cpuLoadingUpStep"] = this->ui.spinBox_cpuLoadingUpStep->value();
    j["cpuLoadingDownStep"] = this->ui.spinBox_cpuLoadingDownStep->value();
    j["cpuPowerIndexMin"] = this->ui.spinBox_cpuPowerIndexMin->value();
    j["cpuPowerIndexMax"] = this->ui.spinBox_cpuPowerIndexMax->value();

    std::ofstream out("pref.json");
    out << j.dump();;
    out.close();
}

void CpuPowerRegulator::showTrayIcon() {
    if (trayIcon == nullptr) {
        trayIcon = new QSystemTrayIcon(QIcon(":/CpuPowerRegulator/icon/frequency-64x64.png"), this);
    }
    if (trayIconMenu == nullptr) {
		trayIconMenu = new QMenu("PowerRegulator", this);
	}
    if (actionShowDialog == nullptr) {
        actionShowDialog = new QAction("Show Dialog", this);
		connect(actionShowDialog, SIGNAL(triggered()), this, SLOT(showDialog()));
    }
    if (actionExit == nullptr) {
        actionExit = new QAction("Exit", this);
        connect(actionExit, SIGNAL(triggered()), this, SLOT(exitApp()));
    }
    
    trayIconMenu->addAction(actionShowDialog);
    trayIconMenu->addAction(actionExit);
    trayIcon->setContextMenu(trayIconMenu);
    trayIcon->show();
}

void CpuPowerRegulator::hideTrayIcon() {
    if (trayIcon == nullptr) {
        return;
    }
    trayIcon->hide();
}