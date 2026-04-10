#pragma once

#include <QPlainTextEdit>

namespace texloom
{

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

        // File operations
        bool loadFile(const QString &filePath);
        bool saveFile(const QString &filePath);
        bool saveFile(); // Save to current file

        // File properties
        QString currentFile() const { return m_currentFile; }
        bool isModified() const { return document()->isModified(); }

        // Editor mode
        Mode editorMode() const { return m_mode; }
        void setEditorMode(Mode mode);

    signals:
        void fileModified(bool modified);
        void modeChanged(Mode mode);

    private:
        QString m_currentFile;
        Mode m_mode = Mode::Code;
    };

} // namespace texloom
