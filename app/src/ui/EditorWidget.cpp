#include "EditorWidget.h"
#include "MarkdownHighlighter.h"

#include <QEvent>
#include <QFile>
#include <QTextStream>
#include <QFileInfo>
#include <QFrame>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QResizeEvent>
#include <QTextDocument>
#include <QToolButton>

namespace texloom
{

    EditorWidget::EditorWidget(QWidget *parent)
        : QPlainTextEdit(parent)
    {
        setLineWrapMode(QPlainTextEdit::WidgetWidth);
        m_highlighter = new MarkdownHighlighter(document());

        connect(document(), &QTextDocument::modificationChanged,
                this, &EditorWidget::fileModified);

        // Find bar — floats over the bottom of the viewport
        m_findBar = new QFrame(this);
        m_findBar->setFrameShape(QFrame::StyledPanel);
        m_findBar->setFixedHeight(36);

        auto *findLayout = new QHBoxLayout(m_findBar);
        findLayout->setContentsMargins(6, 0, 6, 0);
        findLayout->setSpacing(6);

        auto *closeBtn = new QToolButton(m_findBar);
        closeBtn->setText(QStringLiteral("×"));
        connect(closeBtn, &QToolButton::clicked, this, &EditorWidget::closeFindBar);
        findLayout->addWidget(closeBtn);

        findLayout->addWidget(new QLabel(tr("Find:"), m_findBar));

        m_findEdit = new QLineEdit(m_findBar);
        m_findEdit->setPlaceholderText(tr("Search…"));
        m_findEdit->installEventFilter(this);
        connect(m_findEdit, &QLineEdit::textChanged, this, &EditorWidget::onFindTextChanged);
        findLayout->addWidget(m_findEdit, 1);

        auto *prevBtn = new QToolButton(m_findBar);
        prevBtn->setText(QStringLiteral("▲"));
        prevBtn->setToolTip(tr("Previous (Shift+Enter)"));
        connect(prevBtn, &QToolButton::clicked, this, &EditorWidget::findPrevious);
        findLayout->addWidget(prevBtn);

        auto *nextBtn = new QToolButton(m_findBar);
        nextBtn->setText(QStringLiteral("▼"));
        nextBtn->setToolTip(tr("Next (Enter)"));
        connect(nextBtn, &QToolButton::clicked, this, &EditorWidget::findNext);
        findLayout->addWidget(nextBtn);

        m_findResultLabel = new QLabel(m_findBar);
        m_findResultLabel->setMinimumWidth(70);
        findLayout->addWidget(m_findResultLabel);

        m_findBar->hide();
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

    void EditorWidget::refreshSyntaxTheme()
    {
        if (m_highlighter)
        {
            m_highlighter->refreshFromPalette();
        }
    }

    void EditorWidget::changeEvent(QEvent *event)
    {
        QPlainTextEdit::changeEvent(event);

        if (event->type() == QEvent::PaletteChange && m_highlighter)
        {
            refreshSyntaxTheme();
        }
    }

    void EditorWidget::showFindBar()
    {
        updateFindBarGeometry();
        setViewportMargins(0, 0, 0, m_findBar->sizeHint().height());
        m_findBar->show();
        m_findBar->raise();

        QTextCursor cursor = textCursor();
        if (!cursor.hasSelection())
            cursor.select(QTextCursor::WordUnderCursor);
        m_findEdit->setText(cursor.selectedText());
        m_findEdit->selectAll();
        m_findEdit->setFocus();
    }

    void EditorWidget::closeFindBar()
    {
        m_findBar->hide();
        setViewportMargins(0, 0, 0, 0);
        setExtraSelections({});
        setFocus();
    }

    void EditorWidget::updateFindBarGeometry()
    {
        const int barH = m_findBar->sizeHint().height();
        m_findBar->setGeometry(0, height() - barH, width(), barH);
    }

    void EditorWidget::resizeEvent(QResizeEvent *event)
    {
        QPlainTextEdit::resizeEvent(event);
        if (m_findBar->isVisible())
            m_findBar->setGeometry(0, height() - m_findBar->height(), width(), m_findBar->height());
    }

    void EditorWidget::keyPressEvent(QKeyEvent *event)
    {
        if (event->key() == Qt::Key_Escape && m_findBar->isVisible())
        {
            closeFindBar();
            return;
        }
        QPlainTextEdit::keyPressEvent(event);
    }

    bool EditorWidget::eventFilter(QObject *obj, QEvent *event)
    {
        if (obj == m_findEdit && event->type() == QEvent::KeyPress)
        {
            const auto *key = static_cast<const QKeyEvent *>(event);
            if (key->key() == Qt::Key_Escape)
            {
                closeFindBar();
                return true;
            }
            if (key->key() == Qt::Key_Return || key->key() == Qt::Key_Enter)
            {
                (key->modifiers() & Qt::ShiftModifier) ? findPrevious() : findNext();
                return true;
            }
        }
        return QPlainTextEdit::eventFilter(obj, event);
    }

    void EditorWidget::onFindTextChanged(const QString &text)
    {
        setExtraSelections({});
        if (text.isEmpty())
        {
            m_findResultLabel->clear();
            m_findEdit->setStyleSheet(QString());
            return;
        }

        QList<QTextEdit::ExtraSelection> selections;
        QTextCharFormat fmt;
        fmt.setBackground(QColor(255, 220, 0, 150));

        int count = 0;
        QTextCursor c(document());
        while (!(c = document()->find(text, c)).isNull())
        {
            ++count;
            QTextEdit::ExtraSelection sel;
            sel.format = fmt;
            sel.cursor = c;
            selections.append(sel);
        }
        setExtraSelections(selections);

        if (count == 0)
        {
            m_findResultLabel->setText(tr("No results"));
            m_findEdit->setStyleSheet(QStringLiteral("background: #ffbdbd;"));
        }
        else
        {
            m_findResultLabel->setText(tr("%1 result%2").arg(count).arg(count > 1 ? "s" : ""));
            m_findEdit->setStyleSheet(QString());
            QTextCursor start = textCursor();
            start.movePosition(QTextCursor::Start);
            setTextCursor(start);
            find(text);
        }
    }

    void EditorWidget::findNext()
    {
        const QString text = m_findEdit->text();
        if (text.isEmpty())
            return;
        if (!find(text))
        {
            QTextCursor c = textCursor();
            c.movePosition(QTextCursor::Start);
            setTextCursor(c);
            find(text);
        }
    }

    void EditorWidget::findPrevious()
    {
        const QString text = m_findEdit->text();
        if (text.isEmpty())
            return;
        if (!find(text, QTextDocument::FindBackward))
        {
            QTextCursor c = textCursor();
            c.movePosition(QTextCursor::End);
            setTextCursor(c);
            find(text, QTextDocument::FindBackward);
        }
    }

} // namespace texloom
