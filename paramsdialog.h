#ifndef PARAMSDIALOG_H
#define PARAMSDIALOG_H

#include <QDialog>

namespace Ui {
	class CParamsDialog;
}

class CParamsDialog : public QDialog
{
Q_OBJECT
public:
	CParamsDialog(QWidget *parent = 0);
	~CParamsDialog();

	double Sensivity() const;
	double WindowLength() const;
	int SecondsCount() const;

	bool changed;

private:
	Ui::CParamsDialog *ui;
	bool creating;

	double sensivity;
	double window_length;
	int seconds_count;


private slots:
	void Changed();

};

#endif // PARAMSDIALOG_H
