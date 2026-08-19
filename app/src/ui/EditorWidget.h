#pragma once

#include <QPlainTextEdit>

QT_BEGIN_NAMESPACE
class QFrame;
class QLabel;
class QLineEdit;
QT_END_NAMESPACE

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
        void refreshSyntaxTheme();

        void showFindBar();

        bool eventFilter(QObject *obj, QEvent *event) override;

    protected:
        void changeEvent(QEvent *event) override;
        void resizeEvent(QResizeEvent *event) override;
        void keyPressEvent(QKeyEvent *event) override;

    signals:
        void fileModified(bool modified);
        void modeChanged(Mode mode);

    private slots:
        void closeFindBar();
        void findNext();
        void findPrevious();
        void onFindTextChanged(const QString &text);

    private:
        void updateFindBarGeometry();

        QString m_currentFile;
        MarkdownHighlighter *m_highlighter = nullptr;
        Mode m_mode = Mode::Code;

        QFrame *m_findBar = nullptr;
        QLineEdit *m_findEdit = nullptr;
        QLabel *m_findResultLabel = nullptr;
    };

} // namespace texloom
