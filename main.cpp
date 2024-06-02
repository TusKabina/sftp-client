#include <QApplication>
#include <QPushButton>
#include <QMessageBox>

void handleButton() {
    QMessageBox::information(nullptr, "Hello", "Hello Linux!");
}

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    QPushButton button("Click Me!");
    QObject::connect(&button, &QPushButton::clicked, handleButton);

    button.resize(200, 100);
    button.show();

    return app.exec();
}