/*
 *
 * This file is part of the Notepadng text editor.
 *
 * Copyright(c) 2010 Notepadqq team.
 * http://notepadqq.sourceforge.net/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef NOTEPADNG_H
#define NOTEPADNG_H

#include "mainwindow.h"

#include "config.h"

#include <QApplication>
#include <QCommandLineParser>
#include <QFile>
#include <QList>
#include <QObject>
#include <QProcessEnvironment>
#include <QString>
#include <QUrl>

#define POINTVERSION CMAKE_PROJECT_VERSION // major.minor.revision

#define COPYRIGHT_YEAR "2026"

#define MIB_UTF_8 106

/**
 * @brief Global information and utility functions.
 */
class Notepadng : public QObject
{
    Q_OBJECT
public:
    static Notepadng &getInstance()
    {
        static Notepadng instance;
        return instance;
    }

    static const QString version;
    static const QString contributorsUrl;
    static const QString website;
    static QString copyright();
    static QString appDataPath(QString fileName = QString());
    static QString configDirPath();
    static QString fileNameFromUrl(const QUrl &url);
    static QSharedPointer<QCommandLineParser> getCommandLineArgumentsParser(const QStringList &arguments);

    static void showQtVersionWarning(bool showCheckBox, QWidget *parent = nullptr);

    static QList<QString> translations();

    /**
     * @brief Print environment information for debugging purposes.
     */
    static void printEnvironmentInfo();

signals:
    void newWindow(MainWindow *window);

private:
    Notepadng() {}
    Notepadng(Notepadng const &);      // Don't implement
    void operator=(Notepadng const &); // Don't implement
};

#endif // NOTEPADNG_H
