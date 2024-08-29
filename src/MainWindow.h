#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "ui_MainWindow.h"

class Server;
class ConnectingWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(Server* server, QWidget* parent = nullptr);
    ~MainWindow() override;

private:
    Ui_MainWindow m_ui;
    Server* m_server;

    void onServerStateChanged();

    Q_DISABLE_COPY(MainWindow)
};

#endif
