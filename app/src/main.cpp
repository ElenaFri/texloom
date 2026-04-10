#include <QApplication>
#include "ui/MainWindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // Set application metadata
    app.setApplicationName("TexLoom");
    app.setApplicationVersion("0.1.0");
    app.setOrganizationName("TexLoom");

    // Create and show main window
    texloom::MainWindow window;
    window.show();

    return app.exec();
}
