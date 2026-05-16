#include "MarkdownHighlighter.h"

#include <QBrush>
#include <QColor>
#include <QRegularExpression>
#include <QTextDocument>

namespace texloom
{

    namespace
    {
        constexpr int CodeBlockState = 1;
    }

    MarkdownHighlighter::MarkdownHighlighter(QTextDocument *parent)
        : QSyntaxHighlighter(parent)
    {
        m_heading1Format.setForeground(QColor("#8b1e3f"));
        m_heading1Format.setFontWeight(QFont::Bold);

        m_heading2Format.setForeground(QColor("#a14a1a"));
        m_heading2Format.setFontWeight(QFont::Bold);

        m_heading3Format.setForeground(QColor("#155e63"));
        m_heading3Format.setFontWeight(QFont::DemiBold);

        m_boldFormat.setForeground(QColor("#18206f"));
        m_boldFormat.setFontWeight(QFont::Bold);

        m_italicFormat.setForeground(QColor("#6a1b78"));
        m_italicFormat.setFontItalic(true);

        m_inlineCodeFormat.setForeground(QColor("#234e52"));
        m_inlineCodeFormat.setBackground(QColor("#e6f4f1"));
        m_inlineCodeFormat.setFontFamilies({QStringLiteral("monospace")});

        m_linkFormat.setForeground(QColor("#005a9c"));
        m_linkFormat.setFontUnderline(true);

        m_codeBlockFormat.setForeground(QColor("#1f2933"));
        m_codeBlockFormat.setBackground(QColor("#eef2f6"));
        m_codeBlockFormat.setFontFamilies({QStringLiteral("monospace")});
    }

    void MarkdownHighlighter::highlightBlock(const QString &text)
    {
        const bool wasInCodeBlock = previousBlockState() == CodeBlockState;
        const bool isFenceLine = text.startsWith(QStringLiteral("```"));

        if (wasInCodeBlock)
        {
            setFormat(0, text.length(), m_codeBlockFormat);
            if (isFenceLine)
            {
                setCurrentBlockState(-1);
            }
            else
            {
                setCurrentBlockState(CodeBlockState);
            }
            return;
        }

        setCurrentBlockState(-1);

        if (isFenceLine)
        {
            setFormat(0, text.length(), m_codeBlockFormat);
            setCurrentBlockState(CodeBlockState);
            return;
        }

        if (text.startsWith(QStringLiteral("### ")))
        {
            setFormat(0, text.length(), m_heading3Format);
            return;
        }

        if (text.startsWith(QStringLiteral("## ")))
        {
            setFormat(0, text.length(), m_heading2Format);
            return;
        }

        if (text.startsWith(QStringLiteral("# ")))
        {
            setFormat(0, text.length(), m_heading1Format);
            return;
        }

        highlightInlineElements(text);
    }

    void MarkdownHighlighter::highlightInlineElements(const QString &text)
    {
        static const QRegularExpression linkPattern(QStringLiteral(R"(\[[^\]\n]+\]\([^)\n]+\))"));
        static const QRegularExpression inlineCodePattern(QStringLiteral(R"(`[^`\n]+`)"));
        static const QRegularExpression boldPattern(QStringLiteral(R"(\*\*[^*\n]+\*\*)"));
        static const QRegularExpression italicPattern(QStringLiteral(R"((?<!\*)\*[^*\n]+\*(?!\*))"));

        const auto applyMatches = [this, &text](const QRegularExpression &pattern, const QTextCharFormat &format)
        {
            QRegularExpressionMatchIterator iterator = pattern.globalMatch(text);
            while (iterator.hasNext())
            {
                const QRegularExpressionMatch match = iterator.next();
                if (match.hasMatch())
                {
                    setFormat(match.capturedStart(), match.capturedLength(), format);
                }
            }
        };

        applyMatches(linkPattern, m_linkFormat);
        applyMatches(inlineCodePattern, m_inlineCodeFormat);
        applyMatches(boldPattern, m_boldFormat);
        applyMatches(italicPattern, m_italicFormat);
    }

} // namespace texloom