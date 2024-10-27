#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QSystemTrayIcon>

#include "screentaker.h"
#include "keycoordinates.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr, ScreenTaker* screentaker = nullptr, KeyCoordinates* key_сoordinates = nullptr);
    ~MainWindow();

private slots:
    void on_apply_settings_button_clicked();
    void onTrayIconActivated(QSystemTrayIcon::ActivationReason reason);
    void onExitTriggered();

private:
    Ui::MainWindow *ui;
    ScreenTaker* screentaker_;
    KeyCoordinates* key_coordinates_;
    QSystemTrayIcon *trayIcon;
    QMenu *trayMenu;

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;
    void closeEvent(QCloseEvent *event) override;
};
#endif // MAINWINDOW_H
