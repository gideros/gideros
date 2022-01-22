#include "qtutils.h"

#include <QDir>
#include <QFileInfo>
#include <QProcess>

void doShowInFinder(const QString& path){
#if defined(Q_OS_WIN)
        QString param;
        if (!QFileInfo(path).isDir())
            param = QLatin1String("/select,");
        param += QDir::toNativeSeparators(path);
        QProcess process;
        process.setProgram("explorer.exe");
        process.setNativeArguments(param);
        process.startDetached();

#elif defined(Q_OS_MAC)
    QStringList scriptArgs;
        scriptArgs << QLatin1String("-e")
                   << QString::fromLatin1("tell application \"Finder\" to reveal POSIX file \"%1\"")
                                         .arg(path);
        QProcess::execute(QLatin1String("/usr/bin/osascript"), scriptArgs);
        scriptArgs.clear();
        scriptArgs << QLatin1String("-e")
                   << QLatin1String("tell application \"Finder\" to activate");
        QProcess::execute("/usr/bin/osascript", scriptArgs);
#endif
}
