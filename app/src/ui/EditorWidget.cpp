#include "EditorWidget.h"
#include "MarkdownHighlighter.h"

#include <QFile>
#include <QTextStream>
#include <QFileInfo>

namespace texloom
{

    EditorWidget::EditorWidget(QWidget *parent)
        : QPlainTextEdit(parent)
    {
        // Basic editor setup
        setLineWrapMode(QPlainTextEdit::WidgetWidth);
        m_highlighter = new MarkdownHighlighter(document());

        // Connect modification signal
        connect(document(), &QTextDocument::modificationChanged,
                this, &EditorWidget::fileModified);
    }

    bool EditorWidget::loadFile(const QString &filePath)
    {
        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            return false;
        }

        QTextStream in(&file);
        setPlainText(in.readAll());
        file.close();

        m_currentFile = filePath;
        document()->setModified(false);

        return true;
    }

    bool EditorWidget::saveFile(const QString &filePath)
    {
        QFile file(filePath);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            return false;
        }

        QTextStream out(&file);
        out << toPlainText();
        file.close();

        m_currentFile = filePath;
        document()->setModified(false);

        return true;
    }

    bool EditorWidget::saveFile()
    {
        if (m_currentFile.isEmpty())
        {
            return false;
        }

        return saveFile(m_currentFile);
    }

    void EditorWidget::setEditorMode(Mode mode)
    {
        if (m_mode != mode)
        {
            m_mode = mode;
            emit modeChanged(mode);

            // TODO: Implement WYSIWYG mode
            // For now, only Code mode is supported
        }
    }

} // namespace texloom
