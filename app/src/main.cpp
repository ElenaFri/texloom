#include <QApplication>
#include <QByteArray>
#include <QProcessEnvironment>
#include "ui/MainWindow.h"

int main(int argc, char *argv[])
{
    // Prevent noisy Gtk-WARNING theme parse errors on systems with
    // malformed custom GTK themes. Respect explicit user settings.
    if (qEnvironmentVariableIsEmpty("GTK_THEME"))
    {
        qputenv("GTK_THEME", QByteArray("Adwaita"));
    }

    // Some Linux themes ship malformed gtk.css that trigger noisy Gtk-WARNING
    // messages when Qt picks the GTK platform theme. Keep explicit user choice,
    // otherwise default to a non-GTK platform theme.
    if (qEnvironmentVariableIsEmpty("QT_QPA_PLATFORMTHEME"))
    {
        qputenv("QT_QPA_PLATFORMTHEME", QByteArray("xdgdesktopportal"));
    }

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
