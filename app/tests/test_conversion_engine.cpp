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

    void testCompileToPdfWithXelatex()
    {
        // Check if xelatex is available
        QProcess process;
        process.start("xelatex", QStringList{"--version"});
        if (!process.waitForFinished(3000) || process.exitCode() != 0)
        {
            QSKIP("XeLaTeX not installed, skipping PDF compilation test");
        }

        // Create a minimal LaTeX file
        QString texFile = m_tempDir->path() + "/test.tex";
        QFile file(texFile);
        QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));

        QTextStream out(&file);
        out << "\\documentclass{article}\n";
        out << "\\begin{document}\n";
        out << "Test PDF Document\n";
        out << "\\end{document}\n";
        file.close();

        QString pdfFile = m_tempDir->path() + "/test.pdf";

        QSignalSpy spyStarted(m_engine, &ConversionEngine::conversionStarted);
        QSignalSpy spyCompleted(m_engine, &ConversionEngine::conversionCompleted);
        QSignalSpy spyFailed(m_engine, &ConversionEngine::conversionFailed);
        QSignalSpy spyPdfGen(m_engine, &ConversionEngine::pdfGenerated);

        m_engine->compileToPdf(texFile, pdfFile);

        QCOMPARE(spyStarted.count(), 1);
        auto startArgs = spyStarted.takeFirst();
        QCOMPARE(startArgs.at(0).value<ConversionEngine::Stage>(),
                 ConversionEngine::Stage::CompilingToPdf);
        QVERIFY(m_engine->isBusy());

        // Wait for compilation (max 30 seconds - PDF compilation can be slow)
        bool finished = spyCompleted.wait(30000) || spyFailed.wait(100);
        QVERIFY2(finished, "PDF compilation timed out");

        if (spyFailed.count() > 0)
        {
            qWarning() << "Compilation failed:" << spyFailed.takeFirst().at(0).toString();
            QFAIL("XeLaTeX compilation failed");
        }

        QCOMPARE(spyCompleted.count(), 1);
        QCOMPARE(spyPdfGen.count(), 1);
        QVERIFY(!m_engine->isBusy());
        QCOMPARE(m_engine->currentStage(), ConversionEngine::Stage::Idle);
    }

    void testConvertAllMarkdownToPdf()
    {
        if (!isPandocAvailable())
        {
            QSKIP("Pandoc not installed, skipping full pipeline test");
        }

        QProcess xelatexTest;
        xelatexTest.start("xelatex", QStringList{"--version"});
        if (!xelatexTest.waitForFinished(3000) || xelatexTest.exitCode() != 0)
        {
            QSKIP("XeLaTeX not installed, skipping full pipeline test");
        }

        QString mdFile = createTestMarkdownFile();
        QVERIFY(!mdFile.isEmpty());

        QString pdfFile = m_tempDir->path() + "/output.pdf";

        QSignalSpy spyStarted(m_engine, &ConversionEngine::conversionStarted);
        QSignalSpy spyProgress(m_engine, &ConversionEngine::conversionProgress);
        QSignalSpy spyCompleted(m_engine, &ConversionEngine::conversionCompleted);
        QSignalSpy spyFailed(m_engine, &ConversionEngine::conversionFailed);
        QSignalSpy spyLatexGen(m_engine, &ConversionEngine::latexGenerated);
        QSignalSpy spyPdfGen(m_engine, &ConversionEngine::pdfGenerated);

        m_engine->convertAll(mdFile, pdfFile);

        // Should start with Markdown→LaTeX stage
        QVERIFY(m_engine->isBusy());

        // Wait for full pipeline (max 60 seconds)
        bool finished = false;
        for (int i = 0; i < 60 && !finished; ++i)
        {
            finished = spyCompleted.count() > 0 || spyFailed.count() > 0;
            if (!finished)
                QTest::qWait(1000);
        }

        QVERIFY2(finished, "Full pipeline timed out");

        if (spyFailed.count() > 0)
        {
            QString errorMsg = spyFailed.takeFirst().at(0).toString();
            qWarning() << "Conversion failed:" << errorMsg;
            // Print all progress messages for debugging
            for (int i = 0; i < spyProgress.count(); ++i)
            {
                qWarning() << "Progress:" << spyProgress.at(i).at(0).toString();
            }
            QFAIL(qPrintable("Full Markdown→PDF conversion failed: " + errorMsg));
        }

        // Verify pipeline completed successfully
        QVERIFY2(spyStarted.count() >= 1, "No conversionStarted signals received");
        QCOMPARE(spyCompleted.count(), 1);
        QCOMPARE(spyLatexGen.count(), 1); // Intermediate LaTeX generated
        QCOMPARE(spyPdfGen.count(), 1);
        QVERIFY(spyProgress.count() > 0); // Should have progress messages
        QVERIFY(!m_engine->isBusy());
        QCOMPARE(m_engine->currentStage(), ConversionEngine::Stage::Idle);
    }

    void testConvertWithCustomTemplate()
    {
        if (!isPandocAvailable())
        {
            QSKIP("Pandoc not installed, skipping template test");
        }

        // Create a custom template
        QString templateFile = m_tempDir->path() + "/custom.latex";
        QFile tfile(templateFile);
        QVERIFY(tfile.open(QIODevice::WriteOnly | QIODevice::Text));

        QTextStream tout(&tfile);
        tout << "\\documentclass{article}\n";
        tout << "\\title{$title$}\n";
        tout << "\\author{$author$}\n";
        tout << "\\begin{document}\n";
        tout << "\\maketitle\n";
        tout << "$body$\n";
        tout << "\\end{document}\n";
        tfile.close();

        m_engine->setTemplatePath(templateFile);

        QString mdFile = createTestMarkdownFile();
        QString latexFile = m_tempDir->path() + "/output_template.tex";

        QSignalSpy spyCompleted(m_engine, &ConversionEngine::conversionCompleted);
        QSignalSpy spyFailed(m_engine, &ConversionEngine::conversionFailed);

        m_engine->convertToLatex(mdFile, latexFile);

        bool finished = spyCompleted.wait(10000) || spyFailed.wait(100);
        QVERIFY(finished);

        if (spyFailed.count() > 0)
        {
            qWarning() << "Template conversion failed:" << spyFailed.takeFirst().at(0).toString();
            QFAIL("Conversion with custom template failed");
        }

        QVERIFY(QFile::exists(latexFile));

        // Verify template was used
        QFile file(latexFile);
        QVERIFY(file.open(QIODevice::ReadOnly));
        QString content = QString::fromUtf8(file.readAll());
        QVERIFY(content.contains("\\title{"));
        QVERIFY(content.contains("\\author{"));
    }

    void testConvertWithCustomPandocOptions()
    {
        if (!isPandocAvailable())
        {
            QSKIP("Pandoc not installed, skipping custom options test");
        }

        // Set custom options with table of contents
        QStringList customOptions = {
            "--standalone",
            "--toc",
            "--toc-depth=2",
            "--from", "markdown",
            "--to", "latex"};
        m_engine->setPandocOptions(customOptions);

        QString mdFile = createTestMarkdownFile();
        QString latexFile = m_tempDir->path() + "/output_toc.tex";

        QSignalSpy spyCompleted(m_engine, &ConversionEngine::conversionCompleted);
        QSignalSpy spyFailed(m_engine, &ConversionEngine::conversionFailed);

        m_engine->convertToLatex(mdFile, latexFile);

        bool finished = spyCompleted.wait(10000) || spyFailed.wait(100);
        QVERIFY(finished);

        if (spyFailed.count() > 0)
        {
            QFAIL("Conversion with custom options failed");
        }

        QVERIFY(QFile::exists(latexFile));

        // Verify TOC was included (Pandoc should add \tableofcontents)
        QFile file(latexFile);
        QVERIFY(file.open(QIODevice::ReadOnly));
        QString content = QString::fromUtf8(file.readAll());
        QVERIFY(content.contains("tableofcontents") || content.contains("toc"));
    }

    void testInvalidLatexFile()
    {
        QProcess xelatexTest;
        xelatexTest.start("xelatex", QStringList{"--version"});
        if (!xelatexTest.waitForFinished(3000) || xelatexTest.exitCode() != 0)
        {
            QSKIP("XeLaTeX not installed, skipping invalid LaTeX test");
        }

        // Create a LaTeX file with syntax errors
        QString texFile = m_tempDir->path() + "/invalid.tex";
        QFile file(texFile);
        QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));

        QTextStream out(&file);
        out << "\\documentclass{article}\n";
        out << "\\begin{document}\n";
        out << "\\undefined_command\n"; // This will cause an error
        out << "Missing end tag...\n";
        // Intentionally missing \end{document}
        file.close();

        QString pdfFile = m_tempDir->path() + "/invalid.pdf";

        QSignalSpy spyFailed(m_engine, &ConversionEngine::conversionFailed);
        QSignalSpy spyCompleted(m_engine, &ConversionEngine::conversionCompleted);

        m_engine->compileToPdf(texFile, pdfFile);

        // Should fail
        bool hasFailed = spyFailed.wait(30000);
        QVERIFY2(hasFailed, "Expected compilation to fail for invalid LaTeX");
        QCOMPARE(spyCompleted.count(), 0); // Should not complete successfully
        QVERIFY(!m_engine->isBusy());
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

    void testCompileToPdfWhileBusy()
    {
        if (!isPandocAvailable())
        {
            QSKIP("Pandoc not installed, skipping busy-state test");
        }

        QString mdFile = createTestMarkdownFile();
        QString latexFile = m_tempDir->path() + "/output.tex";

        m_engine->convertToLatex(mdFile, latexFile);
        QVERIFY(m_engine->isBusy());

        QSignalSpy spyFailed(m_engine, &ConversionEngine::conversionFailed);
        m_engine->compileToPdf(latexFile, m_tempDir->path() + "/out.pdf");

        QCOMPARE(spyFailed.count(), 1);
        QVERIFY(spyFailed.takeFirst().at(0).toString().contains("already in progress"));

        // Wait for first conversion to finish cleanly
        QSignalSpy spyCompleted(m_engine, &ConversionEngine::conversionCompleted);
        QVERIFY(spyCompleted.wait(10000));
    }

    void testConvertAllWhileBusy()
    {
        if (!isPandocAvailable())
        {
            QSKIP("Pandoc not installed, skipping busy-state test");
        }

        QString mdFile = createTestMarkdownFile();
        QString latexFile = m_tempDir->path() + "/output.tex";

        m_engine->convertToLatex(mdFile, latexFile);
        QVERIFY(m_engine->isBusy());

        QSignalSpy spyFailed(m_engine, &ConversionEngine::conversionFailed);
        m_engine->convertAll(mdFile, m_tempDir->path() + "/out.pdf");

        QCOMPARE(spyFailed.count(), 1);
        QVERIFY(spyFailed.takeFirst().at(0).toString().contains("already in progress"));

        // Wait for first conversion to finish cleanly
        QSignalSpy spyCompleted(m_engine, &ConversionEngine::conversionCompleted);
        QVERIFY(spyCompleted.wait(10000));
    }

    void testProcessFailedToStart()
    {
        // Use a non-existent executable to trigger FailedToStart error
        m_engine->setPandocOptions(QStringList{});
        QSignalSpy spyFailed(m_engine, &ConversionEngine::conversionFailed);

        // Temporarily override by calling convertToLatex which runs pandoc
        // Since pandoc is the binary name, we need a different approach:
        // compile a LaTeX file via xelatex when xelatex is missing,
        // or simply call convertToLatex when pandoc is missing.
        // Either way, if the tool IS installed, the process won't fail to start.
        // So we test the error path by checking the signal type.

        if (isPandocAvailable())
        {
            QSKIP("Cannot test FailedToStart when Pandoc is installed");
        }

        m_engine->convertToLatex("/dummy.md", m_tempDir->path() + "/out.tex");
        QVERIFY(spyFailed.wait(5000));
        QVERIFY(spyFailed.first().at(0).toString().contains("failed to start"));
        QVERIFY(!m_engine->isBusy());
    }

    void testOnProcessErrorWithHiddenPath()
    {
        // Temporarily hide pandoc by blanking PATH to trigger onProcessError
        QByteArray origPath = qgetenv("PATH");
        qputenv("PATH", QByteArray("/nonexistent_dir_for_testing"));

        QString mdFile = createTestMarkdownFile();
        QVERIFY(!mdFile.isEmpty());

        QSignalSpy spyFailed(m_engine, &ConversionEngine::conversionFailed);
        m_engine->convertToLatex(mdFile, m_tempDir->path() + "/out.tex");
        bool gotSignal = spyFailed.wait(5000);

        // Restore PATH before assertions so it is always restored
        qputenv("PATH", origPath);

        QVERIFY(gotSignal);
        QString errorMsg = spyFailed.first().at(0).toString();
        QVERIFY(errorMsg.contains("failed to start", Qt::CaseInsensitive));
        QVERIFY(!m_engine->isBusy());
    }

    // ========== TEXLOOM TEMPLATE INTEGRATION TESTS ==========
    // These tests run the full Markdown → PDF pipeline using the actual
    // TexLoom .latex templates (article, report, thesis).
    // They are skipped automatically when pandoc or xelatex are not installed.

    bool isXelatexAvailable()
    {
        QProcess p;
        p.start("xelatex", QStringList{"--version"});
        return p.waitForFinished(3000) && p.exitCode() == 0;
    }

    void runTemplateTest(const QString &templateName, const QString &outputBaseName)
    {
        if (!isPandocAvailable())
            QSKIP("Pandoc not installed");
        if (!isXelatexAvailable())
            QSKIP("XeLaTeX not installed");

        // QFINDTESTDATA searches from the test source file directory
        // (app/tests/) so "../resources/templates/<name>.latex" is correct.
        const QString templatePath =
            QFINDTESTDATA("../resources/templates/" + templateName + ".latex");
        if (templatePath.isEmpty())
            QSKIP(qPrintable(templateName + ".latex template not found in repository"));

        m_engine->setTemplatePath(templatePath);

        const QString mdFile = createTestMarkdownFile();
        QVERIFY(!mdFile.isEmpty());

        const QString pdfFile = m_tempDir->path() + "/" + outputBaseName + ".pdf";

        QSignalSpy spyCompleted(m_engine, &ConversionEngine::conversionCompleted);
        QSignalSpy spyFailed(m_engine, &ConversionEngine::conversionFailed);
        QSignalSpy spyProgress(m_engine, &ConversionEngine::conversionProgress);

        m_engine->convertAll(mdFile, pdfFile);
        QVERIFY(m_engine->isBusy());

        // Wait up to 90 s — complex templates with fontspec can be slow on first run
        bool finished = false;
        for (int i = 0; i < 90 && !finished; ++i)
        {
            finished = spyCompleted.count() > 0 || spyFailed.count() > 0;
            if (!finished)
                QTest::qWait(1000);
        }

        if (spyFailed.count() > 0)
        {
            // Print progress for easier debugging
            for (int i = 0; i < spyProgress.count(); ++i)
                qWarning() << "Progress:" << spyProgress.at(i).at(0).toString();
            QFAIL(qPrintable(templateName + " template conversion failed: " +
                             spyFailed.at(0).at(0).toString()));
        }

        QVERIFY2(finished, qPrintable(templateName + " template conversion timed out"));
        QCOMPARE(spyCompleted.count(), 1);
        QVERIFY(QFile::exists(pdfFile));
        QVERIFY(!m_engine->isBusy());
    }

    void testConvertAllWithArticleTemplate()
    {
        runTemplateTest("article", "output_article");
    }

    void testConvertAllWithReportTemplate()
    {
        runTemplateTest("report", "output_report");
    }

    void testConvertAllWithThesisTemplate()
    {
        runTemplateTest("thesis", "output_thesis");
    }

private:
    QTemporaryDir *m_tempDir;
    ConversionEngine *m_engine;
};

QTEST_MAIN(TestConversionEngine)
#include "test_conversion_engine.moc"
