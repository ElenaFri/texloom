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
        const QColor headingBlue("#1f5eb6");
        const QColor bodyText("#2f3641");

        m_heading1Format.setForeground(headingBlue);
        m_heading1Format.setFontWeight(QFont::Bold);

        m_heading2Format.setForeground(headingBlue);
        m_heading2Format.setFontWeight(QFont::Bold);

        m_heading3Format.setForeground(headingBlue);
        m_heading3Format.setFontWeight(QFont::DemiBold);

        m_boldFormat.setForeground(bodyText);
        m_boldFormat.setFontWeight(QFont::Bold);

        m_italicFormat.setForeground(bodyText);
        m_italicFormat.setFontItalic(true);

        m_inlineCodeFormat.setForeground(QColor("#2b3340"));
        m_inlineCodeFormat.setBackground(QColor("#f1f4f8"));
        m_inlineCodeFormat.setFontFamilies({QStringLiteral("monospace")});

        m_linkFormat.setForeground(QColor("#1f5eb6"));
        m_linkFormat.setFontUnderline(true);

        m_codeBlockFormat.setForeground(QColor("#2b3340"));
        m_codeBlockFormat.setBackground(QColor("#f7f9fc"));
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