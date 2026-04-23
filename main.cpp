#include <QApplication>
#include "eventdialog.h"
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    EventDialog createDialog;
    if (createDialog.exec() == QDialog::Accepted) {
        MainWindow mainWindow(createDialog.getJsonFilePath());
        mainWindow.show();
        return a.exec();
    }

    return 0;
}
