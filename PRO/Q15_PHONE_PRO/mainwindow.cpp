#include "mainwindow.h"
#include "ui_mainwindow.h"
static const QLatin1String serviceUuid("00001101-0000-1000-8000-00805F9B34FB");

QString hc_05_address = "00:18:E5:03:7C:49";
static const QString id_num = "01233222";
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    save_length = 0;
    this->his_index =   0;
    this->history_occupy_flag = false;
    timer = new QTimer(this);
    connect(timer,SIGNAL(timeout()),this,SLOT(timerUpdate()));
    timer->start(1000);
    discoveryAgent = new QBluetoothDeviceDiscoveryAgent();
    localDevice = new QBluetoothLocalDevice();
    socket = new QBluetoothSocket(QBluetoothServiceInfo::RfcommProtocol);
    ui->dateTimeEdit->setDisplayFormat("yyyy-MM-dd HH:mm:ss");
    ui->dateTimeEdit->setDateTime( QDateTime::currentDateTime() );

    connect(discoveryAgent,
            SIGNAL(deviceDiscovered(QBluetoothDeviceInfo)),
            this,
            SLOT(addBlueToothDevicesToList(QBluetoothDeviceInfo))
            );
    connect(ui->list,
            SIGNAL(itemActivated(QListWidgetItem*)),
            this,
            SLOT(itemActivated(QListWidgetItem*))
            );
    connect(socket,
            SIGNAL(readyRead()),
            this,
            SLOT(readBluetoothDataEvent())
            );
    connect(socket,
            SIGNAL(connected()),
            this,
            SLOT(bluetoothConnectedEvent())
            );
    connect(socket,
            SIGNAL(disconnected()),
            this,
            SLOT(bluetoothDisconnectedEvent())
            );

    if( localDevice->hostMode() == QBluetoothLocalDevice::HostPoweredOff ) {
        ui->pushButton_openBluetooth->setEnabled(true);
        ui->pushButton_closeDevice->setEnabled(false);
        ui->pushButton_scan->setEnabled(false);
    }else {
        ui->pushButton_openBluetooth->setEnabled(false);
        ui->pushButton_closeDevice->setEnabled(true);
        ui->pushButton_scan->setEnabled(true);
    }
    ui->pushButton_disconnect->setEnabled(false);
    if( localDevice->hostMode() == QBluetoothLocalDevice::HostDiscoverable ) {
        ui->checkBox_discoverable->setChecked(true);
    }else {
        ui->checkBox_discoverable->setChecked(false);
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::timerUpdate(void)
{
    ui->dateTimeEdit->setDateTime( QDateTime::currentDateTime() );

}

void MainWindow::on_pushButton_openBluetooth_clicked()
{
    localDevice->powerOn();
    ui->pushButton_closeDevice->setEnabled(true);
    ui->pushButton_openBluetooth->setEnabled(false);
    ui->pushButton_scan->setEnabled(true);
}

void MainWindow::on_pushButton_scan_clicked()
{
    discoveryAgent->start();
    ui->pushButton_scan->setEnabled(false);
}

void MainWindow::on_pushButton_disconnect_clicked()
{
    socket->disconnectFromService();
}

void MainWindow::on_pushButton_closeDevice_clicked()
{
    localDevice->setHostMode(QBluetoothLocalDevice::HostPoweredOff);
    ui->pushButton_closeDevice->setEnabled(true);
    ui->pushButton_openBluetooth->setEnabled(true);
    ui->pushButton_scan->setEnabled(true);

}

void MainWindow::on_pushButton_clear_clicked()
{
    ui->label_stateText->setText("Device normal.!");
    ui->textBrowser->clear();

}

void MainWindow::addBlueToothDevicesToList(const QBluetoothDeviceInfo &info)
{
    QString label = QString("%1 %2").arg(info.address().toString()).arg(info.name());

    QList<QListWidgetItem *> items = ui->list->findItems(label, Qt::MatchExactly);
    qDebug() << "the add label : " + label;
    if (items.empty()) {

        ui->textBrowser->append("address : " + info.address().toString());
        QListWidgetItem *item = new QListWidgetItem(label);
        QBluetoothLocalDevice::Pairing pairingStatus = localDevice->pairingStatus(info.address());

        if( !QString::compare( info.address().toString(),hc_05_address ) ) {
            this->bluetooth_label   =   label;
        }
        if (pairingStatus == QBluetoothLocalDevice::Paired || pairingStatus == QBluetoothLocalDevice::AuthorizedPaired )
            item->setTextColor(QColor(Qt::green));
        else
            item->setTextColor(QColor(Qt::black));
        ui->list->addItem(item);
    }
}

void MainWindow::itemActivated(QListWidgetItem *item)
{
    QString text = item->text();

    int index = text.indexOf(' ');

    if (index == -1)
        return;

    QBluetoothAddress address(text.left(index));
    QString name(text.mid(index + 1));
    qDebug() << "You has choice the bluetooth address is " << address;
    qDebug() << "The device is connneting.... ";
    QMessageBox::information(this,tr("Info"),tr("The device is connecting..."));
    socket->connectToService(address, QBluetoothUuid(serviceUuid) ,QIODevice::ReadWrite);

}
// 12    3  4     5  6     7  8      9  10    11 12  1314
// @@  *  8H+8L  *  8H+8L  *  8H+8L  *   8H+8L  *  8H+8L *  ##
//head   temp     humi      co        ch4      dust  tail
void MainWindow::readBluetoothDataEvent()
{
    HISTORY history_t;
    QByteArray rxArray;
    QString rxString;
    QByteArray array;
    QStringList cmd_array;
    QString temp_s,humi_s,co_s,ch4_s,dust_s;
    rxArray = socket->readAll();
    rxDataBuffer.append(rxArray);
    rxString.clear();
    rxString.append(rxDataBuffer);

    if( rxDataBuffer.contains( "@@" ) && rxDataBuffer.contains( "##" ) ) {
        rxString.clear();
        rxString.append(rxDataBuffer);

        // if the @@ > ##
        if( rxString.indexOf("@@") < rxString.indexOf("##") ) {

            history_t.time = QDateTime::currentDateTime();
            cmd_array = rxString.split('*');
            history_t.temp_value    = temp_s  =   QString(cmd_array.at(0)).mid(2,7);
            history_t.humi_value    = humi_s  =   cmd_array.at(1);
            history_t.co_value      = co_s    =   cmd_array.at(2);
            history_t.ch4_value     = ch4_s   =   cmd_array.at(3);
            history_t.dust_value    = dust_s  =   cmd_array.at(4);

            if( this->history_occupy_flag == false ) {
                this->display_all(&history_t);
            }

        }else{
            // 0000##@@0000000
            QString modi_str = "";
            modi_str.clear();
            modi_str.append( "@@" + rxString.right( rxString.indexOf("@@") ) );
            modi_str.append( rxString.mid(0, rxString.indexOf("##") ) );
            ui->textBrowser->append(modi_str);

        }
        rxDataBuffer.clear();
        rxString.clear();
    }
}
void    MainWindow::display_all(HISTORY *his)
{
    ui->dateTimeEdit->setDateTime(his->time);
    ui->lineEdit_temp_value->setText(his->temp_value);
    ui->lineEdit_humi_value->setText(his->humi_value);
    ui->lineEdit_co_value->setText(his->co_value);
    ui->lineEdit_ch4_value->setText(his->ch4_value);
    ui->lineEdit_dust_value->setText(his->dust_value);
}
void MainWindow::bluetoothConnectedEvent()
{

    discoveryAgent->stop();
    qDebug() << "The android device has been connected successfully!";
    QMessageBox::information(this,tr("Info"),tr("Successful connection!"));
    ui->pushButton_disconnect->setEnabled(true);
    ui->pushButton_scan->setEnabled(false);
    socket->write("Welcome use bluetooth.\n\r");
}

void MainWindow::bluetoothDisconnectedEvent()
{
    qDebug() << "The android device has been disconnected successfully!";
    QMessageBox::information(this,tr("Info"),tr("Successful disconnection!"));
    ui->pushButton_disconnect->setEnabled(false);
    ui->pushButton_scan->setEnabled(true);

}

void MainWindow::on_pushButton_save_history_clicked()
{
    HISTORY his_t;

    his_t.ch4_value = ui->lineEdit_ch4_value->text();
    his_t.co_value  =   ui->lineEdit_co_value->text();
    his_t.dust_value= ui->lineEdit_dust_value->text();
    his_t.humi_value = ui->lineEdit_humi_value->text();
    his_t.temp_value = ui->lineEdit_temp_value->text();
    his_t.time  =   QDateTime::currentDateTime();
    this->his[this->save_length] = his_t;
    this->save_length ++;
    ui->textBrowser->append("SYSTEM: save the "+QString::number(this->save_length)+ " data");
    if( this->save_length > 100 ) {
        this->save_length = 0;
    }
    //
}

void MainWindow::on_pushButton_view_history_back_clicked()
{
    uint8_t current_index = 0;
    this->history_occupy_flag = true;
    if( this->save_length < this->his_index ) {
        ui->textBrowser->append("SYSTEM: no history.");
        return;
    }
    current_index = this->save_length - this->his_index;
    this->display_all(&this->his[current_index]);
    ui->textBrowser->append("SYSTEM: VIEW "+QString::number(current_index)+ " data");
    this->his_index --;
    this->timer->stop();

}

void MainWindow::on_pushButton_view_history_forward_clicked()
{
    uint8_t current_index;
    this->history_occupy_flag = true;
    if( this->save_length == 0 ) {
        ui->textBrowser->append("SYSTEM: no history.");
        return;
    }
    current_index = this->save_length - this->his_index;
    if( current_index == 0 ) {
        ui->textBrowser->append("SYSTEM: no history.");
        return;
    }
    this->display_all(&this->his[current_index]);
    ui->textBrowser->append("SYSTEM: VIEW "+QString::number(current_index)+ " data");
    this->his_index ++;
    this->timer->stop();
}

void MainWindow::on_pushButton_dsiplay_current_clicked()
{
    this->history_occupy_flag = false;
    this->his_index = 0;
    this->timer->start(1000);

}


void MainWindow::on_checkBox_disp_only_clicked(bool checked)
{
    if( checked == true ) {
        discoveryAgent->stop();
        QListWidgetItem *item = new QListWidgetItem(bluetooth_label);
        ui->list->clear();
        ui->list->addItem(item);
    }else {
        ui->list->clear();
        on_pushButton_scan_clicked();
    }
}
