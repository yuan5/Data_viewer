#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <iostream>
#include <fstream>
#include <stdio.h>

#include <QCheckBox>
#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_legend.h>
#include <qwt_plot_grid.h>
#include <QTimer>

#define CONFIG_ITEM_LENTH	(3)
#define SEL_LENTH			(10)

using namespace std;


typedef struct _LINE_DATA{
	QwtPlotCurve *Curve = NULL;
	QVector<double> y_data;
	QString Line_name;
	QCheckBox* sel_tab;
	int sel_show;
}LINE_DATA;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit MainWindow(QWidget *parent = 0);
	~MainWindow();

private slots:

	void Timer_update_handler(void);

	void on_C_B_line0_stateChanged(int arg1);

	void on_C_B_line1_stateChanged(int arg1);

	void on_C_B_line2_stateChanged(int arg1);

	void on_C_B_line3_stateChanged(int arg1);

	void on_C_B_line4_stateChanged(int arg1);

	void on_C_B_line5_stateChanged(int arg1);

	void on_C_B_line6_stateChanged(int arg1);

	void on_C_B_line7_stateChanged(int arg1);

	void on_C_B_line8_stateChanged(int arg1);

	void on_C_B_line9_stateChanged(int arg1);

	void on_Start_button_clicked();

	void on_Open_uart_button_clicked();

private:
	Ui::MainWindow *ui;
	QTimer *update_timer;

	//int Line_sel_tab[SEL_LENTH] = {0};
	QVector<double> x_data;
	LINE_DATA Line[SEL_LENTH];

	int16_t Zeng_Config_get(void);
	void Zeng_dis_command(string msg);
	void Zeng_dis_protocol(QString msg);
	void system_init(void);
	void Line_sec_func(int arg, int num);
	void Zeng_Curve_init(LINE_DATA *curve);
	void Zeng_Curve_update(LINE_DATA *curve, double data);
	void Zeng_Curve_updateAll(double *data);


	//QCheckBox* Line_tab[10];
};

#endif // MAINWINDOW_H
