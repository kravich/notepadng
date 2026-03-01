#include "include/notepadng.h"

#include "include/nngsettings.h"

#include <QCheckBox>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QMessageBox>
#include <QStandardPaths>

const QString Notepadng::version = POINTVERSION;
const QString Notepadng::contributorsUrl = "https://github.com/kravich/notepadng/graphs/contributors";
const QString Notepadng::website = "https://notepadng.github.io";

QString Notepadng::copyright()
{
    return QString::fromUtf8("Copyright © %1, Evgeny Kravchenko\nCopyright © 2010-2025, Daniele Di Sarli").arg(COPYRIGHT_YEAR);
}

QString Notepadng::appDataPath(QString fileName)
{
#ifdef Q_OS_MACX
    QString def = QString("%1/../Resources/").arg(qApp->applicationDirPath());
#else
    QString def = QString("%1/../appdata/").arg(qApp->applicationDirPath());
#endif

    if (!QDir(def).exists())
        def = QString("%1/../share/%2/").arg(qApp->applicationDirPath()).arg(qApp->applicationName().toLower());

    if (!fileName.isNull())
    {
        def.append(fileName);
    }

    return def;
}

QString Notepadng::configDirPath()
{
    QStringList appConfigLocations = QStandardPaths::standardLocations(QStandardPaths::AppConfigLocation);
    return appConfigLocations.first();
}

QString Notepadng::fileNameFromUrl(const QUrl &url)
{
    return QFileInfo(url.toDisplayString(
                         QUrl::RemoveScheme |
                         QUrl::RemovePassword |
                         QUrl::RemoveUserInfo |
                         QUrl::RemovePort |
                         QUrl::RemoveAuthority |
                         QUrl::RemoveQuery |
                         QUrl::RemoveFragment |
                         QUrl::PreferLocalFile))
        .fileName();
}

QSharedPointer<QCommandLineParser> Notepadng::getCommandLineArgumentsParser(const QStringList &arguments)
{
    QSharedPointer<QCommandLineParser> parser =
        QSharedPointer<QCommandLineParser>(new QCommandLineParser());

    parser->setApplicationDescription("Text editor for developers");
    parser->addHelpOption();
    parser->addVersionOption();

    QCommandLineOption newWindowOption("new-window",
                                       QObject::tr("Open a new window in an existing instance of %1.")
                                           .arg(QCoreApplication::applicationName()));
    parser->addOption(newWindowOption);

    QCommandLineOption setLine({"l", "line"},
                               QObject::tr("Open file at specified line."),
                               "line",
                               "0");
    parser->addOption(setLine);

    QCommandLineOption setCol({"c", "column"},
                              QObject::tr("Open file at specified column."),
                              "column",
                              "0");
    parser->addOption(setCol);

    QCommandLineOption allowRootOption("allow-root", QObject::tr("Allows Notepadng to be run as root."));
    parser->addOption(allowRootOption);

    QCommandLineOption printDebugOption("print-debug-info", QObject::tr("Print system information for debugging."));
    parser->addOption(printDebugOption);

    parser->addPositionalArgument("urls",
                                  QObject::tr("Files to open."),
                                  "[urls...]");

    parser->process(arguments);

    return parser;
}

QList<QString> Notepadng::translations()
{
    QList<QString> out;

    QDir dir(":/translations");
    QStringList fileNames = dir.entryList(QStringList("notepadng_*.qm"));

    // FIXME this can be removed if we create a .qm file for English too, which should exist for consistency purposes
    out.append("en");

    for (int i = 0; i < fileNames.size(); ++i)
    {
        // get locale extracted by filename
        QString langCode;
        langCode = fileNames[i]; // "notepadng_de.qm"
        langCode.truncate(langCode.lastIndexOf('.')); // "notepadng_de"
        langCode.remove(0, langCode.indexOf('_') + 1); // "de"

        out.append(langCode);
    }

    return out;
}

void Notepadng::printEnvironmentInfo()
{
    qDebug() << QString("Notepadng: %1").arg(POINTVERSION).toStdString().c_str();
#ifdef BUILD_SNAP
    qDebug() << "Snap build: yes";
#else
    qDebug() << "Snap build: no";
#endif
    qDebug() << QString("Qt: %1 - %2").arg(qVersion(), QSysInfo::buildAbi()).toStdString().c_str();
    qDebug() << QString("OS: %1 (%2 %3)")
                    .arg(QSysInfo::prettyProductName(), QSysInfo::productType(), QSysInfo::productVersion())
                    .toStdString()
                    .c_str();
    qDebug() << QString("CPU: %1").arg(QSysInfo::currentCpuArchitecture()).toStdString().c_str();
    qDebug() << QString("Kernel: %1 - %2").arg(QSysInfo::kernelType(), QSysInfo::kernelVersion()).toStdString().c_str();
}
