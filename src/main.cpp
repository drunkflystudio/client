#include "MainWindow.h"
#include "Network/Server.h"
#include <QApplication>
#include <ctime>
#include <cstdlib>

int main(int argc, char** argv)
{
    srand(time(NULL));

    QApplication::setOrganizationName(QStringLiteral("Drunk Fly Oy"));
    QApplication::setOrganizationDomain(QStringLiteral("studio.drunkfly.eu"));
    QApplication::setApplicationName(QStringLiteral("Drunk Fly Studio"));
    QApplication::setApplicationVersion(QStringLiteral("1.0"));

    QApplication app(argc, argv);
    app.setStyle(QStringLiteral("Fusion"));
    app.setStyleSheet(QStringLiteral(R"CSS(
            QPushButton {
                padding-top: 2px;
                padding-bottom: 3px;
                padding-left: 10px;
                padding-right: 10px;
            }
        )CSS"));

  #ifndef WASM_TARGET
    QFont defaultFont = QApplication::font();
    defaultFont.setPointSize(defaultFont.pointSize() + 1);
    qApp->setFont(defaultFont);
  #endif

    QPalette darkPalette;
    darkPalette.setColor(QPalette::Window, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::WindowText, Qt::white);
    darkPalette.setColor(QPalette::Disabled, QPalette::WindowText, QColor(127, 127, 127));
    darkPalette.setColor(QPalette::Base, QColor(42, 42, 42));
    darkPalette.setColor(QPalette::AlternateBase, QColor(66, 66, 66));
    darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
    darkPalette.setColor(QPalette::ToolTipText, Qt::white);
    darkPalette.setColor(QPalette::Text, Qt::white);
    darkPalette.setColor(QPalette::Disabled, QPalette::Text, QColor(127, 127, 127));
    darkPalette.setColor(QPalette::Dark, QColor(35, 35, 35));
    darkPalette.setColor(QPalette::Shadow, QColor(20, 20, 20));
    darkPalette.setColor(QPalette::Button, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::ButtonText, Qt::white);
    darkPalette.setColor(QPalette::Disabled, QPalette::ButtonText, QColor(127, 127, 127));
    darkPalette.setColor(QPalette::BrightText, Qt::red);
    darkPalette.setColor(QPalette::Link, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::Highlight, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::Disabled, QPalette::Highlight, QColor(80, 80, 80));
    darkPalette.setColor(QPalette::HighlightedText, Qt::white);
    darkPalette.setColor(QPalette::Disabled, QPalette::HighlightedText, QColor(127, 127, 127));
    darkPalette.setColor(QPalette::PlaceholderText, QColor(100, 100, 100));
    app.setPalette(darkPalette);

    auto server = new Server();
    server->openConnection();

    auto mainWindow = new MainWindow(server);
    mainWindow->show();

    return app.exec();
}
