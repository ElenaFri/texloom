#pragma once

#include <QObject>
#include <QString>
#include <QProcess>
#include <QTemporaryDir>

namespace texloom
{

    /**
     * @brief Handles Markdown to LaTeX to PDF conversion pipeline
     *
     * Uses Pandoc for Markdown->LaTeX conversion and XeLaTeX for PDF compilation.
     * Runs conversions asynchronously using QProcess and emits signals for progress.
     */
    class ConversionEngine : public QObject
    {
        Q_OBJECT

    public:
        enum class Stage
        {
            Idle,
            ConvertingToLatex,
            CompilingToPdf
        };

        explicit ConversionEngine(QObject *parent = nullptr);
        ~ConversionEngine() override;

        // Conversion operations
        void convertToLatex(const QString &markdownFile, const QString &outputLatex);
        void compileToPdf(const QString &latexFile, const QString &outputPdf);
        void convertAll(const QString &markdownFile, const QString &outputPdf);

        // State
        Stage currentStage() const { return m_stage; }
        bool isBusy() const { return m_stage != Stage::Idle; }

        // Options
        void setPandocOptions(const QStringList &options);
        void setXelatexOptions(const QStringList &options);
        void setTemplatePath(const QString &path);

    signals:
        void conversionStarted(Stage stage);
        void conversionProgress(const QString &message);
        void conversionCompleted(const QString &outputFile);
        void conversionFailed(const QString &error);

        void latexGenerated(const QString &latexFile);
        void pdfGenerated(const QString &pdfFile);

    private slots:
        void onPandocFinished(int exitCode, QProcess::ExitStatus status);
        void onXelatexFinished(int exitCode, QProcess::ExitStatus status);
        void onProcessError(QProcess::ProcessError error);

    private:
        void runPandoc(const QString &input, const QString &output);
        void runXelatex(const QString &input, const QString &output);
        void cleanupTempFiles();

        Stage m_stage = Stage::Idle;

        QProcess *m_pandocProcess = nullptr;
        QProcess *m_xelatexProcess = nullptr;

        QStringList m_pandocOptions;
        QStringList m_xelatexOptions;
        QString m_templatePath;

        // For multi-stage conversion
        QString m_pendingPdfOutput;
        QString m_tempLatexFile;
        QTemporaryDir *m_tempDir = nullptr;
    };

} // namespace texloom
