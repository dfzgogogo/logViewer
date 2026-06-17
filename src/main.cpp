#include <QApplication>
#include "logviewerdialog.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // Default to current directory; pass a path on the command line to override.
    const QString logDir = (argc > 1) ? QString::fromLocal8Bit(argv[1]) : QStringLiteral(".");
    LogViewerDialog dlg(logDir);
    return dlg.exec();
}
