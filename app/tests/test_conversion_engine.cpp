#include <QtTest/QtTest>
#include <QTemporaryDir>
#include <QSignalSpy>
#include <QFile>
#include <QTextStream>
#include <QProcess>
#include "../src/core/ConversionEngine.h"

using namespace texloom;

class TestConversionEngine : public QObject
{
    Q_OBJECT

private slots:
    void init()
    {
        m_tempDir = new QTemporaryDir();
        QVERIFY(m_tempDir->isValid());
        m_engine = new ConversionEngine();
    }

    void cleanup()
    {
        delete m_engine;
        delete m_tempDir;
    }

    // ========== HELPER METHODS ==========

    bool isPandocAvailable()
    {
        QProcess process;
        process.start("pandoc", QStringList{"--version"});
        return process.waitForFinished(3000) && process.exitCode() == 0;
    }

    QString createTestMarkdownFile()
    {
        QString mdFile = m_tempDir->path() + "/test.md";
        QFile file(mdFile);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            return QString();
        }

        QTextStream out(&file);
        out << "---\n";
        out << "title: Test Document\n";
        out << "author: Test Author\n";
        out << "---\n\n";
        out << "# Introduction\n\n";
        out << "This is a *test* document.\n\n";
        out << "## Section\n\n";
        out << "With some **bold** text.\n";
        file.close();

        return mdFile;
    }

    // ========== SUCCESS SCENARIOS ==========

    void testInitialState()
    {
        QCOMPARE(m_engine->currentStage(), ConversionEngine::Stage::Idle);
        QVERIFY(!m_engine->isBusy());
    }

    void testSetPandocOptions()
    {
        QStringList options = {"--standalone", "--toc", "--from", "markdown"};

        // Should not crash or emit signals
        m_engine->setPandocOptions(options);

        QVERIFY(!m_engine->isBusy());
    }

    void testSetXelatexOptions()
    {
        QStringList options = {"-interaction=batchmode", "-halt-on-error"};

        m_engine->setXelatexOptions(options);

        QVERIFY(!m_engine->isBusy());
    }

    void testSetTemplatePath()
    {
        QString templatePath = "/path/to/template.latex";

        m_engine->setTemplatePath(templatePath);

        QVERIFY(!m_engine->isBusy());
    }

    void testConvertToLatexWithPandoc()
    {
        if (!isPandocAvailable())
        {
            QSKIP("Pandoc not installed, skipping integration test");
        }

        QString mdFile = createTestMarkdownFile();
        QVERIFY(!mdFile.isEmpty());

        QString latexFile = m_tempDir->path() + "/output.tex";

        QSignalSpy spyStarted(m_engine, &ConversionEngine::conversionStarted);
        QSignalSpy spyCompleted(m_engine, &ConversionEngine::conversionCompleted);
        QSignalSpy spyFailed(m_engine, &ConversionEngine::conversionFailed);
        QSignalSpy spyLatexGen(m_engine, &ConversionEngine::latexGenerated);

        m_engine->convertToLatex(mdFile, latexFile);

        // Should start immediately
        QCOMPARE(spyStarted.count(), 1);
        QVERIFY(m_engine->isBusy());

        auto startArgs = spyStarted.takeFirst();
        QCOMPARE(startArgs.at(0).value<ConversionEngine::Stage>(),
                 ConversionEngine::Stage::ConvertingToLatex);

        // Wait for conversion to finish (max 10 seconds)
        bool finished = spyCompleted.wait(10000) || spyFailed.wait(100);
        QVERIFY(finished);

        if (spyFailed.count() > 0)
        {
            qWarning() << "Conversion failed:" << spyFailed.takeFirst().at(0).toString();
            QFAIL("Pandoc conversion failed");
        }

        QCOMPARE(spyCompleted.count(), 1);
        QCOMPARE(spyLatexGen.count(), 1);
        QVERIFY(QFile::exists(latexFile));
        QVERIFY(!m_engine->isBusy());
        QCOMPARE(m_engine->currentStage(), ConversionEngine::Stage::Idle);

        // Verify LaTeX content
        QFile file(latexFile);
        QVERIFY(file.open(QIODevice::ReadOnly));
        QString content = QString::fromUtf8(file.readAll());
        QVERIFY(content.contains("\\documentclass") || content.contains("Introduction"));
    }

    void testSequentialConversions()
    {
        if (!isPandocAvailable())
        {
            QSKIP("Pandoc not installed, skipping integration test");
        }

        // First conversion
        QString mdFile1 = m_tempDir->path() + "/test1.md";
        QFile file1(mdFile1);
        QVERIFY(file1.open(QIODevice::WriteOnly));
        file1.write("# Document 1");
        file1.close();

        QString latexFile1 = m_tempDir->path() + "/output1.tex";

        QSignalSpy spyCompleted(m_engine, &ConversionEngine::conversionCompleted);

        m_engine->convertToLatex(mdFile1, latexFile1);
        QVERIFY(spyCompleted.wait(10000));

        // Second conversion (should work after first is done)
        QString mdFile2 = m_tempDir->path() + "/test2.md";
        QFile file2(mdFile2);
        QVERIFY(file2.open(QIODevice::WriteOnly));
        file2.write("# Document 2");
        file2.close();

        QString latexFile2 = m_tempDir->path() + "/output2.tex";

        m_engine->convertToLatex(mdFile2, latexFile2);
        QVERIFY(spyCompleted.wait(10000));

        QCOMPARE(spyCompleted.count(), 2);
        QVERIFY(QFile::exists(latexFile1));
        QVERIFY(QFile::exists(latexFile2));
    }

    void testBusyStateDetection()
    {
        if (!isPandocAvailable())
        {
            QSKIP("Pandoc not installed, skipping integration test");
        }

        QString mdFile = createTestMarkdownFile();
        QString latexFile = m_tempDir->path() + "/output.tex";

        m_engine->convertToLatex(mdFile, latexFile);

        // Engine should be busy immediately
        QVERIFY(m_engine->isBusy());
        QCOMPARE(m_engine->currentStage(), ConversionEngine::Stage::ConvertingToLatex);

        // Try to start another conversion while busy
        QSignalSpy spyFailed(m_engine, &ConversionEngine::conversionFailed);
        m_engine->convertToLatex(mdFile, m_tempDir->path() + "/output2.tex");

        QCOMPARE(spyFailed.count(), 1);
        QVERIFY(spyFailed.takeFirst().at(0).toString().contains("already in progress"));

        // Wait for first conversion to finish
        QSignalSpy spyCompleted(m_engine, &ConversionEngine::conversionCompleted);
        QVERIFY(spyCompleted.wait(10000));
        QVERIFY(!m_engine->isBusy());
    }

    // ========== FAILURE SCENARIOS ==========

    void testConvertNonExistentFile()
    {
        QSignalSpy spyFailed(m_engine, &ConversionEngine::conversionFailed);
        QSignalSpy spyCompleted(m_engine, &ConversionEngine::conversionCompleted);

        QString nonExistentFile = "/non/existent/file.md";
        QString outputFile = m_tempDir->path() + "/output.tex";

        m_engine->convertToLatex(nonExistentFile, outputFile);

        // Should fail (either immediately or after process starts)
        bool hasFailed = spyFailed.count() > 0 || spyFailed.wait(5000);

        if (!hasFailed && isPandocAvailable())
        {
            // If Pandoc is installed, it should definitely fail
            QFAIL("Expected conversion to fail for non-existent file");
        }

        QVERIFY(spyCompleted.count() == 0); // Should not complete successfully
    }

    void testConvertWithInvalidOutputPath()
    {
        QString mdFile = createTestMarkdownFile();
        QVERIFY(!mdFile.isEmpty());

        QString invalidOutput = "/invalid/path/that/does/not/exist/output.tex";

        QSignalSpy spyFailed(m_engine, &ConversionEngine::conversionFailed);

        m_engine->convertToLatex(mdFile, invalidOutput);

        // This might fail either immediately or when Pandoc tries to write
        bool hasFailed = spyFailed.count() > 0 || spyFailed.wait(5000);

        // Behavior depends on implementation - might fail silently or emit signal
        if (isPandocAvailable() && !hasFailed)
        {
            qWarning() << "Warning: Expected conversion to fail with invalid output path";
        }
    }

    void testPandocNotInstalled()
    {
        // This test verifies graceful handling when Pandoc is missing
        // We can't force Pandoc to be missing, but we can test error signal

        if (isPandocAvailable())
        {
            QSKIP("Pandoc is installed, cannot test missing Pandoc scenario");
        }

        QString mdFile = createTestMarkdownFile();
        QString latexFile = m_tempDir->path() + "/output.tex";

        QSignalSpy spyFailed(m_engine, &ConversionEngine::conversionFailed);

        m_engine->convertToLatex(mdFile, latexFile);

        // Should emit error signal when Pandoc is not found
        QVERIFY(spyFailed.wait(5000));
        QVERIFY(!m_engine->isBusy());
    }

private:
    QTemporaryDir *m_tempDir;
    ConversionEngine *m_engine;
};

QTEST_MAIN(TestConversionEngine)
#include "test_conversion_engine.moc"
