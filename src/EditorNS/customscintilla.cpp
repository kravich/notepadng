#include "EditorNS/customscintilla.h"

#include <QMimeData>
#include <QMenu>

namespace EditorNS
{

CustomScintilla::CustomScintilla(QWidget *parent) :
    QsciScintilla(parent)
{
    // Connect handlers for notifications
    connect(this, &CustomScintilla::SCN_ZOOM, this, [this]() { emit zoomChanged(getZoom()); });

    // Always use some monospace font by default (if not overriden by lexer)
    QFont font("Monospace");
    font.setStyleHint(QFont::TypeWriter);
    setFont(font);
}

/* Information */

int CustomScintilla::positionFromPoint(const QPoint &pos)
{
    return SendScintilla(SCI_POSITIONFROMPOINT, pos.x(), pos.y());
}

int CustomScintilla::pointXFromPosition(int position)
{
    return SendScintilla(SCI_POINTXFROMPOSITION, 0, position);
}

/* Scrolling and automatic scrolling */

int CustomScintilla::getXOffset()
{
    return SendScintilla(SCI_GETXOFFSET);
}

void CustomScintilla::setXOffset(int xOffset)
{
    SendScintilla(SCI_SETXOFFSET, xOffset);
}

int CustomScintilla::pointYFromPosition(int position)
{
    return SendScintilla(SCI_POINTYFROMPOSITION, 0, position);
}

/* Keyboard commands */

void CustomScintilla::lineDelete()
{
    SendScintilla(SCI_LINEDELETE);
}

void CustomScintilla::lineDuplicate()
{
    SendScintilla(SCI_LINEDUPLICATE);
}

void CustomScintilla::moveSelectedLinesUp()
{
    SendScintilla(SCI_MOVESELECTEDLINESUP);
}

void CustomScintilla::moveSelectedLinesDown()
{
    SendScintilla(SCI_MOVESELECTEDLINESDOWN);
}

/* Zooming */

int CustomScintilla::getZoom()
{
    return SendScintilla(SCI_GETZOOM);
}

void CustomScintilla::keyPressEvent(QKeyEvent *ev)
{
    switch (ev->key())
    {
    case Qt::Key_Insert:
        ev->ignore();
        break;
    default:
        QsciScintilla::keyPressEvent(ev);
    }
}

void CustomScintilla::dropEvent(QDropEvent *ev)
{
    if (ev->mimeData()->hasUrls())
    {
        ev->ignore();
        emit urlsDropped(ev->mimeData()->urls());
    }
    else
    {
        QsciScintilla::dropEvent(ev);
    }
}

void CustomScintilla::focusInEvent(QFocusEvent *event)
{
    QsciScintilla::focusInEvent(event);
    emit gotFocus();
}

void CustomScintilla::contextMenuEvent(QContextMenuEvent *event)
{
    updateCursorPositionPerContextMenuActivation(event->pos());

    QMenu *menu = new QMenu(this);

    QList<QAction*> contextActions = actions();

    if (contextActions.size() > 0)
    {
        for (QAction *action : contextActions)
        {
            menu->insertAction(nullptr, action);
        }
    }
    else
    {
        QAction *placeholderAction = new QAction(this);
        placeholderAction->setText("Placeholder");
        placeholderAction->setEnabled(false);
        menu->insertAction(nullptr, placeholderAction);
    }

    menu->popup(event->globalPos());
}

void CustomScintilla::updateCursorPositionPerContextMenuActivation(const QPoint &cursorPos)
{
    int position = positionFromPoint(cursorPos);
    bool positionIsWithinSelection = isPositionWithinSelection(position);

    // Save selection
    int selectedLineFrom = 0;
    int selectedIndexFrom = 0;
    int selectedLineTo = 0;
    int selectedIndexTo = 0;

    getSelection(&selectedLineFrom, &selectedIndexFrom, &selectedLineTo, &selectedIndexTo);

    int line = 0;
    int index = 0;

    lineIndexFromPosition(position, &line, &index);
    setCursorPosition(line, index);

    if (positionIsWithinSelection)
    {
        // Restore selection
        setSelection(selectedLineFrom, selectedIndexFrom, selectedLineTo, selectedIndexTo);
    }
}

bool CustomScintilla::isPositionWithinSelection(int position)
{
    if (!hasSelectedText())
    {
        return false;
    }

    // Current selection
    int selectedLineFrom = 0;
    int selectedIndexFrom = 0;
    int selectedLineTo = 0;
    int selectedIndexTo = 0;

    getSelection(&selectedLineFrom, &selectedIndexFrom, &selectedLineTo, &selectedIndexTo);

    // Get coordinates for specified position
    int line = -1;
    int index = -1;

    lineIndexFromPosition(position, &line, &index);

    if (selectedLineFrom == selectedLineTo) // Single-line selection
    {
        if (selectedLineFrom == line)
        {
            if (selectedIndexFrom <= index && index <= selectedIndexTo)
            {
                return true;
            }
            else
            {
                return false;
            }
        }
        else // selectedLineFrom != line
        {
            return false;
        }
    }
    else // Multi-line selection
    {
        if (line < selectedLineFrom)
        {
            return false;
        }
        else if (line == selectedLineFrom)
        {
            if (index < selectedIndexFrom)
            {
                return false;
            }
            else // selectedIndexFrom <= index
            {
                return true;
            }
        }
        else if (selectedLineFrom < line && line < selectedLineTo)
        {
            return true;
        }
        else if (line == selectedLineTo)
        {
            if (index <= selectedIndexTo)
            {
                return true;
            }
            else // selectedIndexTo < index
            {
                return false;
            }
        }
        else // selectedLineTo < line
        {
            return false;
        }
    }
}

} // namespace EditorNS
