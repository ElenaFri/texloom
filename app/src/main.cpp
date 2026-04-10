#include <QApplication>
#include <QMainWindow>
#include <QLabel>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // Temporary main window for testing
    QMainWindow window;
    window.setWindowTitle("TexLoom - Development Build");
    window.resize(800, 600);
    
    QLabel* label = new QLabel("TexLoom is loading...\n\nThis is a placeholder window.", &window);
    label->setAlignment(Qt::AlignCenter);
    window.setCentralWidget(label);
    
    window.show();
    
    return app.exec();
}
