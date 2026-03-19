#include "main_window.h"
#include <QApplication>
#include <QStyleFactory>
#include <QDir>

using namespace eda;

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    // Set application properties
    app.setApplicationName("EDA Integrated");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("EDA Team");

    // Set style
    app.setStyle(QStyleFactory::create("Fusion"));

    // Create and show main window
    MainWindow window;
    window.show();

    // Load file if provided as argument
    if (argc > 1) {
        QString filename = QString::fromLocal8Bit(argv[1]);
        if (QFile::exists(filename)) {
            window.loadCircuit(filename.toStdString());
        }
    }

    return app.exec();
}
