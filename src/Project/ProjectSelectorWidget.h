#ifndef PROJECT_PROJECTSELECTORWIDGET_H
#define PROJECT_PROJECTSELECTORWIDGET_H

#include <QWidget>
#include "ui_ProjectSelectorWidget.h"

class Server;

class ProjectSelectorWidget final : public QWidget
{
    Q_OBJECT

public:
    explicit ProjectSelectorWidget(QWidget* parent = nullptr);
    ~ProjectSelectorWidget() override;

    void setServer(Server* server);

    Q_SLOT void on_createButton_triggered();

private:
    Ui_ProjectSelectorWidget m_ui;
    Server* m_server;

    Q_DISABLE_COPY_MOVE(ProjectSelectorWidget)
};

#endif
