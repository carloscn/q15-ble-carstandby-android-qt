#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QList>
#include <QListWidgetItem>
#include <QMessageBox>
#include "QByteArray"
#include <QtBluetooth/qbluetoothglobal.h>
#include <QtBluetooth/qbluetoothlocaldevice.h>
#include <QtBluetooth/qbluetoothaddress.h>
#include <QtBluetooth/qbluetoothdevicediscoveryagent.h>
#include <QtBluetooth/qbluetoothlocaldevice.h>
#include <QtBluetooth/qbluetoothsocket.h>
#include <QGraphicsView>
#include <QTimer>
#include <qdatetime.h>

namespace Ui {
class MainWindow;
}

typedef struct  history {

    QDateTime   time;
    QString temp_value;
    QString humi_value;
    QString co_value;
    QString dust_value;
    QString ch4_value;

} HISTORY;


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:

    void addBlueToothDevicesToList(const QBluetoothDeviceInfo&);

    void itemActivated(QListWidgetItem *item);

    void readBluetoothDataEvent();

    void bluetoothConnectedEvent();

    void bluetoothDisconnectedEvent();

    void on_pushButton_openBluetooth_clicked();

    void on_pushButton_scan_clicked();

    void on_pushButton_disconnect_clicked();

    void on_pushButton_closeDevice_clicked();

    void on_pushButton_clear_clicked();

    void timerUpdate(void);

    void on_pushButton_save_history_clicked();

    void on_pushButton_view_history_back_clicked();

    void on_pushButton_view_history_forward_clicked();

    void on_pushButton_dsiplay_current_clicked();

    void on_checkBox_disp_only_clicked(bool checked);


private:
    Ui::MainWindow *ui;
    void display_all( HISTORY *his );
    QBluetoothDeviceDiscoveryAgent *discoveryAgent;
    QBluetoothLocalDevice *localDevice;
    QBluetoothSocket *socket;
    QByteArray rxDataBuffer;
    HISTORY his[100];
    uint8_t save_length;
    uint8_t his_index;
    bool history_occupy_flag;
    QString bluetooth_label;
    QTimer *timer;

};

#endif // MAINWINDOW_H
