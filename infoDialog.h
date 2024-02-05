#pragma once

#include <QDialog>
#include "ui_infoDialog.h"

class infoDialog : public QDialog
{
	Q_OBJECT

public:
	infoDialog(QWidget *parent = nullptr);
	~infoDialog();

private:
	Ui::infoDialogClass ui;
};
