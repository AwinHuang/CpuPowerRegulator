#pragma once

#include <QtWidgets/QDialog>
#include <QTimer>
#include <QSystemTrayIcon>
#include "AwSystemCpuLoading.h"
#include "ui_CpuPowerRegulator.h"
#include "infoDialog.h"


class CpuPowerRegulator : public QDialog
{
    Q_OBJECT

public:
    CpuPowerRegulator(QWidget *parent = nullptr);
    ~CpuPowerRegulator();

public slots:
    void cpuPolling();
    void checkBox_autoAdjustPowerIndex_stateChanged(int state);
    void horizontalSlider_manualPowerIndex_valueChanged(int value);
    void showDialog();
    void exitApp();
    void showInfoDialog();

protected:
    void closeEvent(QCloseEvent* event);

private:
    void loadPreference();
    void savePerference();
    void showTrayIcon();
    void hideTrayIcon();

private:
    Ui::CpuPowerRegulatorClass ui;
    QTimer pollingTimer;
    AwSystemCpuLoading cpuLoading;
    QSystemTrayIcon* trayIcon = nullptr;
    QMenu* trayIconMenu = nullptr;
    QAction* actionShowDialog = nullptr;
    QAction* actionExit = nullptr;
    bool exitAppFlag = false;
    infoDialog* infoDlg = nullptr;
};
