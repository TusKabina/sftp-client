#include <QApplication>
#include <QPushButton>
#include <QMessageBox>
#include "mainGui.h"
void handleButton() {
    QMessageBox::information(nullptr, "Hello", "Hello Linux!");
}

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    TreeViewWidget w;
    w.showMaximized();
    /*QPushButton button("Click Me!");
    QObject::connect(&button, &QPushButton::clicked, handleButton);

    button.resize(200, 100);
    button.show();*/

    return app.exec();
}