#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QUuid>
#include "ui_MainWindow.h"

class Server;
class ConnectingWidget;

class MainWindow final : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(Server* server, QWidget* parent = nullptr);
    ~MainWindow() override;

private:
    Ui_MainWindow m_ui;
    Server* m_server;
    QUuid m_currentProject;
    bool m_projectSelection;

    void updateTitle();

    void onServerStateChanged();

    Q_SLOT void on_actionNewProject_triggered();
    Q_SLOT void on_actionOpenProject_triggered();
    Q_SLOT void on_actionLogout_triggered();

    Q_DISABLE_COPY_MOVE(MainWindow)
};

#endif
