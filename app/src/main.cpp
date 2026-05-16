#include <QApplication>
#include <QByteArray>
#include <QSettings>
#include "ui/MainWindow.h"

int main(int argc, char *argv[])
{
    // Apply persisted GTK theme early (before QApplication), as platform theme
    // plugins are initialized at startup time.
    QSettings settings;
    settings.beginGroup(QStringLiteral("appearance"));
    const QString themeType = settings.value(QStringLiteral("themeType")).toString();
    const QString themeValue = settings.value(QStringLiteral("themeValue")).toString();
    settings.endGroup();

    if (themeType == QStringLiteral("gtk") && !themeValue.isEmpty())
    {
        qputenv("GTK_THEME", themeValue.toUtf8());
        qputenv("QT_QPA_PLATFORMTHEME", QByteArray("gtk3"));
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
