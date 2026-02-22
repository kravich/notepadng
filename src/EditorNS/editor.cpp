#include "EditorNS/editor.h"

#include "notepadng.h"
#include "nngsettings.h"

#include "Search/searchstring.h"

#include "EditorNS/lexerfactory.h"
#include "EditorNS/scintillatheme.h"

#include <QDir>
#include <QEventLoop>
#include <QMessageBox>
#include <QRegExp>
#include <QRegularExpression>
#include <QTimer>
#include <QVBoxLayout>

#include <cmath>

namespace EditorNS
{

const QString defaultFontFamily = "Courier New,Monospace";  // Use Courier New if availiable, otherwise fallback to any monospace font
const int defaultFontSizePt = 12;
const double defaultLineHeightEm = 1.0;

const int lineNumbersMarginIdx = 0;
const int foldingMarginIdx = 1;

const int foldingMarginWidthPx = 16;

namespace
{

int PtToPx(int pt)
{
    return pt * 72 / 96;
}

}

QQueue<QSharedPointer<Editor>> Editor::m_editorBuffer = QQueue<QSharedPointer<Editor>>();

Editor::Editor(QWidget *parent) :
    QWidget(parent)
{
    QString themeName = NngSettings::getInstance().Appearance.getColorScheme();

    fullConstructor(themeFromName(themeName));
}

Editor::Editor(const Theme &theme, QWidget *parent) :
    QWidget(parent)
{
    fullConstructor(theme);
}

void Editor::fullConstructor(const Theme &theme)
{
    m_scintilla = new CustomScintilla(this);

    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(0);
    m_layout->addWidget(m_scintilla, 1);
    setLayout(m_layout);

    connect(m_scintilla, &CustomScintilla::zoomChanged, this, &Editor::zoomChanged);
    connect(m_scintilla, &CustomScintilla::zoomChanged, this, &Editor::refreshMargins);

    connect(m_scintilla, &CustomScintilla::cursorPositionChanged, this, &Editor::requestDocumentInfo);
    connect(m_scintilla, &CustomScintilla::selectionChanged, this, &Editor::requestDocumentInfo);
    connect(m_scintilla, &CustomScintilla::textChanged, this, &Editor::requestDocumentInfo);

    connect(this, &Editor::documentInfoRequested, this, &Editor::cursorActivity);   // To trigger cursorActivity() also

    connect(m_scintilla, &CustomScintilla::textChanged, this, &Editor::contentChanged);
    connect(m_scintilla, &CustomScintilla::modificationChanged, this, [&]() { emit cleanChanged(isClean()); });
    connect(m_scintilla, &CustomScintilla::linesChanged, this, &Editor::refreshMargins);

    connect(m_scintilla, &CustomScintilla::textChanged, this, &Editor::incrementGeneration);
    // FIXME: Handle undo somehow to decrement generation

    connect(m_scintilla, &CustomScintilla::urlsDropped, this, &Editor::urlsDropped);
    connect(m_scintilla, &CustomScintilla::gotFocus, this, &Editor::gotFocus);
    setLanguage(nullptr);
    setTheme(theme);
}

QSharedPointer<Editor> Editor::getNewEditor(QWidget *parent)
{
    QSharedPointer<Editor> out;

    if (m_editorBuffer.length() == 0)
    {
        addEditorToBuffer(1);
        out = QSharedPointer<Editor>::create();
    }
    else if (m_editorBuffer.length() == 1)
    {
        addEditorToBuffer(1);
        out = m_editorBuffer.dequeue();
    }
    else
    {
        out = m_editorBuffer.dequeue();
    }

    out->setParent(parent);
    return out;
}

void Editor::addEditorToBuffer(const int howMany)
{
    for (int i = 0; i < howMany; i++)
        m_editorBuffer.enqueue(QSharedPointer<Editor>::create());
}

void Editor::insertContextMenuAction(QAction *before, QAction *action)
{
    m_scintilla->insertAction(before, action);
}

void Editor::invalidateEditorBuffer()
{
    m_editorBuffer.clear();
}

void Editor::setFocus()
{
    m_scintilla->setFocus();
}

void Editor::clearFocus()
{
    m_scintilla->clearFocus();
}

    /**
 * Automatically converts local relative file names to absolute ones.
 */
void Editor::setFilePath(const QUrl &filename)
{
    QUrl old = m_filePath;
    QUrl newUrl = filename;

    if (newUrl.isLocalFile())
        newUrl = QUrl::fromLocalFile(QFileInfo(filename.toLocalFile()).absoluteFilePath());

    m_filePath = newUrl;
    emit fileNameChanged(old, newUrl);
}

    /**
 * Always returns an absolute url.
 */
QUrl Editor::filePath() const
{
    return m_filePath;
}

QString Editor::tabName() const
{
    return m_tabName;
}

void Editor::setTabName(const QString &name)
{
    m_tabName = name;
}

void Editor::refreshMargins()
{
    m_scintilla->setMarginLineNumbers(lineNumbersMarginIdx, m_lineNumbersVisible);
    m_scintilla->setFolding(m_lineNumbersVisible ? CustomScintilla::BoxedTreeFoldStyle : CustomScintilla::NoFoldStyle, foldingMarginIdx); // FIXME: Make folding a setting separate from line numbers

    if (m_lineNumbersVisible)
    {
        QString lineNumbersMarginWidthString = QString(" ") + QString::number(m_scintilla->lines());

        m_scintilla->setMarginWidth(lineNumbersMarginIdx, lineNumbersMarginWidthString);
        m_scintilla->setMarginWidth(foldingMarginIdx, foldingMarginWidthPx);
    }
    else
    {
        m_scintilla->setMarginWidth(lineNumbersMarginIdx, 0);
        m_scintilla->setMarginWidth(foldingMarginIdx, 0);
    }
}

bool Editor::isClean()
{
    if (m_forceModified)
    {
        return false;
    }

    return !m_scintilla->isModified();
}

void Editor::markClean()
{
    m_forceModified = false;
    m_scintilla->setModified(false);
}

void Editor::markDirty()
{
    m_forceModified = true;
    emit cleanChanged(isClean());
}

int Editor::getHistoryGeneration()
{
    return m_generation;
}

void Editor::setLanguage(const Language *lang)
{
    if (lang == nullptr)
    {
        lang = LanguageService::getInstance().lookupById("plaintext");
    }

    m_currentLanguage = lang;
    setIndentationMode(lang);   // Refresh indentation settings

    refreshAppearance();

    emit currentLanguageChanged(m_currentLanguage->id, m_currentLanguage->name);
}

void Editor::setLanguage(const QString &language)
{
    auto &cache = LanguageService::getInstance();
    auto lang = cache.lookupById(language);
    if (lang != nullptr)
    {
        setLanguage(lang);
    }
}

void Editor::setLanguageFromFilePath(const QString &filePath)
{
    auto name = QFileInfo(filePath).fileName();

    auto &cache = LanguageService::getInstance();
    auto lang = cache.lookupByFileName(name);
    if (lang != nullptr)
    {
        setLanguage(lang);
        return;
    }
    lang = cache.lookupByExtension(name);
    if (lang != nullptr)
    {
        setLanguage(lang);
    }
}

void Editor::setLanguageFromFilePath()
{
    setLanguageFromFilePath(filePath().toString());
}

void Editor::setIndentationMode(const Language *lang)
{
    const auto &s = NngSettings::getInstance().Languages;
    const bool useDefaults = s.getUseDefaultSettings(lang->id);
    const auto &langId = useDefaults ? "default" : lang->id;

    setIndentationMode(!s.getIndentWithSpaces(langId), s.getTabSize(langId));
}

bool Editor::searchAndSelect(bool inSelection,
                             const QString &string,
                             SearchHelpers::SearchMode searchMode,
                             bool forward,
                             const SearchHelpers::SearchOptions &searchOptions,
                             bool wrap)
{
    QString expr = string;

    bool isRegex = (searchMode == SearchHelpers::SearchMode::Regex);
    bool isCaseSensitive = searchOptions.MatchCase;
    bool isWholeWord = searchOptions.MatchWholeWord;

    if (searchMode == SearchHelpers::SearchMode::SpecialChars)
    {
        // Implement searching for special characters through regex
        isRegex = true;
        expr = SearchString::format(expr, searchMode, searchOptions);   // FIXME: Ensure correctness of formatting
    }

    bool isFoundAndSelected = false;

    if (inSelection)
    {
        isFoundAndSelected = m_scintilla->findFirstInSelection(expr, isRegex, isCaseSensitive, isWholeWord, wrap, forward);
    }
    else
    {
        int selectionLineFrom = -1;
        int selectionIndexFrom = -1;
        int selectionLineTo = -1;
        int selectionIndexTo = -1;

        m_scintilla->getSelection(&selectionLineFrom, &selectionIndexFrom, &selectionLineTo, &selectionIndexTo);

        // WA: If searching backward, start search from the beginning of selection, not the end.
        //     This way we won't stuck on already found text
        // FIXME: Study this in more detail and understand if this is a QScintilla bug or not
        int line = forward ? selectionLineTo : selectionLineFrom;
        int index = forward ? selectionIndexTo : selectionIndexFrom;

        isFoundAndSelected = m_scintilla->findFirst(expr, isRegex, isCaseSensitive, isWholeWord, wrap, forward, line, index);
    }

    return isFoundAndSelected;
}

void Editor::incrementGeneration()
{
    m_generation++;
}

void Editor::setIndentationMode(const bool useTabs, const int size)
{
    m_scintilla->setIndentationsUseTabs(useTabs);
    m_scintilla->setTabWidth(size);
}

Editor::IndentationMode Editor::indentationMode()
{
    IndentationMode out;
    out.useTabs = m_scintilla->indentationsUseTabs();
    out.size = m_scintilla->tabWidth();
    return out;
}

void Editor::setCustomIndentationMode(const bool useTabs, const int size)
{
    m_customIndentationMode = true;
    setIndentationMode(useTabs, size);
}

void Editor::setCustomIndentationMode(const bool useTabs)
{
    m_customIndentationMode = true;
    setIndentationMode(useTabs, 0);
}

void Editor::clearCustomIndentationMode()
{
    m_customIndentationMode = false;
    setIndentationMode(getLanguage());
}

bool Editor::isUsingCustomIndentationMode() const
{
    return m_customIndentationMode;
}

void Editor::setSmartIndent(bool enabled)
{
    m_scintilla->setAutoIndent(enabled);
}

void Editor::setValue(const QString &value)
{
    auto lang = LanguageService::getInstance().lookupByContent(value);
    if (lang != nullptr)
    {
        setLanguage(lang);
    }
    m_scintilla->setText(value);
}

QString Editor::value()
{
    return m_scintilla->text();
}

bool Editor::fileOnDiskChanged() const
{
    return m_fileOnDiskChanged;
}

void Editor::setFileOnDiskChanged(bool fileOnDiskChanged)
{
    m_fileOnDiskChanged = fileOnDiskChanged;
}

void Editor::setZoomFactor(int factor)
{
    m_scintilla->zoomTo(factor);
}

int Editor::zoomFactor() const
{
    return m_scintilla->getZoom();
}

void Editor::setSelectionsText(const QStringList &texts, SelectMode mode)
{
    QString replacement = texts.join("\n");
    m_scintilla->replaceSelectedText(replacement);
    fprintf(stderr, "FIXME: support cursor placement according to 'mode' in Editor::setSelectionsText()\n");
}

void Editor::setSelectionsText(const QStringList &texts)
{
    setSelectionsText(texts, SelectMode::After);
}

void Editor::insertBanner(QWidget *banner)
{
    m_layout->insertWidget(0, banner);
}

void Editor::removeBanner(QWidget *banner)
{
    if (banner != m_scintilla && m_layout->indexOf(banner) >= 0)
    {
        m_layout->removeWidget(banner);
        emit bannerRemoved(banner);
    }
}

void Editor::removeBanner(QString objectName)
{
    for (auto &&banner : findChildren<QWidget *>(objectName))
    {
        removeBanner(banner);
    }
}

void Editor::setLineWrap(const bool wrap)
{
    m_scintilla->setWrapMode(wrap ? CustomScintilla::WrapWord : CustomScintilla::WrapNone);
}

void Editor::setEOLVisible(const bool showeol)
{
    m_scintilla->setEolVisibility(showeol);
}

void Editor::setWhitespaceVisible(const bool showspace)
{
    m_scintilla->setWhitespaceVisibility(showspace ? CustomScintilla::WsVisible : CustomScintilla::WsInvisible);
}

void Editor::setIndentGuideVisible(bool showindentguide)
{
    m_scintilla->setIndentationGuides(showindentguide);
}

void Editor::requestDocumentInfo()
{
    // Cursor
    int cursorLine = 0;
    int cursorIndex = 0;

    m_scintilla->getCursorPosition(&cursorLine, &cursorIndex);

    // Selections
    int selectedLines = 0;
    int selectedChars = 0;

    if (m_scintilla->hasSelectedText())
    {
        int selectedLineFrom = 0;
        int selectedIndexFrom = 0;
        int selectedLineTo = 0;
        int selectedIndexTo = 0;

        m_scintilla->getSelection(&selectedLineFrom, &selectedIndexFrom, &selectedLineTo, &selectedIndexTo);

        selectedLines = selectedLineTo - selectedLineFrom + 1;
        selectedChars = m_scintilla->selectedText().length();
    }

    // Content
    int linesCount = m_scintilla->lines();
    int charsCount = m_scintilla->text().length();

    CursorInfo cursorInfo;

    cursorInfo.cursor.line = cursorLine;
    cursorInfo.cursor.column = cursorIndex;

    cursorInfo.selectedLines = selectedLines;
    cursorInfo.selectedChars = selectedChars;

    cursorInfo.totalLines = linesCount;
    cursorInfo.totalChars = charsCount;

    emit documentInfoRequested(cursorInfo);
}

QPair<int, int> Editor::cursorPosition()
{
    int cursorLine = 0;
    int cursorIndex = 0;

    m_scintilla->getCursorPosition(&cursorLine, &cursorIndex);

    return {cursorLine, cursorIndex};
}

void Editor::setCursorPosition(const int line, const int column)
{
    m_scintilla->setCursorPosition(line, column);
}

void Editor::setCursorPosition(const QPair<int, int> &position)
{
    setCursorPosition(position.first, position.second);
}

void Editor::setCursorPosition(const Cursor &cursor)
{
    setCursorPosition(cursor.line, cursor.column);
}

void Editor::setSelection(int fromLine, int fromCol, int toLine, int toCol)
{
    m_scintilla->setSelection(fromLine, fromCol, toLine, toCol);
}

QPair<int, int> Editor::scrollPosition()
{
    int line = m_scintilla->firstVisibleLine();
    return {0, line};   // FIXME: Support horizontal scrolling
}

void Editor::setScrollPosition(const int left, const int top)
{
    m_scintilla->setXOffset(0); // FIXME: Support horizontal scrolling
    m_scintilla->setFirstVisibleLine(top);
}

void Editor::setScrollPosition(const QPair<int, int> &position)
{
    setScrollPosition(position.first, position.second);
}

QString Editor::endOfLineSequence() const
{
    return m_scintilla->eolMode() == CustomScintilla::EolWindows ? "\r\n" :
           m_scintilla->eolMode() == CustomScintilla::EolUnix    ? "\n" :
                                                                   "\r";
}

void Editor::setEndOfLineSequence(const QString &newLineSequence)
{
    CustomScintilla::EolMode eolMode = newLineSequence == "\r\n" ? CustomScintilla::EolWindows :
                                       newLineSequence == "\n"   ? CustomScintilla::EolUnix :
                                                                   CustomScintilla::EolMac;

    m_scintilla->setEolMode(eolMode);
    m_scintilla->convertEols(eolMode);
    m_scintilla->append("");    // Reset history
}

void Editor::setFont(QString fontFamily, int fontSize, double lineHeight)
{
    m_currentFontFamily = fontFamily;
    m_currentFontSizePt = fontSize;
    m_currentLineHeightEm = lineHeight;
    refreshAppearance();
}

void Editor::setLineNumbersVisible(bool visible)
{
    m_lineNumbersVisible = visible;

    refreshMargins();
}

QTextCodec *Editor::codec() const
{
    return m_codec;
}

void Editor::setCodec(QTextCodec *codec)
{
    m_codec = codec;
}

bool Editor::bom() const
{
    return m_bom;
}

void Editor::setBom(bool bom)
{
    m_bom = bom;
}

Editor::Theme Editor::themeFromName(QString name)
{
    if (name == "default" || name.isEmpty())
        return Theme();

    QFileInfo configDirPathInfo = QFileInfo(Notepadng::configDirPath());
    QDir userThemesDir(configDirPathInfo.absolutePath() + "/themes/");

    if (userThemesDir.exists(name + ".xml"))
        return Theme(name, userThemesDir.filePath(name + ".xml"));

    QFileInfo appDataPathInfo(Notepadng::appDataPath());
    QDir bundledThemesDir(appDataPathInfo.absolutePath() + "/themes/");

    if (bundledThemesDir.exists(name + ".xml"))
        return Theme(name, bundledThemesDir.filePath(name + ".xml"));

    return Theme();
}

QList<Editor::Theme> Editor::themes()
{
    QList<Theme> out;

    // Load user themes
    QFileInfo configDirPathInfo = QFileInfo(Notepadng::configDirPath());
    QDir userThemesDir(configDirPathInfo.absolutePath() + "/themes/", "*.xml");

    for (auto &&theme : userThemesDir.entryInfoList())
    {
        if (theme.completeBaseName() == "Default")
        {
            // Do not include Default.xml into list
            continue;
        }

        out.append(Theme(theme.completeBaseName(), theme.filePath()));
    }

    // Load system-wide themes
    auto appDataPathInfo = QFileInfo(Notepadng::appDataPath());
    QDir bundledThemesDir(appDataPathInfo.absolutePath() + "/themes/", "*.xml");

    for (auto &&theme : bundledThemesDir.entryInfoList())
    {
        if (theme.completeBaseName() == "Default")
        {
            // Do not include Default.xml into list
            continue;
        }

        out.append(Theme(theme.completeBaseName(), theme.filePath()));
    }

    return out;
}

void Editor::setTheme(Theme theme)
{
    m_currentTheme = theme;
    refreshAppearance();
}

QList<Editor::Selection> Editor::selections()
{
    QList<Selection> out;

    if (m_scintilla->hasSelectedText())
    {
        // FIMXE: Support several selections

        int selectedLineFrom = 0;
        int selectedIndexFrom = 0;
        int selectedLineTo = 0;
        int selectedIndexTo = 0;

        m_scintilla->getSelection(&selectedLineFrom, &selectedIndexFrom, &selectedLineTo, &selectedIndexTo);

        Editor::Selection selection;
        selection.from.line = selectedLineFrom;
        selection.from.column = selectedIndexFrom;
        selection.to.line = selectedLineTo;
        selection.to.column = selectedIndexTo;

        out.append(selection);
    }

    return out;
}

QStringList Editor::selectedTexts()
{
    QString selectedText = m_scintilla->selectedText();

    QStringList selectedTextsList;

    if (selectedText.length() > 0)
    {
        // FIMXE: Support several selections
        selectedTextsList.append(selectedText);
    }

    return selectedTextsList;
}

void Editor::setOverwrite(bool overwrite)
{
    m_scintilla->setOverwriteMode(overwrite);
}

void Editor::print(std::shared_ptr<QPrinter> printer)
{
    fprintf(stderr, "FIXME: Implement Editor::print()\n");
}

QByteArray Editor::printToPdf(const QPageLayout &pageLayout)
{
    fprintf(stderr, "FIXME: Implement Editor::printToPdf()\n");
    return QByteArray();
}

int Editor::lineCount()
{
    return m_scintilla->lines();
}

void Editor::selectAll()
{
    m_scintilla->selectAll();
}

void Editor::undo()
{
    m_scintilla->undo();
}

void Editor::redo()
{
    m_scintilla->redo();
}

void Editor::deleteCurrentLine()
{
    m_scintilla->lineDelete();
}

void Editor::duplicateCurrentLine()
{
    m_scintilla->lineDuplicate();
}

void Editor::moveCurrentLineUp()
{
    m_scintilla->moveSelectedLinesUp();
}

void Editor::moveCurrentLineDown()
{
    m_scintilla->moveSelectedLinesDown();
}

void Editor::trimTrailingWhitespaces()
{
    m_scintilla->beginUndoAction();
    replaceAllNoCheckpoint("\\s+$", SearchHelpers::SearchMode::Regex, SearchHelpers::SearchOptions(), "");
    m_scintilla->endUndoAction();
}

void Editor::trimLeadingWhitespaces()
{
    m_scintilla->beginUndoAction();
    replaceAllNoCheckpoint("^\\s+", SearchHelpers::SearchMode::Regex, SearchHelpers::SearchOptions(), "");
    m_scintilla->endUndoAction();
}

void Editor::trimLeadingAndTrailingWhitespaces()
{
    m_scintilla->beginUndoAction();
    replaceAllNoCheckpoint("^\\s+", SearchHelpers::SearchMode::Regex, SearchHelpers::SearchOptions(), "");
    replaceAllNoCheckpoint("\\s+$", SearchHelpers::SearchMode::Regex, SearchHelpers::SearchOptions(), "");
    m_scintilla->endUndoAction();
}

void Editor::convertEolToSpace()
{
    fprintf(stderr, "FIXME: Implement Editor::convertEolToSpace()\n");
}

void Editor::convertTabsToSpaces()
{
    IndentationMode indentation = indentationMode();

    QString spacesReplacement = QString(" ").repeated(indentation.size);

    m_scintilla->beginUndoAction();
    replaceAllNoCheckpoint("\\t", SearchHelpers::SearchMode::Regex, SearchHelpers::SearchOptions(), spacesReplacement);
    m_scintilla->endUndoAction();
}

void Editor::convertAllSpacesToTabs()
{
    IndentationMode indentation = indentationMode();

    QString regex = QString(" ").repeated(indentation.size);

    m_scintilla->beginUndoAction();
    replaceAllNoCheckpoint(regex, SearchHelpers::SearchMode::Regex, SearchHelpers::SearchOptions(), "\t");
    m_scintilla->endUndoAction();
}

void Editor::convertLeadingSpacesToTabs()
{
    fprintf(stderr, "FIXME: Implement Editor::convertLeadingSpacesToTabs()\n");
}

void Editor::search(const QString &string,
                    SearchHelpers::SearchMode searchMode,
                    bool forward,
                    const SearchHelpers::SearchOptions &searchOptions)
{
    searchAndSelect(false, string, searchMode, forward, searchOptions, true);
}

void Editor::replace(const QString &string,
                     SearchHelpers::SearchMode searchMode,
                     bool forward,
                     const SearchHelpers::SearchOptions &searchOptions,
                     const QString &replacement)
{
    bool isFoundAndSelected = searchAndSelect(true, string, searchMode, forward, searchOptions, true);

    if (!isFoundAndSelected)
    {
        isFoundAndSelected = searchAndSelect(false, string, searchMode, forward, searchOptions, true);
    }

    if (isFoundAndSelected)
    {
        m_scintilla->replace(replacement);
    }
}

int Editor::replaceAll(const QString &string,
                       SearchHelpers::SearchMode searchMode,
                       const SearchHelpers::SearchOptions &searchOptions,
                       const QString &replacement)
{
    m_scintilla->beginUndoAction();
    int count = replaceAllNoCheckpoint(string, searchMode, searchOptions, replacement);
    m_scintilla->endUndoAction();

    return count;
}

int Editor::replaceAllNoCheckpoint(const QString &string,
                                   SearchHelpers::SearchMode searchMode,
                                   const SearchHelpers::SearchOptions &searchOptions,
                                   const QString &replacement)
{
    int count = 0;

    m_scintilla->setCursorPosition(0, 0);

    while (searchAndSelect(false, string, searchMode, true, searchOptions, false))
    {
        m_scintilla->replace(replacement);
        count++;
    }

    return count;
}

QFont StyleFont(QFont font, int fontStyle)
{
    font.setBold(fontStyle & FONT_STYLE_BOLD);
    font.setItalic(fontStyle & FONT_STYLE_ITALIC);
    font.setUnderline(fontStyle & FONT_STYLE_UNDERLINE);
    return font;
}

void Editor::refreshAppearance()
{
    // Lexer part
    QsciLexer *oldLexer = m_scintilla->lexer();

    QsciLexer *currentLexer = LexerFactory::CreateLexerForId(m_scintilla, getLanguage()->id);
    m_scintilla->setLexer(currentLexer);

    if (oldLexer)
    {
        delete oldLexer;
    }

    // We expect that lexer will always be available
    // If there is no language-specific lexer, default NullLexer will be used
    assert(currentLexer);

    // Font part
    QString fontFamily = m_currentFontFamily;
    int fontSizePt = m_currentFontSizePt;
    double lineHeightEm = m_currentLineHeightEm;

    if (fontFamily.isEmpty())
    {
        fontFamily = defaultFontFamily;
    }

    if (fontSizePt == 0)
    {
        fontSizePt = defaultFontSizePt;
    }

    if (lineHeightEm == 0.0)
    {
        lineHeightEm = defaultLineHeightEm;
    }

    QFont baseFont(fontFamily, fontSizePt);

    if (fontFamily == defaultFontFamily)
    {
        baseFont.setStyleHint(QFont::TypeWriter);
    }

    double descentEm = lineHeightEm - 1.0;
    int descentPx = PtToPx(round(fontSizePt * descentEm));

    // Style part
    Theme theme = m_currentTheme;

    if (theme.name == "default")
    {
        theme.path = Notepadng::appDataPath() + "themes/Default.xml";
    }

    ScintillaTheme scintillaTheme;

    if (scintillaTheme.LoadFromFile(theme.path) < 0)
    {
        fprintf(stderr, "Failed to load theme %s\n", theme.name.toUtf8().data());
        return;
    }

    // Reset everything to default in Scintilla (lexer itself is already in default state)
    m_scintilla->setColor(QColor("black"));
    m_scintilla->setPaper(QColor("white"));
    m_scintilla->setCaretLineVisible(false);
    m_scintilla->setCaretForegroundColor(QColor("black"));
    m_scintilla->setCaretLineBackgroundColor(QColor("white"));
    m_scintilla->resetFoldMarginColors();
    m_scintilla->resetSelectionForegroundColor();
    m_scintilla->resetSelectionBackgroundColor();

    // Set common scintilla settings
    m_scintilla->setFont(baseFont);
    m_scintilla->setMarginsFont(baseFont);

    currentLexer->setDefaultFont(baseFont);
    currentLexer->setFont(baseFont, -1);

    m_scintilla->setExtraDescent(descentPx);

    // Initialize everything initially with default style
    // If we have custom global styles for individual elements or lexer styles for current language,
    // we will re-style everything at the next stages
    if (scintillaTheme.defaultStyles.contains(globalStyleDefault))
    {
        const Style &style = scintillaTheme.defaultStyles[globalStyleDefault];

        QFont font = StyleFont(baseFont, style.fontStyle);

        m_scintilla->setColor(style.fgColor);
        m_scintilla->setPaper(style.bgColor);
        m_scintilla->setFont(font);

        m_scintilla->setCaretForegroundColor(style.fgColor);
        m_scintilla->setCaretLineBackgroundColor(style.bgColor);
        m_scintilla->setMarginsFont(font);

        // FIXME: Fold markers are colored wiht default style. How to set their color?

        currentLexer->setDefaultColor(style.fgColor);
        currentLexer->setDefaultPaper(style.bgColor);
        currentLexer->setDefaultFont(font);

        currentLexer->setColor(style.fgColor, -1);
        currentLexer->setPaper(style.bgColor, -1);
        currentLexer->setFont(font, -1);
    }

    // Style individual interface elements with global styles
    for (const Style &style : scintillaTheme.defaultStyles)
    {
        if (style.name == globalStyleCaretColour)
        {
            m_scintilla->setCaretForegroundColor(style.fgColor);
        }
        else if (style.name == globalStyleCurrentLineBackgroundColour)
        {
            m_scintilla->setCaretLineBackgroundColor(style.bgColor);
            m_scintilla->setCaretLineVisible(true);
        }
        else if (style.name == globalStyleLineNumberMargin)
        {
            m_scintilla->setMarginsForegroundColor(style.fgColor);
            m_scintilla->setMarginsBackgroundColor(style.bgColor); // FIXME: Why it does not work with lineNumbersMarginIdx?
        }
        else if (style.name == globalStyleFoldMargin)
        {
            m_scintilla->setFoldMarginColors(style.fgColor, style.bgColor);
        }
        else if (style.name == globalStyleBraceHightlightStyle)
        {
            m_scintilla->setMatchedBraceForegroundColor(style.fgColor);
            m_scintilla->setMatchedBraceBackgroundColor(style.bgColor);
        }
        else if (style.name == globalStyleBadBraceColour)
        {
            m_scintilla->setUnmatchedBraceForegroundColor(style.fgColor);
            m_scintilla->setUnmatchedBraceBackgroundColor(style.bgColor);
        }
        else if (style.name == globalStyleSelectedTextColour)
        {
            m_scintilla->setSelectionBackgroundColor(style.bgColor);
        }
        else if (style.name == globalStyleIndentGuidelineStyle)
        {
            m_scintilla->setIndentationGuidesForegroundColor(style.fgColor);
            m_scintilla->setIndentationGuidesBackgroundColor(style.bgColor);
        }
    }

    // Style language tokens with lexer styles if they present
    const QString languageId = getLanguage()->id;
    const QString nppLanguageName = MapIdToNppLanguageName(languageId);

    if (scintillaTheme.lexerStyles.contains(nppLanguageName))
    {
        for (const Style &style : scintillaTheme.lexerStyles[nppLanguageName])
        {
            QFont font = StyleFont(baseFont, style.fontStyle);

            currentLexer->setColor(style.fgColor, style.styleId);
            currentLexer->setPaper(style.bgColor, style.styleId);
            currentLexer->setFont(font, style.styleId);
        }
    }

    // Margins part
    refreshMargins();
}

} //namespace EditorNS
