#include "EditorNS/scintillatheme.h"

#include <QDomDocument>
#include <QFile>

namespace EditorNS
{

int ScintillaTheme::LoadFromFile(const QString &themePath)
{
    QMap<QString, Style> newDefaultStyles;
    QMap<QString, QList<Style>> newLexerStyles;

    QFile themeFile(themePath);

    if (!themeFile.open(QIODevice::ReadOnly))
    {
        fprintf(stderr, "Failed to open %s\n", themePath.toUtf8().data());
        return -1;
    }

    QDomDocument themeDoc;

    QString parseErrorMessage;
    int parseErrorLine = 0;
    int parseErrorColumn = 0;

#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
    QDomDocument::ParseResult parseResult = themeDoc.setContent(&themeFile);
    if (!parseResult)
    {
        parseErrorMessage = parseResult.errorMessage;
        parseErrorLine = parseResult.errorLine;
        parseErrorColumn = parseResult.errorColumn;
    }
#else
    bool parseResult = themeDoc.setContent(&themeFile, &parseErrorMessage, &parseErrorLine, &parseErrorColumn);
#endif

    themeFile.close();

    if (!parseResult)
    {
        fprintf(stderr, "Failed to parse %s(%d:%d): %s\n", themePath.toUtf8().data(), parseErrorLine, parseErrorColumn, parseErrorMessage.toUtf8().data());
        return -1;
    }

    QDomElement widgetStyleElement = themeDoc.firstChildElement("NotepadPlus")
                                             .firstChildElement("GlobalStyles")
                                             .firstChildElement("WidgetStyle");

    for (; !widgetStyleElement.isNull(); widgetStyleElement = widgetStyleElement.nextSiblingElement())
    {
        QString styleName = widgetStyleElement.attribute("name");

        Style &style = newDefaultStyles[styleName];

        style.name = styleName;
        style.styleId = widgetStyleElement.attribute("styleID").toInt();
        style.fgColor = widgetStyleElement.attribute("fgColor").toUInt(nullptr, 16);
        style.bgColor = widgetStyleElement.attribute("bgColor").toUInt(nullptr, 16);
        style.fontStyle = widgetStyleElement.attribute("fontStyle").toInt();
    }

    // FIXME: Validate newDefaultStyles?

    QDomElement lexerTypeElement = themeDoc.firstChildElement("NotepadPlus")
                                       .firstChildElement("LexerStyles")
                                       .firstChildElement("LexerType");

    for (; !lexerTypeElement.isNull(); lexerTypeElement = lexerTypeElement.nextSiblingElement())
    {
        QString lexerName = lexerTypeElement.attribute("name");
        QList<Style> styles;

        QDomElement wordStyleElement = lexerTypeElement.firstChildElement("WordsStyle");

        for (; !wordStyleElement.isNull(); wordStyleElement = wordStyleElement.nextSiblingElement())
        {
            Style style;

            style.name = wordStyleElement.attribute("name");
            style.styleId = wordStyleElement.attribute("styleID").toInt();
            style.fgColor = wordStyleElement.attribute("fgColor").toUInt(nullptr, 16);
            style.bgColor = wordStyleElement.attribute("bgColor").toUInt(nullptr, 16);
            style.fontStyle = wordStyleElement.attribute("fontStyle").toInt();

            styles.append(style);
        }

        // FIXME: validate

        newLexerStyles[lexerName] = styles;
    }

    defaultStyles = newDefaultStyles;
    lexerStyles = newLexerStyles;

    return 0;
}

QString MapIdToNppLanguageName(const QString &id)
{
    return  id == "asn.1"       ? "asn1" :
            id == "csharp"      ? "cs" :
            id == "commonlisp"  ? "lisp" :
            id == "aspnet"      ? "asp" :
            id == "objective_c" ? "objc" :
            id == "properties"  ? "props" :
            id == "vbscript"    ? "vb" :
            id == "javascript"  ? "javascript.js" :
            id == "ejs"         ? "javascript" :
                                  id;   // Map as is
}

} // namespace EditorNS
