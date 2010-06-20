
#include "paramsdialog.h"
#include "ui_paramsdialog.h"


CParamsDialog::CParamsDialog(QWidget *parent) :
	QDialog(parent),
	changed(false),
	ui(new Ui::CParamsDialog),
	creating(true),
	sensivity(1.0),
	window_length(1.0),
	seconds_count(1)
{
	ui->setupUi(this);

	ui->edtSensivity->setValue(sensivity);
	ui->edtWindowLen->setValue(window_length);
	ui->edtSecondsCount->setValue(seconds_count);

	creating = false;
}

CParamsDialog::~CParamsDialog()
{
	delete ui;
}

double CParamsDialog::Sensivity() const
{
	return sensivity;
}

double CParamsDialog::WindowLength() const
{
	return window_length;
}

int CParamsDialog::SecondsCount() const
{
	return seconds_count;
}

void CParamsDialog::Changed()
{
	if(!creating)
	{
		changed = true;

		sensivity = ui->edtSensivity->value();
		window_length = ui->edtWindowLen->value();
		seconds_count = ui->edtSecondsCount->value();
	}
}

