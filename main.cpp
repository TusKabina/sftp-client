#include <QApplication>
#include <QPushButton>
#include <QMessageBox>
#include "Qt/mainGui.h"
#include "Utilities/Logger.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    qRegisterMetaType<TransferStatus>("TransferStatus");
    qRegisterMetaType<QString>("QString");
    qRegisterMetaType<LogLevel>("LogLevel");

    TreeViewWidget w;
    w.showMaximized();

    return app.exec();
}