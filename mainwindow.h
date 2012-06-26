#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSettings>
#include "lusb0_usb.h"
namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void timerEvent(QTimerEvent *);

private:
    usb_dev_handle *usbdevhandle;
    Ui::MainWindow *ui;
    QSettings *S;
};

#endif // MAINWINDOW_H
