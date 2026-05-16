#pragma once

#include <QPlainTextEdit>

namespace texloom
{

    class MarkdownHighlighter;

    /**
     * @brief Markdown editor widget
     *
     * Text editor with Markdown syntax highlighting and editing features.
     * Supports both Code mode (raw Markdown) and WYSIWYG mode (future).
     */
    class EditorWidget : public QPlainTextEdit
    {
        Q_OBJECT

    public:
        enum class Mode
        {
            Code,
            Wysiwyg
        };

        explicit EditorWidget(QWidget *parent = nullptr);
        ~EditorWidget() override = default;

        // Non-copyable and non-movable (QWidget already is)
        EditorWidget(const EditorWidget &) = delete;
        EditorWidget &operator=(const EditorWidget &) = delete;
        EditorWidget(EditorWidget &&) = delete;
        EditorWidget &operator=(EditorWidget &&) = delete;

        // File operations
        bool loadFile(const QString &filePath);
        bool saveFile(const QString &filePath);
        bool saveFile(); // Save to current file

        // File properties
        [[nodiscard]] QString currentFile() const noexcept { return m_currentFile; }
        [[nodiscard]] bool isModified() const { return document()->isModified(); }

        // Editor mode
        [[nodiscard]] Mode editorMode() const noexcept { return m_mode; }
        void setEditorMode(Mode mode);

    signals:
        void fileModified(bool modified);
        void modeChanged(Mode mode);

    private:
        QString m_currentFile;
        MarkdownHighlighter *m_highlighter = nullptr;
        Mode m_mode = Mode::Code;
    };

} // namespace texloom
