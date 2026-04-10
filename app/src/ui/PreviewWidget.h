#pragma once

#include <QWidget>
#include <QLabel>

namespace texloom
{

    /**
     * @brief PDF preview widget
     *
     * Displays compiled PDF output.
     * TODO: Use QPdfView (Qt 6.4+) or external PDF viewer widget
     */
    class PreviewWidget : public QWidget
    {
        Q_OBJECT

    public:
        explicit PreviewWidget(QWidget *parent = nullptr);
        ~PreviewWidget() override = default;

        // Non-copyable and non-movable (QWidget already is)
        PreviewWidget(const PreviewWidget &) = delete;
        PreviewWidget &operator=(const PreviewWidget &) = delete;
        PreviewWidget(PreviewWidget &&) = delete;
        PreviewWidget &operator=(PreviewWidget &&) = delete;

        // Preview operations
        bool loadPdf(const QString &pdfPath);
        void clear();

        [[nodiscard]] QString currentPdf() const noexcept { return m_currentPdf; }

    signals:
        void pdfLoaded(const QString &path);
        void pdfLoadFailed(const QString &error);

    private:
        QString m_currentPdf;
        QLabel *m_placeholderLabel = nullptr;

        // TODO: Replace with actual PDF viewer widget
        // QPdfView* m_pdfView = nullptr;
    };

} // namespace texloom
