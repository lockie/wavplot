
#include <cfloat>

#include <QFileDialog>
#include <QMessageBox>

#include <qwt_plot_magnifier.h>
#include <qwt_plot_panner.h>

#include <sndfile.hh>
#include <fftw3.h>

#include "mainwindow.h"
#include "ui_mainwindow.h"


#ifdef _MSC_VER
#	define isnan _isnan
#	define finite _finite
#endif  // _MSC_VER

CMainWindow::CMainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::CMainWindow)
{
	ui->setupUi(this);

	// цвет графиков
	time_graph.setPen(QPen(Qt::darkRed));
	spectrum_graph.setPen(QPen(Qt::darkMagenta));
	exp_graph.setPen(QPen(Qt::darkBlue));
	stddev_graph.setPen(QPen(Qt::green));

	// графики
	time_graph.attach(ui->pltTimeline);
	spectrum_graph.attach(ui->pltPassing);
	stddev_graph.attach(ui->pltAveraged);
	exp_graph.attach(ui->pltAveraged);

	// подписи к осям
	ui->pltTimeline->setAxisTitle(QwtPlot::xBottom, tr("time, s"));
	ui->pltTimeline->setAxisTitle(QwtPlot::yLeft, tr("amplitude, V"));
	ui->pltPassing->setAxisTitle(QwtPlot::xBottom, tr("frequency, Hz"));
	ui->pltPassing->setAxisTitle(QwtPlot::yLeft, tr("amplitude, dB"));
	ui->pltAveraged->setAxisTitle(QwtPlot::xBottom, tr("frequency, Hz"));
	ui->pltAveraged->setAxisTitle(QwtPlot::yLeft, tr("amplitude, dB"));

	// Крутилки
	QwtPlotMagnifier* magnifier = new QwtPlotMagnifier(ui->pltTimeline->canvas());
	magnifier->setAxisEnabled(QwtPlot::xBottom, false);
	magnifier->setAxisEnabled(QwtPlot::xTop, false);
	QwtPlotPanner* panner = new QwtPlotPanner(ui->pltTimeline->canvas());
	panner->setAxisEnabled(QwtPlot::xBottom, false);
	panner->setAxisEnabled(QwtPlot::xTop, false);
	magnifier = new QwtPlotMagnifier(ui->pltPassing->canvas());
	magnifier->setAxisEnabled(QwtPlot::xBottom, false);
	magnifier->setAxisEnabled(QwtPlot::xTop, false);
	panner = new QwtPlotPanner(ui->pltPassing->canvas());
	panner->setAxisEnabled(QwtPlot::xBottom, false);
	panner->setAxisEnabled(QwtPlot::xTop, false);
	magnifier = new QwtPlotMagnifier(ui->pltAveraged->canvas());
	magnifier->setAxisEnabled(QwtPlot::xBottom, false);
	magnifier->setAxisEnabled(QwtPlot::xTop, false);
	panner = new QwtPlotPanner(ui->pltAveraged->canvas());
	panner->setAxisEnabled(QwtPlot::xBottom, false);
	panner->setAxisEnabled(QwtPlot::xTop, false);
}

CMainWindow::~CMainWindow()
{
	delete ui;
}

static double sqr(double x)
{
	return x * x;
}

static double to_db(double v)
{
	return 20.0 * log10(v / 2e-5);
}

bool CMainWindow::Analyze(const QString &fn)
{
	// Параметры
	double sens = params.Sensivity();
	int spectrum_second = params.SecondsCount();
	double window_length = params.WindowLength();

	// Входной wav-файл
	SndfileHandle snd_file(fn.toLocal8Bit().data());
	if(!snd_file)
	{
		Error(tr("Unable to open file."));
		return false;
	}
	if(snd_file.channels() != 1)
	{
		Error(tr("Record must have 1 channel."));
		return false;
	}
	// Параметры
	snd_file.command(SFC_SET_NORM_DOUBLE, NULL, SF_TRUE);
	snd_file.command(SFC_SET_NORM_FLOAT, NULL, SF_TRUE);
	double discr_freq = snd_file.samplerate();
	// Число сэмплов
	int nsamples = discr_freq * window_length;
	int N = 1 + nsamples / 2;
	// Разместить массивы
	QVector<double> E;
	E.fill(0.0, N);
	QVector<double> E2;
	E2.fill(0.0, N);
	double* buffer = (double*)fftw_malloc(nsamples * sizeof(double));
	if(!buffer)
	{
		Error(tr("Unable to allocate memory."));
		return false;
	}
	fftw_complex* out = (fftw_complex*)
		fftw_malloc( N * sizeof(fftw_complex) );
	if(!out)
	{
		fftw_free(buffer);
		Error(tr("Unable to allocate memory."));
		return false;
	}
	// Запланировать БПФ
	fftw_plan plan =
		fftw_plan_dft_r2c_1d(nsamples, buffer, out, FFTW_ESTIMATE);
	if(!plan)
	{
		fftw_free(out);
		fftw_free(buffer);
		Error(tr("Unable to calculate FFT."));
		return false;
	}
	// Частоты
	freqs.clear();
	//freqs.resize(discr_freq / 2.0);
	for(double f = 1.0; f < discr_freq / 2.0; f++)
		freqs.append(f);
	// Опять массивы
	time.clear();
	timeline.clear();
	spectrum.clear();

	// Проехаться по файлу
	int length = 0;
	while(snd_file.read(buffer, nsamples)==nsamples)
	{
		for(int i = 0; i < nsamples; i++)
		{
			time.append((double)length + (double)i / nsamples);
			buffer[i] *= sens;
			timeline.append(buffer[i]);
		}
		fftw_execute(plan);

		// сосчитать модули
		QVector<double> x(N-1);
		x[0] = 0.0;
		for(int i = 1; i < N; i++)
			x[i-1] = sqrt( sqr(out[i][0]) + sqr(out[i][1]) ) / sqrt(nsamples);

		// сосчитать суммы Xi и Xi^2
		for(int i = 0; i < freqs.size(); i++)
		{
			double db = to_db(x[i]);
			Q_ASSERT(finite(db));
			if(length == spectrum_second)
				spectrum.append(db);
			E[i] += db;
			E2[i] += sqr(db);
		}
		length++;
	}

	expectations.clear();
	stddevs.clear();
	// Матожидания
	Q_ASSERT(length * window_length ==
		floor((double)snd_file.frames()/snd_file.samplerate()));
	for(int i = 0; i < freqs.size(); i++)
		expectations.append(E[i] / length);
	// Дисперсии
	for(int i = 0; i < freqs.size(); i++)
		stddevs.append( sqrt(E2[i] / length - sqr(expectations[i])) );

	// Освободить ресурсы
	fftw_destroy_plan(plan);
	fftw_free(out);
	fftw_free(buffer);

	return true;
}

void CMainWindow::Plot()
{
	time_graph.setData(time, timeline);
	spectrum_graph.setData(freqs, spectrum);
	exp_graph.setData(freqs, expectations);
	stddev_graph.setData(freqs, stddevs);

	ui->pltTimeline->replot();
	ui->pltPassing->replot();
	ui->pltAveraged->replot();
}

void CMainWindow::Error(const QString& text)
{
	QMessageBox::critical(this, tr("Error"), text);
}

void CMainWindow::Open()
{
	QString fn = QFileDialog::getOpenFileName(
		this,
		tr("Select file to open"),
		QDir::homePath(),
		tr("WAV files (*.wav)") );
	if(fn.isEmpty())
		return;
	if(Analyze(filename = fn))
		Plot();
}

void CMainWindow::Params()
{
	params.exec();
	if(params.changed && !filename.isEmpty())
		if(Analyze(filename))
			Plot();
	params.changed = false;
}

void CMainWindow::AboutQt()
{
	QApplication::aboutQt();
}
