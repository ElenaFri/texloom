#include "ConversionEngine.h"
#include <QFileInfo>
#include <QDir>
#include <QFile>
#include <QDebug>

namespace texloom
{

    ConversionEngine::ConversionEngine(QObject *parent)
        : QObject(parent)
    {
        // Default Pandoc options
        m_pandocOptions = QStringList{
            "--standalone",
            "--from", "markdown",
            "--to", "latex"};

        // Default XeLaTeX options
        m_xelatexOptions = QStringList{
            "-interaction=nonstopmode",
            "-halt-on-error"};
    }

    ConversionEngine::~ConversionEngine()
    {
        // QProcess objects are auto-deleted via Qt parent ownership
        if (m_pandocProcess)
        {
            m_pandocProcess->kill();
        }

        if (m_xelatexProcess)
        {
            m_xelatexProcess->kill();
        }

        // QTemporaryDir needs manual cleanup
        delete m_tempDir;
    }

    void ConversionEngine::convertToLatex(const QString &markdownFile, const QString &outputLatex)
    {
        if (isBusy())
        {
            emit conversionFailed("Conversion already in progress");
            return;
        }

        m_stage = Stage::ConvertingToLatex;
        emit conversionStarted(m_stage);
        runPandoc(markdownFile, outputLatex);
    }

    void ConversionEngine::compileToPdf(const QString &latexFile, const QString &outputPdf)
    {
        if (isBusy())
        {
            emit conversionFailed("Compilation already in progress");
            return;
        }

        m_stage = Stage::CompilingToPdf;
        emit conversionStarted(m_stage);
        runXelatex(latexFile, outputPdf);
    }

    void ConversionEngine::convertAll(const QString &markdownFile, const QString &outputPdf)
    {
        if (isBusy())
        {
            emit conversionFailed("Conversion already in progress");
            return;
        }

        // Create temporary directory for intermediate files
        delete m_tempDir;
        m_tempDir = new QTemporaryDir();

        if (!m_tempDir->isValid())
        {
            emit conversionFailed("Cannot create temporary directory");
            return;
        }

        QFileInfo mdInfo(markdownFile);
        m_tempLatexFile = m_tempDir->path() + "/" + mdInfo.baseName() + ".tex";
        m_pendingPdfOutput = outputPdf;

        m_stage = Stage::ConvertingToLatex;
        emit conversionStarted(m_stage);
        runPandoc(markdownFile, m_tempLatexFile);
    }

    void ConversionEngine::setPandocOptions(const QStringList &options)
    {
        m_pandocOptions = options;
    }

    void ConversionEngine::setXelatexOptions(const QStringList &options)
    {
        m_xelatexOptions = options;
    }

    void ConversionEngine::setTemplatePath(const QString &path)
    {
        m_templatePath = path;
    }

    void ConversionEngine::runPandoc(const QString &input, const QString &output)
    {
        // Delete old process if exists (not owned by parent if recreated)
        if (m_pandocProcess)
        {
            m_pandocProcess->deleteLater();
        }
        m_pandocProcess = new QProcess(this);

        connect(m_pandocProcess, &QProcess::finished,
                this, &ConversionEngine::onPandocFinished);
        connect(m_pandocProcess, &QProcess::errorOccurred,
                this, &ConversionEngine::onProcessError);

        QStringList args = m_pandocOptions;

        if (!m_templatePath.isEmpty())
        {
            args << "--template" << m_templatePath;
        }

        args << "-o" << output << input;

        emit conversionProgress("Running: pandoc " + args.join(" "));
        m_pandocProcess->start("pandoc", args);
    }

    void ConversionEngine::runXelatex(const QString &input, const QString &output)
    {
        // Delete old process if exists (not owned by parent if recreated)
        if (m_xelatexProcess)
        {
            m_xelatexProcess->deleteLater();
        }
        m_xelatexProcess = new QProcess(this);

        connect(m_xelatexProcess, &QProcess::finished,
                this, &ConversionEngine::onXelatexFinished);
        connect(m_xelatexProcess, &QProcess::errorOccurred,
                this, &ConversionEngine::onProcessError);

        QFileInfo inputInfo(input);
        QFileInfo outputInfo(output);

        // XeLaTeX needs to run in the directory containing the .tex file
        m_xelatexProcess->setWorkingDirectory(inputInfo.absolutePath());

        QStringList args = m_xelatexOptions;
        args << "-output-directory" << outputInfo.absolutePath();
        args << input;

        emit conversionProgress("Running: xelatex " + args.join(" "));
        m_xelatexProcess->start("xelatex", args);
    }

    void ConversionEngine::onPandocFinished(int exitCode, QProcess::ExitStatus status)
    {
        if (status != QProcess::NormalExit || exitCode != 0)
        {
            QString error = m_pandocProcess->readAllStandardError();
            m_stage = Stage::Idle;
            emit conversionFailed("Pandoc failed: " + error);
            return;
        }

        QString output = m_pandocProcess->readAllStandardOutput();
        emit conversionProgress(output);

        // If this was Markdown->LaTeX for a full conversion, continue to PDF
        if (!m_pendingPdfOutput.isEmpty())
        {
            emit latexGenerated(m_tempLatexFile);
            m_stage = Stage::CompilingToPdf;
            emit conversionStarted(m_stage);
            runXelatex(m_tempLatexFile, m_pendingPdfOutput);
            return;
        }

        m_stage = Stage::Idle;
        emit latexGenerated(m_pandocProcess->arguments().last());
        emit conversionCompleted(m_pandocProcess->arguments().last());
    }

    void ConversionEngine::onXelatexFinished(int exitCode, QProcess::ExitStatus status)
    {
        if (status != QProcess::NormalExit || exitCode != 0)
        {
            QString error = m_xelatexProcess->readAllStandardError();
            QString output = m_xelatexProcess->readAllStandardOutput();
            m_stage = Stage::Idle;
            m_pendingPdfOutput.clear();

            // XeLaTeX often writes errors to stdout, not stderr
            QString fullError = error;
            if (!output.isEmpty())
            {
                fullError += (fullError.isEmpty() ? "" : "\n") + output;
            }

            emit conversionFailed("XeLaTeX failed: " + fullError);
            cleanupTempFiles();
            return;
        }

        QString output = m_xelatexProcess->readAllStandardOutput();
        emit conversionProgress(output);

        // XeLaTeX generates a PDF with the same basename as the .tex file
        // We need to find it and potentially rename it to the desired output
        QString texFile = m_xelatexProcess->arguments().last();
        QFileInfo texInfo(texFile);

        // Find the -output-directory argument
        QStringList args = m_xelatexProcess->arguments();
        QString outputDir = m_xelatexProcess->workingDirectory(); // default
        int outDirIdx = args.indexOf("-output-directory");
        if (outDirIdx != -1 && outDirIdx + 1 < args.size())
        {
            outputDir = args.at(outDirIdx + 1);
        }

        QString generatedPdf = outputDir + "/" + texInfo.completeBaseName() + ".pdf";

        QString finalPdfPath;
        if (!m_pendingPdfOutput.isEmpty())
        {
            finalPdfPath = m_pendingPdfOutput;

            // If the generated PDF is not where we want it, move it
            if (QFileInfo(generatedPdf).absoluteFilePath() != QFileInfo(finalPdfPath).absoluteFilePath())
            {
                QFile generatedFile(generatedPdf);
                if (!generatedFile.exists())
                {
                    m_stage = Stage::Idle;
                    m_pendingPdfOutput.clear();
                    emit conversionFailed("XeLaTeX did not generate expected PDF: " + generatedPdf);
                    cleanupTempFiles();
                    return;
                }

                // Remove target if exists
                QFile::remove(finalPdfPath);

                if (!generatedFile.rename(finalPdfPath))
                {
                    m_stage = Stage::Idle;
                    m_pendingPdfOutput.clear();
                    emit conversionFailed("Failed to move PDF from " + generatedPdf + " to " + finalPdfPath);
                    cleanupTempFiles();
                    return;
                }
            }
        }
        else
        {
            // Direct compilation, PDF is where XeLaTeX put it
            finalPdfPath = generatedPdf;
        }

        m_stage = Stage::Idle;
        m_pendingPdfOutput.clear();

        emit pdfGenerated(finalPdfPath);
        emit conversionCompleted(finalPdfPath);

        cleanupTempFiles();
    }

    void ConversionEngine::onProcessError(QProcess::ProcessError error)
    {
        QString errorMsg;

        switch (error)
        {
        case QProcess::FailedToStart:
            errorMsg = "Process failed to start. Is the tool installed?";
            break;
        case QProcess::Crashed:
            errorMsg = "Process crashed";
            break;
        case QProcess::Timedout:
            errorMsg = "Process timed out";
            break;
        default:
            errorMsg = "Unknown process error";
        }

        m_stage = Stage::Idle;
        m_pendingPdfOutput.clear();
        emit conversionFailed(errorMsg);
        cleanupTempFiles();
    }

    void ConversionEngine::cleanupTempFiles()
    {
        delete m_tempDir;
        m_tempDir = nullptr;
        m_tempLatexFile.clear();
    }

} // namespace texloom
