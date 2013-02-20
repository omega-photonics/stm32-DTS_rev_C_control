#include "mainwindow.h"
#include "ui_mainwindow.h"


/* the device's vendor and product id */
#define MY_VID 0x0525
#define MY_PID 0x7959

/* the device's endpoints */
#define EP_IN 0x81
#define EP_OUT 0x01

unsigned char usb_in[64];
unsigned char usb_out[64];

usb_dev_handle* usb_connect()
{
    struct usb_bus *bus;
    struct usb_device *dev_tmp;

    int ret;

    ret=usb_find_busses();
    ret=usb_find_devices();

    for(bus = usb_get_busses(); bus; bus = bus->next)
    {
        for(dev_tmp = bus->devices; dev_tmp; dev_tmp = dev_tmp->next)
        {
            if(dev_tmp->descriptor.idVendor == MY_VID
                    && dev_tmp->descriptor.idProduct == MY_PID)
            {
                usb_dev_handle *dev = usb_open(dev_tmp);
                if(dev==NULL)
                    break;
                ret=usb_set_configuration(dev, 1);
                ret=usb_claim_interface(dev, 1);

                return dev;
            }
        }
    }
    return NULL;
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowFlags(Qt::WindowStaysOnTopHint);

    S = new QSettings("settings.ini", QSettings::IniFormat);
    ui->checkBox_led1->setChecked(S->value("LED1").toBool());
    ui->checkBox_led2->setChecked(S->value("LED2").toBool());
    ui->checkBox_pd1->setChecked(S->value("PD1").toBool());
    ui->checkBox_pd2->setChecked(S->value("PD2").toBool());
    ui->spinBox_dil->setValue(S->value("DIL").toInt());
    ui->spinBox_fol->setValue(S->value("FOL").toInt());
    ui->spinBox_gain1->setValue(S->value("Gain1").toInt());
    ui->spinBox_gain2->setValue(S->value("Gain2").toInt());
    ui->spinBox_hv1->setValue(S->value("HV1").toInt());
    ui->spinBox_hv2->setValue(S->value("HV2").toInt());
    ui->spinBox_shift1->setValue(S->value("Shift1").toInt());
    ui->spinBox_shift2->setValue(S->value("Shift2").toInt());
    ui->spinBox_slope1->setValue(S->value("Slope1").toInt());
    ui->spinBox_slope2->setValue(S->value("Slope2").toInt());

    usb_init();
    usbdevhandle = usb_connect();

    startTimer(200);
}

void MainWindow::timerEvent(QTimerEvent *)
{
    int ret=0;

    if(usbdevhandle!=NULL) ret=usb_bulk_read(usbdevhandle, EP_IN, (char*)usb_in, 64, 1000);
    if(ret!=64)
    {
        ui->label_status->setText("Connecting");
        if(usbdevhandle!=NULL) usb_close(usbdevhandle);
        usbdevhandle=NULL;
        usbdevhandle=usb_connect();
        if(usbdevhandle==NULL)
        {
            return;
        }
        else
        {
            ui->label_status->setText("Working");
        }
    }
    else ui->label_status->setText("Working");

    float temp_raw, temp_raw_max;
    float temp_C;
    temp_raw=(usb_in[0]<<8)|usb_in[1];
    temp_raw_max=(usb_in[2]<<8)|usb_in[3];
    if(temp_raw!=temp_raw_max) temp_C = 235.0 - 400.0*temp_raw/(temp_raw_max-temp_raw);
    else temp_C = -273.0;
    ui->label_t->setText(QString::number(temp_C));




    usb_out[63]=95; //marker

    int tc1=0, tv1=0;

    tv1=ui->spinBox_shift1->value();
    usb_out[tc1++]=tv1>>8;
    usb_out[tc1++]=tv1&255;
    S->setValue("Shift1", tv1);

    tv1=ui->spinBox_shift2->value();
    usb_out[tc1++]=tv1>>8;
    usb_out[tc1++]=tv1&255;
    S->setValue("Shift2", tv1);

    tv1=ui->spinBox_gain1->value();
    usb_out[tc1++]=tv1>>8;
    usb_out[tc1++]=tv1&255;
    S->setValue("Gain1", tv1);

    usb_out[tc1++] =
            (ui->checkBox_led2->isChecked()<<3) |
            (ui->checkBox_led1->isChecked()<<2) |
            (ui->checkBox_pd2->isChecked()<<1) |
            (ui->checkBox_pd1->isChecked());

    S->setValue("LED2", ui->checkBox_led2->isChecked());
    S->setValue("LED1", ui->checkBox_led1->isChecked());
    S->setValue("PD2", ui->checkBox_pd2->isChecked());
    S->setValue("PD1", ui->checkBox_pd1->isChecked());

    tv1=ui->spinBox_gain2->value();
    usb_out[tc1++]=tv1>>8;
    usb_out[tc1++]=tv1&255;
    S->setValue("Gain2", tv1);

    tv1=ui->spinBox_slope1->value();
    usb_out[tc1++]=tv1>>8;
    usb_out[tc1++]=tv1&255;
    S->setValue("Slope1", tv1);

    tv1=ui->spinBox_slope2->value();
    usb_out[tc1++]=tv1>>8;
    usb_out[tc1++]=tv1&255;
    S->setValue("Slope2", tv1);

    tv1=ui->spinBox_hv1->value();
    usb_out[tc1++]=tv1>>8;
    usb_out[tc1++]=tv1&255;
    S->setValue("HV1", tv1);

    tv1=ui->spinBox_hv2->value();
    usb_out[tc1++]=tv1>>8;
    usb_out[tc1++]=tv1&255;
    S->setValue("HV2", tv1);

    tv1=ui->spinBox_fol->value();
    usb_out[tc1++]=tv1>>8;
    usb_out[tc1++]=tv1&255;
    S->setValue("FOL", tv1);

    tv1=ui->spinBox_dil->value();
    usb_out[tc1++]=tv1>>8;
    usb_out[tc1++]=tv1&255;
    S->setValue("DIL", tv1);

    ret=usb_bulk_write(usbdevhandle, EP_OUT, (char*)usb_out, 64, 1000);
    if(ret!=64)
    {
        ui->label_status->setText("Connecting");
        if(usbdevhandle!=NULL) usb_close(usbdevhandle);
        usbdevhandle=NULL;
        usbdevhandle=usb_connect();
        if(usbdevhandle==NULL)
        {
            return;
        }
        else
        {
            ui->label_status->setText("Working");
        }
    }
    else ui->label_status->setText("Working");

}

MainWindow::~MainWindow()
{
    delete ui;
}
