#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QKeySequenceEdit>
#include <QComboBox>
#include <QLineEdit>
#include <QIntValidator>
#include <QDoubleValidator>
#include <QKeySequence>
#include <QKeyEvent>
#include <QMenu>
#include <QCloseEvent>
#include <QAction>

#include "keylogger.h"
#include "keycoordinates.h"

MainWindow::MainWindow(QWidget *parent, ScreenTaker* screentaker, KeyCoordinates* key_сoordinates)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , screentaker_(screentaker)
    , key_coordinates_(key_сoordinates)
    , trayIcon(new QSystemTrayIcon(this))
{
    ui->setupUi(this);
    setWindowIcon(QIcon("../../icon.svg"));
    trayIcon->setIcon(QIcon("../../icon.svg"));
    trayIcon->setToolTip("KeyLogger");
    trayMenu = new QMenu(this);
    QAction *exitAction = new QAction("Выход", this);
    connect(exitAction, &QAction::triggered, this, &MainWindow::onExitTriggered);
    trayMenu->addAction(exitAction);
    trayIcon->setContextMenu(trayMenu);
    trayIcon->show();
    connect(trayIcon, &QSystemTrayIcon::activated, this, &MainWindow::onTrayIconActivated);

    if(QLineEdit *lineEdit = ui->hotkey_sequence->findChild<QLineEdit*>("qt_keysequenceedit_lineedit")) {
        lineEdit->setPlaceholderText("Ctrl + ...");
    }

    ui->hotkey_sequence->installEventFilter(this);
    QIntValidator *time_validator = new QIntValidator(1000, 60000, this);
    ui->duration_limit_lineEdit->setValidator(time_validator);
    ui->screen_interval_lineEdit->setValidator(time_validator);

    QRegularExpression regex("^\\d{1,2}(\\.\\d{0,2})?$");
    QRegularExpressionValidator *key_param_validator = new QRegularExpressionValidator(regex, this);
    ui->button_distance_lineEdit->setValidator(key_param_validator);
    ui->button_width_lineEdit->setValidator(key_param_validator);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (trayIcon->isVisible()) {
        hide();
        event->ignore();
    }
}

void MainWindow::onTrayIconActivated(QSystemTrayIcon::ActivationReason reason)
{
    if (reason == QSystemTrayIcon::Trigger) {
        if (isHidden()) {
            show();
            raise();
            activateWindow();
        } else {
            hide();
        }
    }
}

void MainWindow::onExitTriggered()
{
    QApplication::quit();
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == ui->hotkey_sequence) {
        if (event->type() == QEvent::KeyPress) {
            QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
            if (keyEvent->modifiers() & Qt::ControlModifier) {
                return QMainWindow::eventFilter(obj, event);
            } else {
                return true;
            }
        }
    }

    return QMainWindow::eventFilter(obj, event);
}

void MainWindow::on_apply_settings_button_clicked()
{
    int interval = ui->screen_interval_lineEdit->text().toInt();
    screentaker_->SetInterval(interval);
    KeyLogger::SetHotkey(ui->hotkey_sequence->keySequence());
    KeyLogger::SetDurationLimit(ui->duration_limit_lineEdit->text().toUInt());
    KeyCoordinates::SetGap(ui->button_distance_lineEdit->text().toDouble());
    KeyCoordinates::SetKeyWidth(ui->button_width_lineEdit->text().toDouble());
    key_coordinates_->FillDataBase();
}
