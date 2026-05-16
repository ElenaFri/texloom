#pragma once

#include <QSyntaxHighlighter>
#include <QTextCharFormat>

namespace texloom
{

    class MarkdownHighlighter : public QSyntaxHighlighter
    {
        Q_OBJECT

    public:
        explicit MarkdownHighlighter(QTextDocument *parent = nullptr);
        ~MarkdownHighlighter() override = default;
        void refreshFromPalette();

        MarkdownHighlighter(const MarkdownHighlighter &) = delete;
        MarkdownHighlighter &operator=(const MarkdownHighlighter &) = delete;
        MarkdownHighlighter(MarkdownHighlighter &&) = delete;
        MarkdownHighlighter &operator=(MarkdownHighlighter &&) = delete;

    protected:
        void highlightBlock(const QString &text) override;

    private:
        void highlightInlineElements(const QString &text);

        QTextCharFormat m_heading1Format;
        QTextCharFormat m_heading2Format;
        QTextCharFormat m_heading3Format;
        QTextCharFormat m_boldFormat;
        QTextCharFormat m_italicFormat;
        QTextCharFormat m_inlineCodeFormat;
        QTextCharFormat m_linkFormat;
        QTextCharFormat m_codeBlockFormat;
    };

} // namespace texloom