#include <QApplication>
#include <QPushButton>
#include <QMessageBox>
#include "Qt/mainGui.h"
void handleButton() {
    QMessageBox::information(nullptr, "Hello", "Hello Linux!");
}

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    qRegisterMetaType<TransferStatus>("TransferStatus");
    TreeViewWidget w;
    w.showMaximized();
    /*QPushButton button("Click Me!");
    QObject::connect(&button, &QPushButton::clicked, handleButton);

    button.resize(200, 100);
    button.show();*/



    return app.exec();
}