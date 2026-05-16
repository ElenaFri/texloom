#include <QtTest/QtTest>
#include <QTemporaryDir>
#include <QSignalSpy>
#include <QFile>
#include <QTextBlock>
#include <QTextLayout>
#include <QTextStream>
#include "../src/ui/EditorWidget.h"

using namespace texloom;

class TestEditorWidget : public QObject
{
    Q_OBJECT

private:
    static bool blockHasFormat(const QTextBlock &block,
                               const std::function<bool(const QTextLayout::FormatRange &)> &predicate)
    {
        const QList<QTextLayout::FormatRange> formats = block.layout()->formats();
        for (const QTextLayout::FormatRange &formatRange : formats)
        {
            if (predicate(formatRange))
            {
                return true;
            }
        }

        return false;
    }

private slots:

    void init()
    {
        m_tempDir = new QTemporaryDir();
        QVERIFY(m_tempDir->isValid());
        m_editor = new EditorWidget();
    }

    void cleanup()
    {
        delete m_editor;
        delete m_tempDir;
    }

    // ========== SUCCESS SCENARIOS ==========

    void testLoadExistingFile()
    {
        // Create a test file
        QString testFile = m_tempDir->path() + "/test.md";
        QFile file(testFile);
        QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));
        QTextStream out(&file);
        out << "# Test Markdown\n\nThis is a test.";
        file.close();

        bool result = m_editor->loadFile(testFile);

        QVERIFY(result);
        QCOMPARE(m_editor->currentFile(), testFile);
        QVERIFY(m_editor->toPlainText().contains("Test Markdown"));
        QVERIFY(!m_editor->isModified());
    }

    void testSaveToNewFile()
    {
        QString newFile = m_tempDir->path() + "/new.md";
        m_editor->setPlainText("# New Content\n\nTest content.");

        bool result = m_editor->saveFile(newFile);

        QVERIFY(result);
        QCOMPARE(m_editor->currentFile(), newFile);
        QVERIFY(QFile::exists(newFile));
        QVERIFY(!m_editor->isModified());

        // Verify content was written
        QFile file(newFile);
        QVERIFY(file.open(QIODevice::ReadOnly));
        QString content = QString::fromUtf8(file.readAll());
        QVERIFY(content.contains("New Content"));
    }

    void testSaveToCurrentFile()
    {
        // Load a file first
        QString testFile = m_tempDir->path() + "/current.md";
        QFile file(testFile);
        QVERIFY(file.open(QIODevice::WriteOnly));
        file.write("Original content");
        file.close();

        m_editor->loadFile(testFile);
        m_editor->setPlainText("Modified content");

        bool result = m_editor->saveFile(); // Save without path

        QVERIFY(result);
        QVERIFY(!m_editor->isModified());

        // Verify file was updated
        file.open(QIODevice::ReadOnly);
        QString content = QString::fromUtf8(file.readAll());
        QCOMPARE(content, QString("Modified content"));
    }

    void testEditorModeSwitch()
    {
        QCOMPARE(m_editor->editorMode(), EditorWidget::Mode::Code);

        QSignalSpy spyModeChanged(m_editor, &EditorWidget::modeChanged);

        m_editor->setEditorMode(EditorWidget::Mode::Wysiwyg);

        QCOMPARE(m_editor->editorMode(), EditorWidget::Mode::Wysiwyg);
        QCOMPARE(spyModeChanged.count(), 1);

        auto args = spyModeChanged.takeFirst();
        QCOMPARE(args.at(0).value<EditorWidget::Mode>(), EditorWidget::Mode::Wysiwyg);

        // Switch back
        m_editor->setEditorMode(EditorWidget::Mode::Code);
        QCOMPARE(spyModeChanged.count(), 1);
    }

    void testFileModificationSignal()
    {
        // Load a file first (Qt doesn't mark empty documents as modified)
        QString testFile = m_tempDir->path() + "/initial.md";
        QFile file(testFile);
        QVERIFY(file.open(QIODevice::WriteOnly));
        file.write("Initial content");
        file.close();

        m_editor->loadFile(testFile);
        QVERIFY(!m_editor->isModified());

        QSignalSpy spyModified(m_editor, &EditorWidget::fileModified);

        // Modification should trigger signal - use append instead of setPlainText
        // because setPlainText doesn't mark document as modified in Qt
        m_editor->appendPlainText("\nAdditional line");

        // Qt may batch signals, wait for events
        QTest::qWait(10);

        QVERIFY(spyModified.count() > 0);
        QVERIFY(m_editor->isModified());

        // Save should clear modified flag
        QString saveFile = m_tempDir->path() + "/mod.md";
        m_editor->saveFile(saveFile);

        QVERIFY(!m_editor->isModified());
    }

    // ========== FAILURE SCENARIOS ==========

    void testLoadNonExistentFile()
    {
        bool result = m_editor->loadFile("/non/existent/file.md");

        QVERIFY(!result);
        QVERIFY(m_editor->currentFile().isEmpty());
    }

    void testSaveWithoutCurrentFile()
    {
        m_editor->setPlainText("Content without file");

        bool result = m_editor->saveFile(); // Should fail: no current file set

        QVERIFY(!result);
    }

    void testSaveToInvalidPath()
    {
        m_editor->setPlainText("Test content");

        bool result = m_editor->saveFile("/invalid/path/that/does/not/exist/file.md");

        QVERIFY(!result);
        // Current file might be set even if save fails - implementation dependent
    }

    // ========== EDGE CASES ==========

    void testEmptyFile()
    {
        // Create empty file
        QString emptyFile = m_tempDir->path() + "/empty.md";
        QFile file(emptyFile);
        QVERIFY(file.open(QIODevice::WriteOnly));
        file.close();

        bool result = m_editor->loadFile(emptyFile);

        QVERIFY(result);
        QCOMPARE(m_editor->toPlainText(), QString(""));
        QVERIFY(!m_editor->isModified());
    }

    void testLargeFile()
    {
        // Create a large file (10 MB)
        QString largeFile = m_tempDir->path() + "/large.md";
        QFile file(largeFile);
        QVERIFY(file.open(QIODevice::WriteOnly));

        QTextStream out(&file);
        for (int i = 0; i < 100000; ++i)
        {
            out << "# Heading " << i << "\n\nSome content here.\n\n";
        }
        file.close();

        // This tests performance and memory handling
        bool result = m_editor->loadFile(largeFile);

        QVERIFY(result);
        QVERIFY(m_editor->toPlainText().length() > 1000000);
    }

    void testUtf8Encoding()
    {
        // Test Unicode characters (French, Greek, emoji)
        QString unicodeContent = "# Français\n\n"
                                 "Voilà du contenu en français avec des accents: é, è, ê, à, ç.\n\n"
                                 "## Ελληνικά\n\n"
                                 "Δοκιμή με ελληνικούς χαρακτήρες: α, β, γ, δ.\n\n"
                                 "## Emoji\n\n"
                                 "Testing emoji: 🚀 💻 📝 ✅\n";

        QString testFile = m_tempDir->path() + "/unicode.md";
        m_editor->setPlainText(unicodeContent);

        bool saveResult = m_editor->saveFile(testFile);
        QVERIFY(saveResult);

        // Load it back
        EditorWidget *editor2 = new EditorWidget();
        bool loadResult = editor2->loadFile(testFile);

        QVERIFY(loadResult);
        QCOMPARE(editor2->toPlainText(), unicodeContent);

        delete editor2;
    }

    void testHeadingHighlighting()
    {
        m_editor->setPlainText("# Heading 1\n## Heading 2\n### Heading 3\n");

        const QTextBlock first = m_editor->document()->findBlockByNumber(0);
        const QTextBlock second = m_editor->document()->findBlockByNumber(1);
        const QTextBlock third = m_editor->document()->findBlockByNumber(2);

        QVERIFY(blockHasFormat(first, [](const QTextLayout::FormatRange &range)
                               { return range.format.fontWeight() == QFont::Bold; }));
        QVERIFY(blockHasFormat(second, [](const QTextLayout::FormatRange &range)
                               { return range.format.fontWeight() == QFont::Bold; }));
        QVERIFY(blockHasFormat(third, [](const QTextLayout::FormatRange &range)
                               { return range.format.fontWeight() == QFont::DemiBold; }));
    }

    void testBoldItalicInlineCodeAndLinkHighlighting()
    {
        m_editor->setPlainText("**bold** *italic* `code` [link](https://example.com)");

        const QTextBlock block = m_editor->document()->firstBlock();

        QVERIFY(blockHasFormat(block, [](const QTextLayout::FormatRange &range)
                               { return range.format.fontWeight() == QFont::Bold; }));
        QVERIFY(blockHasFormat(block, [](const QTextLayout::FormatRange &range)
                               { return range.format.fontItalic(); }));
        QVERIFY(blockHasFormat(block, [](const QTextLayout::FormatRange &range)
                               { return range.format.background().style() != Qt::NoBrush; }));
        QVERIFY(blockHasFormat(block, [](const QTextLayout::FormatRange &range)
                               { return range.format.fontUnderline(); }));
    }

    void testCodeBlockHighlighting()
    {
        m_editor->setPlainText("```cpp\nint main() { return 0; }\n```\nplain text\n");

        const QTextBlock fenceStart = m_editor->document()->findBlockByNumber(0);
        const QTextBlock codeLine = m_editor->document()->findBlockByNumber(1);
        const QTextBlock fenceEnd = m_editor->document()->findBlockByNumber(2);
        const QTextBlock plainLine = m_editor->document()->findBlockByNumber(3);

        QVERIFY(blockHasFormat(fenceStart, [](const QTextLayout::FormatRange &range)
                               { return range.format.background().style() != Qt::NoBrush; }));
        QVERIFY(blockHasFormat(codeLine, [](const QTextLayout::FormatRange &range)
                               { return range.format.background().style() != Qt::NoBrush; }));
        QVERIFY(blockHasFormat(fenceEnd, [](const QTextLayout::FormatRange &range)
                               { return range.format.background().style() != Qt::NoBrush; }));
        QVERIFY(!blockHasFormat(plainLine, [](const QTextLayout::FormatRange &range)
                                { return range.format.background().style() != Qt::NoBrush; }));
    }

private:
    QTemporaryDir *m_tempDir;
    EditorWidget *m_editor;
};

QTEST_MAIN(TestEditorWidget)
#include "test_editor_widget.moc"
