#include "MarkdownHighlighter.h"

#include <QBrush>
#include <QColor>
#include <QApplication>
#include <QPalette>
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
        refreshFromPalette();
    }

    void MarkdownHighlighter::refreshFromPalette()
    {
        const QPalette palette = QApplication::palette();
        const QColor bodyText = palette.color(QPalette::Text);
        const QColor accent = palette.color(QPalette::Highlight);
        const QColor base = palette.color(QPalette::Base);
        const QColor alternate = palette.color(QPalette::AlternateBase);
        const QColor codeBg = alternate.isValid() ? alternate : base.darker(103);
        const QColor inlineCodeBg = base.lighter(106);

        m_heading1Format.setForeground(accent);
        m_heading1Format.setFontWeight(QFont::Bold);

        m_heading2Format.setForeground(accent);
        m_heading2Format.setFontWeight(QFont::Bold);

        m_heading3Format.setForeground(accent);
        m_heading3Format.setFontWeight(QFont::DemiBold);

        m_boldFormat.setForeground(bodyText);
        m_boldFormat.setFontWeight(QFont::Bold);

        // Keep italics clearly visible by tinting with the active accent color.
        m_italicFormat.setForeground(accent);
        m_italicFormat.setFontItalic(true);

        m_inlineCodeFormat.setForeground(bodyText);
        m_inlineCodeFormat.setBackground(inlineCodeBg);
        m_inlineCodeFormat.setFontFamilies({QStringLiteral("monospace")});

        m_linkFormat.setForeground(accent);
        m_linkFormat.setFontUnderline(true);

        m_codeBlockFormat.setForeground(bodyText);
        m_codeBlockFormat.setBackground(codeBg);
        m_codeBlockFormat.setFontFamilies({QStringLiteral("monospace")});

        rehighlight();
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
        static const QRegularExpression italicStarPattern(QStringLiteral(R"((?<!\*)\*[^*\n]+\*(?!\*))"));
        static const QRegularExpression italicUnderscorePattern(QStringLiteral(R"((?<!_)_[^_\n]+_(?!_))"));

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
        applyMatches(italicStarPattern, m_italicFormat);
        applyMatches(italicUnderscorePattern, m_italicFormat);
    }

} // namespace texloom