#pragma once

#include <QWidget>
#include <QLabel>

QT_BEGIN_NAMESPACE
class QPdfDocument;
class QPdfView;
class QToolButton;
QT_END_NAMESPACE

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
        void updateNavigationUi();
        void setStatusMessage(const QString &message);

        QString m_currentPdf;
        QString m_pendingPdfPath;

        QPdfDocument *m_pdfDocument = nullptr;
        QPdfView *m_pdfView = nullptr;

        QToolButton *m_prevPageButton = nullptr;
        QToolButton *m_nextPageButton = nullptr;
        QLabel *m_pageLabel = nullptr;

        QToolButton *m_zoomOutButton = nullptr;
        QToolButton *m_zoomInButton = nullptr;
        QToolButton *m_fitWidthButton = nullptr;
        QToolButton *m_fitPageButton = nullptr;

        QLabel *m_statusLabel = nullptr;
    };

} // namespace texloom
