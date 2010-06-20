#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <qwt_plot_curve.h>

#include "paramsdialog.h"


namespace Ui {
	class CMainWindow;
}

class CMainWindow : public QMainWindow
{
Q_OBJECT
public:
	CMainWindow(QWidget *parent = 0);
	~CMainWindow();

private:
	Ui::CMainWindow *ui;
	CParamsDialog params;
	QString filename;

	QVector<double> time;
	QVector<double> timeline;
	QVector<double> freqs;
	QVector<double> spectrum;
	QVector<double> expectations;
	QVector<double> variances;

	QwtPlotCurve time_graph;
	QwtPlotCurve spectrum_graph;
	QwtPlotCurve exp_graph;
	QwtPlotCurve var_graph;

	bool Analyze(const QString& fn);
	void Plot();
	void Error(const QString& text);

private slots:
	void Open();
	void Params();
	void AboutQt();

};

#endif // MAINWINDOW_H
