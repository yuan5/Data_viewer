#include "mainwindow.h"
#include "ui_mainwindow.h"


#include <qdebug.h>
#include <string.h>

#include "zeng_data_receive.h"

#define GET_LINE_TO_TABLE(num) Line[num].sel_tab = ui->C_B_line##num
#define dt  (0.03)
#define PI	(3.141592653)

typedef struct ZENG_CONFIG_DATA_{
	int line_num;
	int data_lenth;
	int update_period;
}ZENG_CONFIG_DATA;

const QColor Curve_color[SEL_LENTH]={
	Qt::red,
	Qt::black,
	Qt::green,
	Qt::blue,
	Qt::darkRed,
	Qt::darkBlue,
	Qt::darkGray,
	Qt::darkYellow,
	Qt::darkGreen,
	Qt::yellow
};

ifstream Config_file;


vector<string> Config_name = {"Line number: ",
							  "Slide lenth: ",
							  "Update period: "};
vector<string> Line_name;


ZENG_CONFIG_DATA config_data;

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow)
{
	ui->setupUi(this);

	system_init();
}

MainWindow::~MainWindow()
{
	Config_file.close();
	delete ui;
}

void MainWindow::Zeng_Curve_init(LINE_DATA *curve)
{
	static uint16_t thumb = 0;
	curve->Curve = new QwtPlotCurve();
	curve->Curve->setTitle(curve->Line_name);
	curve->Curve->setPen(Curve_color[thumb], 1);
	for(int i=0; i<config_data.data_lenth; i++)
	{
		curve->y_data.append(10*sin(dt*i+2*PI*thumb/config_data.line_num));
	}
	thumb++;
	curve->Curve->setSamples(x_data, curve->y_data);
	curve->Curve->attach(ui->Line_plot);
	curve->sel_show = 0;
}

void MainWindow::system_init(void)
{
	int ret = 0;
	update_timer = new QTimer();

	GET_LINE_TO_TABLE(0);
	GET_LINE_TO_TABLE(1);
	GET_LINE_TO_TABLE(2);
	GET_LINE_TO_TABLE(3);
	GET_LINE_TO_TABLE(4);
	GET_LINE_TO_TABLE(5);
	GET_LINE_TO_TABLE(6);
	GET_LINE_TO_TABLE(7);
	GET_LINE_TO_TABLE(8);
	GET_LINE_TO_TABLE(9);

	ret = Zeng_Config_get();
	if(ret)
	{
		Zeng_dis_command("[CFG] Get config data fail! ret = "+std::to_string(ret)+"\n");
	}else{
		Zeng_dis_command("[CFG] Get config data successful!\n");
		for(int i=0; i<config_data.data_lenth; i++)
		{
			x_data.append(dt*i);
		}
	}
	//ui->Line_plot->setTitle("Data Viewer");
	ui->Line_plot->setCanvasBackground(Qt::white);
	ui->Line_plot->insertLegend(new QwtLegend(), QwtPlot::RightLegend);
	for(int i=0; i<config_data.line_num; i++)
	{
		Zeng_Curve_init(&Line[i]);
	}

	connect(update_timer, SIGNAL(timeout()), this, SLOT(Timer_update_handler()));
	update_timer->stop();

	Zeng_data_receive_init();
}

void MainWindow::Zeng_Curve_update(LINE_DATA *curve, double data)
{
	curve->y_data.erase(curve->y_data.begin(), curve->y_data.begin()+1);

	curve->y_data.append(data);
	curve->Curve->setSamples(x_data, curve->y_data);
	curve->Curve->attach(ui->Line_plot);
}

void MainWindow::Zeng_Curve_updateAll(double *data)
{
	for(int j=0; j<config_data.line_num; j++)
	{
		if(Line[j].sel_show)
		{
			Zeng_Curve_update(&Line[j], data[j]);
		}else{
			//Zeng_Curve_update(&Line[j], 0);
		}
	}

	ui->Line_plot->replot();
}

/********************************** timer handle ************************************/
void MainWindow::Timer_update_handler(void)
{
	double update_line_data[SEL_LENTH];
	static uint cnt = 0;

	for(int i=0; i<config_data.line_num; i++)
	{
		update_line_data[i] = communication_data.data[i];//10*sin(config_data.data_lenth*dt+dt*cnt+2*PI*i/config_data.line_num);
	}

	Zeng_Curve_updateAll(update_line_data);

	cnt++;
}

void MainWindow::Zeng_dis_command(string msg)
{
	QString dis = QString::fromStdString(msg);
	ui->msg_dis->insertPlainText(dis);
	ui->msg_dis->moveCursor(QTextCursor::End);
}

void MainWindow::Zeng_dis_protocol(QString msg)
{
	msg += "\n";
	ui->procotol_dis->insertPlainText(msg);
	ui->procotol_dis->moveCursor(QTextCursor::End);
}

/********************************** get configure ***********************************/
int16_t MainWindow::Zeng_Config_get(void)
{
	int16_t ret = 0;
	string temp;
	string temp2;
	uint u32_temp;
	uint thumb_temp;
	float float_temp;
	char str_temp[32];

	int Config_num[CONFIG_ITEM_LENTH] = {0};

	Config_file.open("Config\\Config.txt");

	//
	while(getline(Config_file, temp))
	{
		for(uint i=0; i<Config_name.size(); i++)
		{
			temp2 = Config_name[i]+"%d";
			if(sscanf(temp.data(), temp2.data(), &Config_num[i])>0)
			{
				Zeng_dis_command("[cfg] " + Config_name[i]+std::to_string(Config_num[i])+"\n");
				ret++;
			}
		}

		//header length
		if(sscanf(temp.data(), "Data header lenth: %d", &u32_temp))
		{
			communication_data.head_length = u32_temp;
		//data length
		}else if(sscanf(temp.data(), "Data lenth: %d", &u32_temp)){
			communication_data.data_length = u32_temp;
		//if check sum
		}else if(sscanf(temp.data(), "Check sum: %s", str_temp)){
			if(('y'==str_temp[0])||('Y'==str_temp[0]))
			{
				communication_data.is_check = true;
				//qDebug()<<"check!!!"<<endl;
			}else{
				communication_data.is_check = false;
			}
		//header data
		}else if(sscanf(temp.data(), "Data header[%d]: 0x%x",&thumb_temp, &u32_temp)){
			communication_data.head[thumb_temp] = u32_temp;
		//checksum locate
		}else if(sscanf(temp.data(), "Checksum lenth: %d", &u32_temp)){
			communication_data.check_locate = u32_temp;
		//single data size
		}else if(sscanf(temp.data(), "Single data size: %d", &u32_temp)){
			communication_data.single_data_size = u32_temp;
		//data start
		}else if(sscanf(temp.data(), "Data start: %d", &u32_temp)){
			communication_data.data_start = u32_temp;
		//data scale
		}else if(sscanf(temp.data(), "Data scale: %f", &float_temp)){
			communication_data.data_scale = float_temp;
			//qDebug()<<"data scale:"<< communication_data.data_scale<<endl;
		}else if(sscanf(temp.data(), "Figure title: %s", str_temp)){
			ui->Line_plot->setTitle(str_temp);
		}else if(sscanf(temp.data(), "Line[%d] name: %s", &thumb_temp, str_temp)){
			Line_name.push_back(str_temp);
			Line[thumb_temp].Line_name = QString::fromStdString(str_temp);
			Line[thumb_temp].sel_tab->setText(Line[thumb_temp].Line_name);
		}
	}


	if(CONFIG_ITEM_LENTH == ret)
	{
		ret = 0;
		config_data.line_num = Config_num[ret++];
		config_data.data_lenth = Config_num[ret++];
		config_data.update_period = Config_num[ret++];
		communication_data.data_cnt = config_data.line_num;
	}


	//set display line
	if(Line_name.size() >= (uint32_t)config_data.line_num)
	{
		ret = 0;
	}else{
		for(int i=Line_name.size();i<config_data.line_num;i++)
		{
			Line_name.push_back("Line"+std::to_string(i));
			Line[i].Line_name = "Line"+QString::number(i);
			Zeng_dis_command("[Warn] line:"+std::to_string(i)+"\n");
		}
	}
	for(int i=config_data.line_num;i<SEL_LENTH;i++)
	{
		qDebug()<<"line number:"<<config_data.line_num<<endl;
		Line[i].sel_tab->setEnabled(false);
	}

	//get line name from file
//	if(CONFIG_ITEM_LENTH == ret)
//	{
//		ret = 0;
//		config_data.line_num = Config_num[ret++];
//		config_data.data_lenth = Config_num[ret++];
//		config_data.update_period = Config_num[ret++];
//		communication_data.data_cnt = config_data.line_num;
//		Config_file.close();

//		Config_file.open("Config\\Line_name.txt");
//		ret = 0;

//		//get line name
//		while(getline(Config_file, temp))
//		{
//			Line_name.push_back(temp);
//			Zeng_dis_command("[L_name] " + temp+"\n");

//			Line[ret].Line_name = QString::fromStdString(Line_name[ret]);
//			Line[ret].sel_tab->setText(Line[ret].Line_name);
//			ret++;
//		}
//		if(ret >= config_data.line_num)
//		{
//			ret = 0;
//		}else{
//			for(int i=ret; i<config_data.line_num; i++)
//			{
//				Line_name.push_back("Line"+std::to_string(i));
//				Line[ret].Line_name = "Line"+QString::number(i);
//				Zeng_dis_command("[Warn] line: Line" + std::to_string(i) + "\n");
//			}
//			ret = 0;
//		}
//		for(int i=config_data.line_num;i<SEL_LENTH;i++)
//		{
//			Line[i].sel_tab->setEnabled(false);
//		}
//	}


	//protocol message display
	Zeng_dis_protocol("Data length: "+QString::number(communication_data.data_length));
	Zeng_dis_protocol("Head_length: "+QString::number(communication_data.head_length));
	Zeng_dis_protocol("Head(已经从十六进制转化为10进制): ");
	for(uint i=0; i<communication_data.head_length; i++)
	{
		Zeng_dis_protocol(QString::number(communication_data.head[i]));
	}
	Zeng_dis_protocol("Data count: " + QString::number(communication_data.data_cnt));
	Zeng_dis_protocol("Single_data_size: "+QString::number(communication_data.single_data_size));
	Zeng_dis_protocol("Data scale: " + QString::number(communication_data.data_scale));
	if(communication_data.is_check)
	{
		Zeng_dis_protocol("Check status: yes");
		Zeng_dis_protocol("CheckSum locate: "+QString::number(communication_data.data_length-communication_data.check_locate));
	}else{
		Zeng_dis_protocol("Check status: no");
	}

	return ret;
}


/********************************** select item *************************************/
void MainWindow::Line_sec_func(int arg, int num)
{
	//Line_sel_tab[num] = arg;
	Line[num].sel_show = arg;
	//ui->Dis_plot->detachItems(Line[4].Curve->Rtti_PlotItem, false);

	if(arg)
	{
		Zeng_dis_command("[msg] " + Line_name[num] + " selected!\n");
	}else{
		Zeng_dis_command("[msg] " + Line_name[num] + " disselected!\n");
		ui->Line_plot->detachItems(Line[num].Curve->Rtti_PlotItem, false);
		//ui->Line_plot->removeItem(Line[num].Curve->QwtPlotItem);
	}

}

void MainWindow::on_C_B_line0_stateChanged(int arg1)
{
	Line_sec_func(arg1, 0);
}

void MainWindow::on_C_B_line1_stateChanged(int arg1)
{
	Line_sec_func(arg1, 1);
}

void MainWindow::on_C_B_line2_stateChanged(int arg1)
{
	Line_sec_func(arg1, 2);
}

void MainWindow::on_C_B_line3_stateChanged(int arg1)
{
	Line_sec_func(arg1, 3);
}

void MainWindow::on_C_B_line4_stateChanged(int arg1)
{
	Line_sec_func(arg1, 4);
}

void MainWindow::on_C_B_line5_stateChanged(int arg1)
{
	Line_sec_func(arg1, 5);
}

void MainWindow::on_C_B_line6_stateChanged(int arg1)
{
	Line_sec_func(arg1, 6);
}

void MainWindow::on_C_B_line7_stateChanged(int arg1)
{
	Line_sec_func(arg1, 7);
}

void MainWindow::on_C_B_line8_stateChanged(int arg1)
{
	Line_sec_func(arg1, 8);
}

void MainWindow::on_C_B_line9_stateChanged(int arg1)
{
	Line_sec_func(arg1, 9);
}
/********************************** select item *************************************/

/********************************* button clicked ***********************************/
void MainWindow::on_Start_button_clicked()
{

	if("Start" == ui->Start_button->text())
	{
		ui->Start_button->setText("Stop");
		update_timer->start(config_data.update_period);
		for(int i=0; i<config_data.line_num; i++)
		{
			if(Line[i].sel_show == 0)
			{
				ui->Line_plot->detachItems(Line[i].Curve->Rtti_PlotItem, false);
			}
		}
	}else{
		ui->Start_button->setText("Start");
		update_timer->stop();
	}
}
/********************************* button clicked ***********************************/

void MainWindow::on_Open_uart_button_clicked()
{
	int16_t ret = 0;
	if("Open" == ui->Open_uart_button->text())
	{
		ret = /*Zeng_input_usart_init*/Zeng_data_ready_task(ui->Com_sel->currentIndex(), ui->baudrate_sel->currentText().toInt());
		if(0 == ret)
		{
			Zeng_dis_command("Open " + ui->Com_sel->currentText().toStdString()\
							 +" baudrate:"\
							 +ui->baudrate_sel->currentText().toStdString() + " successful!\n");
			ui->Open_uart_button->setText("Close");
		}else{
			Zeng_dis_command("Open " + ui->Com_sel->currentText().toStdString()\
							 +" baudrate:"\
							 +ui->baudrate_sel->currentText().toStdString()\
							 +" fail!\n");
		}
	}else{
		if(Zeng_input_usart_close())
		{
			Zeng_data_suspend_task();
			ui->Open_uart_button->setText("Open");
		}
	}
}
