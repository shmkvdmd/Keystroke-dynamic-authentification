#include "mainwindow.h"
#include "keylogger.h"
#include "screentaker.h"
#include "keycoordinates.h"
#include <QtSql/QSqlDatabase>
#include <filesystem>
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QKeySequence hotkey = QKeySequence(Qt::CTRL | Qt::Key_M);
    KeyLogger::SetHotkey(hotkey);

    KeyCoordinates key_coordinates;
    key_coordinates.FillDataBase();

    ScreenTaker screentaker;
    MainWindow w(nullptr, &screentaker, &key_coordinates);
    w.show();
    std::wstring screenshot_directory = L"../logs/screenshots";
    std::filesystem::create_directories(screenshot_directory);

    screentaker.StartTakingScreenshots(screenshot_directory);

    KeyLogger keylogger;
    keylogger.Start();
    return a.exec();
}
