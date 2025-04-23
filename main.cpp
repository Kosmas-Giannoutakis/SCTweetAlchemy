#include "mainwindow.h" // Include our main window definition
#include <QApplication>  // Include the Qt Application class

int main(int argc, char *argv[])
{
    // Create the Qt Application object
    QApplication app(argc, argv);

    // Create an instance of our main window
    MainWindow mainWindow;

    // Show the main window
    mainWindow.show();

    // Start the Qt event loop (waits for user interaction, etc.)
    // The application exits when this returns (e.g., window closed)
    return app.exec();
}