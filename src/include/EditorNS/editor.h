#ifndef EDITOR_H
#define EDITOR_H

#include "EditorNS/customscintilla.h"
#include "EditorNS/languageservice.h"

#include "Search/searchhelpers.h"

#include <QObject>
#include <QPrinter>
#include <QQueue>
#include <QTextCodec>
#include <QVBoxLayout>
#include <QVariant>
#include <QWheelEvent>

class EditorTabWidget;

namespace EditorNS
{

/**
 * @brief Provides a QScintilla instance.
 */
class Editor : public QWidget
{
    Q_OBJECT
public:
    struct Theme
    {
        QString name;
        QString path;
        Theme(const QString &name = "default", const QString &path = "")
        {
            this->name = name;
            this->path = path;
        }
    };

    explicit Editor(const Theme &theme, QWidget *parent = nullptr);
    explicit Editor(QWidget *parent = nullptr);

        /**
     * @brief Efficiently returns a new Editor object from an internal buffer.
     * @return
     */
    static QSharedPointer<Editor> getNewEditor(QWidget *parent = nullptr);

    static void invalidateEditorBuffer();

    struct Cursor
    {
        int line;
        int column;

        bool operator==(const Cursor &x) const
        {
            return line == x.line && column == x.column;
        }

        bool operator<(const Cursor &x) const
        {
            return std::tie(line, column) < std::tie(x.line, x.column);
        }

        bool operator<=(const Cursor &x) const
        {
            return *this == x || *this < x;
        }

        bool operator>(const Cursor &x) const
        {
            return !(*this <= x);
        }

        bool operator>=(const Cursor &x) const
        {
            return !(*this < x);
        }
    };

    struct Selection
    {
        Cursor from;
        Cursor to;
    };

    struct IndentationMode
    {
        bool useTabs;
        int size;
    };

    struct CursorInfo
    {
        Cursor cursor;

        int selectedChars = 0;
        int selectedLines = 0;

        int totalChars = 0;
        int totalLines = 0;
    };

        /**
     * @brief Just a flag that is used for marking editors that are still loading,
     * meaning for example that the Editor has been created but we still need
     * to load the file contents or setup the syntax highlighting.
     */
    bool isLoading = false;

        /**
     * @brief Adds a new Editor to the internal buffer used by getNewEditor().
     *        You might want to call this method e.g. as soon as the application
     *        starts (so that an Editor is ready as soon as it gets required),
     *        or when the application is idle.
     * @param howMany specifies how many Editors to add
     * @return
     */
    static void addEditorToBuffer(const int howMany = 1);

    void insertContextMenuAction(QAction *before, QAction *action);

        /**
     * @brief Give focus to the editor, so that the user can start
     *        typing. Note that calling won't automatically switch to
     *        the tab where the editor is. Use EditorTabWidget::setCurrentIndex()
     *        and TopEditorContainer::setFocus() for that.
     */
    Q_INVOKABLE void setFocus();

        /**
     * @brief Remove the focus from the editor.
     *
     * @param widgetOnly only clear the focus on the actual widget
     */
    Q_INVOKABLE void clearFocus();

        /**
     * @brief Set the file name associated with this editor
     * @param filename full path of the file
     */
    Q_INVOKABLE void setFilePath(const QUrl &filename);

        /**
     * @brief Get the file name associated with this editor
     * @return
     */
    Q_INVOKABLE QUrl filePath() const;

    Q_INVOKABLE bool fileOnDiskChanged() const;
    Q_INVOKABLE void setFileOnDiskChanged(bool fileOnDiskChanged);

    enum class SelectMode
    {
        Before,
        After,
        Selected
    };

    void insertBanner(QWidget *banner);
    void removeBanner(QWidget *banner);
    void removeBanner(QString objectName);

    Q_INVOKABLE bool isClean();
    Q_INVOKABLE void markClean();
    Q_INVOKABLE void markDirty();

        /**
     * @brief Returns an integer that denotes the editor's history state. Making changes to
     *        the contents increments the integer while reverting changes decrements it again.
     */
    Q_INVOKABLE int getHistoryGeneration();

        /**
     * @brief Set the language to use for the editor.
     *        It automatically adjusts tab settings from
     *        the default configuration for the specified language.
     * @param language Language id
     */
    Q_INVOKABLE void setLanguage(const Language *language);
    Q_INVOKABLE void setLanguage(const QString &language);
    Q_INVOKABLE void setLanguageFromFilePath(const QString &filePath);
    Q_INVOKABLE void setLanguageFromFilePath();
    Q_INVOKABLE void setValue(const QString &value);
    Q_INVOKABLE QString value();

        /**
     * @brief Set custom indentation settings which may be different
     *        from the default tab settings associated with the current
     *        language.
     *        If this method is called, further calls to setLanguage()
     *        will NOT modify these tab settings. Use
     *        clearCustomIndentationMode() to reset to default settings.
     * @param useTabs
     * @param size Size of an indentation. If 0, keeps the current one.
     */
    void setCustomIndentationMode(const bool useTabs, const int size);
    void setCustomIndentationMode(const bool useTabs);
    void clearCustomIndentationMode();
    bool isUsingCustomIndentationMode() const;

    Q_INVOKABLE void setSmartIndent(bool enabled);
    Q_INVOKABLE int zoomFactor() const;
    Q_INVOKABLE void setZoomFactor(int factor);
    Q_INVOKABLE void setSelectionsText(const QStringList &texts, SelectMode mode);
    Q_INVOKABLE void setSelectionsText(const QStringList &texts);
    const Language *getLanguage() { return m_currentLanguage; }
    Q_INVOKABLE void setLineWrap(const bool wrap);
    Q_INVOKABLE void setEOLVisible(const bool showeol);
    Q_INVOKABLE void setWhitespaceVisible(const bool showspace);
    void setIndentGuideVisible(bool showindentguide);

        /**
     * @brief Get the current cursor position
     * @return a <line, column> pair.
     */
    QPair<int, int> cursorPosition();
    void setCursorPosition(const int line, const int column);
    void setCursorPosition(const QPair<int, int> &position);
    void setCursorPosition(const Cursor &cursor);

        /**
     * @brief Tells the editor that mainwindow needs an update on the contents,
     *        selection, and cursor position of the current document
     */
    void requestDocumentInfo();

        /**
     * @brief Get the current scroll position
     * @return a <left, top> pair.
     */
    QPair<int, int> scrollPosition();
    void setScrollPosition(const int left, const int top);
    void setScrollPosition(const QPair<int, int> &position);
    QString endOfLineSequence() const;
    void setEndOfLineSequence(const QString &endOfLineSequence);

        /**
     * @brief Applies a font family/size to the Editor.
     * @param fontFamily the family to be applied. An empty string or
     *                   nullptr denote no override.
     * @param fontSize the size to be applied. 0 denotes no override.
     */
    void setFont(QString fontFamily, int fontSize, double lineHeight);

        /**
     * @brief Toggles line numbers on/off in the editor
     * @param visible when true, the line numbers will be visible,
     * when false the line numbers will be hidden.
     */
    void setLineNumbersVisible(bool visible);

    QTextCodec *codec() const;

        /**
     * @brief Set the codec for this Editor.
     *        This method does not change the in-memory or on-screen
     *        representation of the document (which is always Unicode).
     *        It serves solely as a way to keep track of the encoding
     *        that needs to be used when the document gets saved.
     * @param codec
     */
    void setCodec(QTextCodec *codec);

    bool bom() const;
    void setBom(bool bom);

    QList<Theme> themes();
    void setTheme(Theme theme);
    static Editor::Theme themeFromName(QString name);

    QList<Selection> selections();

        /**
     * @brief Returns the currently selected texts.
     * @return
     */
    Q_INVOKABLE QStringList selectedTexts();

    void setOverwrite(bool overwrite);

    Editor::IndentationMode indentationMode();

    void setSelection(int fromLine, int fromCol, int toLine, int toCol);

    int lineCount();

    void selectAll();

    void undo();
    void redo();

    void deleteCurrentLine();
    void duplicateCurrentLine();
    void moveCurrentLineUp();
    void moveCurrentLineDown();

    void trimTrailingWhitespaces();
    void trimLeadingWhitespaces();
    void trimLeadingAndTrailingWhitespaces();

    void convertEolToSpace();
    void convertTabsToSpaces();
    void convertAllSpacesToTabs();
    void convertLeadingSpacesToTabs();

    void search(const QString &string, SearchHelpers::SearchMode searchMode, bool forward, const SearchHelpers::SearchOptions &searchOptions);
    void replace(const QString &string, SearchHelpers::SearchMode searchMode, bool forward, const SearchHelpers::SearchOptions &searchOptions, const QString &replacement);
    int replaceAll(const QString &string, SearchHelpers::SearchMode searchMode, const SearchHelpers::SearchOptions &searchOptions, const QString &replacement);

private:
    friend class ::EditorTabWidget;

        // These functions should only be used by EditorTabWidget to manage the tab's title. This works around
        // KDE's habit to automatically modify QTabWidget's tab titles to insert shortcut sequences (like &1).
    QString tabName() const;
    void setTabName(const QString &name);

    static QQueue<QSharedPointer<Editor>> m_editorBuffer;
    QVBoxLayout *m_layout;
    CustomScintilla *m_scintilla;
    QUrl m_filePath = QUrl();
    QString m_tabName;
    bool m_fileOnDiskChanged = false;
    QTextCodec *m_codec = QTextCodec::codecForName("UTF-8");
    bool m_bom = false;
    bool m_customIndentationMode = false;
    const Language *m_currentLanguage = nullptr;

    bool m_lineNumbersVisible = false;

    void refreshMargins();

    QString m_currentFontFamily;
    int m_currentFontSizePt = 0;
    double m_currentLineHeightEm = 0.0;

    Theme m_currentTheme;

    void refreshAppearance();

    bool m_forceModified = false;

    void fullConstructor(const Theme &theme);

    void setIndentationMode(const bool useTabs, const int size);
    void setIndentationMode(const Language *);

    bool searchAndSelect(bool inSelection, const QString &string, SearchHelpers::SearchMode searchMode, bool forward, const SearchHelpers::SearchOptions &searchOptions, bool wrap);
    int replaceAllNoCheckpoint(const QString &string, SearchHelpers::SearchMode searchMode, const SearchHelpers::SearchOptions &searchOptions, const QString &replacement);

    void incrementGeneration();
    int m_generation = 0;

signals:
    void gotFocus();
    void urlsDropped(QList<QUrl> urls);
    void bannerRemoved(QWidget *banner);

        // Pre-interpreted messages:
    void contentChanged();
    void cursorActivity(CursorInfo cursorInfo);
    void documentInfoRequested(CursorInfo cursorInfo);
    void cleanChanged(bool isClean);
    void fileNameChanged(const QUrl &oldFileName, const QUrl &newFileName);
    void zoomChanged(int zoomFactor);

    void currentLanguageChanged(QString id, QString name);

public slots:
        /**
     * @brief Print the editor. As of Qt 5.11, it produces low-quality, non-vector graphics with big dimension.
     * @param printer
     */
    void print(std::shared_ptr<QPrinter> printer);

        /**
     * @brief Returns the content of the editor layed out in a pdf file that can be directly saved to disk.
     *        This method produces light, vector graphics.
     * @param pageLayout
     * @return
     */
    QByteArray printToPdf(const QPageLayout &pageLayout = QPageLayout(QPageSize(QPageSize::A4), QPageLayout::Portrait, QMarginsF()));
};

} //namespace EditorNS

#endif // EDITOR_H
