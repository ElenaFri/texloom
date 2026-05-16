#include "PreviewWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileInfo>
#if TEXLOOM_HAS_QT_PDF
#include <QPdfDocument>
#include <QPdfPageNavigator>
#include <QPdfView>
#endif
#include <QToolButton>

namespace texloom
{

    PreviewWidget::PreviewWidget(QWidget *parent)
        : QWidget(parent)
    {
#if TEXLOOM_HAS_QT_PDF
        m_pdfDocument = new QPdfDocument(this);
        m_pdfView = new QPdfView(this);
        m_pdfView->setDocument(m_pdfDocument);
        m_pdfView->setPageMode(QPdfView::PageMode::MultiPage);
        m_pdfView->setZoomMode(QPdfView::ZoomMode::FitInView);

        m_prevPageButton = new QToolButton(this);
        m_prevPageButton->setText(tr("Previous"));
        m_nextPageButton = new QToolButton(this);
        m_nextPageButton->setText(tr("Next"));
        m_pageLabel = new QLabel(this);
        m_pageLabel->setAlignment(Qt::AlignCenter);
        m_pageLabel->setMinimumWidth(92);

        m_zoomOutButton = new QToolButton(this);
        m_zoomOutButton->setText(tr("-"));
        m_zoomInButton = new QToolButton(this);
        m_zoomInButton->setText(tr("+"));
        m_fitWidthButton = new QToolButton(this);
        m_fitWidthButton->setText(tr("Fit Width"));
        m_fitPageButton = new QToolButton(this);
        m_fitPageButton->setText(tr("Fit Page"));

        m_statusLabel = new QLabel(this);
        m_statusLabel->setStyleSheet("QLabel { color: gray; }");

        auto *controlsLayout = new QHBoxLayout();
        controlsLayout->addWidget(m_prevPageButton);
        controlsLayout->addWidget(m_nextPageButton);
        controlsLayout->addWidget(m_pageLabel);
        controlsLayout->addStretch();
        controlsLayout->addWidget(m_zoomOutButton);
        controlsLayout->addWidget(m_zoomInButton);
        controlsLayout->addWidget(m_fitWidthButton);
        controlsLayout->addWidget(m_fitPageButton);

        auto *layout = new QVBoxLayout(this);
        layout->addLayout(controlsLayout);
        layout->addWidget(m_pdfView);
        layout->addWidget(m_statusLabel);
        setLayout(layout);

        connect(m_pdfDocument, &QPdfDocument::statusChanged, this, [this](QPdfDocument::Status status)
                {
            if (status == QPdfDocument::Status::Ready)
            {
                // Emit pdfLoaded exactly once per loadPdf request.
                if (m_pendingPdfPath.isEmpty())
                {
                    updateNavigationUi();
                    return;
                }

                m_currentPdf = m_pendingPdfPath;
                m_pendingPdfPath.clear();

                if (m_pdfDocument->pageCount() > 0 && m_pdfView->pageNavigator()->currentPage() < 0)
                {
                    m_pdfView->pageNavigator()->jump(0, QPointF(0.0, 0.0));
                }

                setStatusMessage(tr("Loaded PDF: %1").arg(QFileInfo(m_currentPdf).fileName()));
                updateNavigationUi();
                emit pdfLoaded(m_currentPdf);
                return;
            }

            if (status == QPdfDocument::Status::Loading)
            {
                setStatusMessage(tr("Loading PDF..."));
                return;
            }

            if (status == QPdfDocument::Status::Error)
            {
                const QString error = tr("Failed to load PDF: %1").arg(static_cast<int>(m_pdfDocument->error()));
                m_currentPdf.clear();
                m_pendingPdfPath.clear();
                updateNavigationUi();
                setStatusMessage(error);
                emit pdfLoadFailed(error);
            } });

        connect(m_pdfDocument, &QPdfDocument::pageCountChanged,
                this, [this](int)
                { updateNavigationUi(); });

        connect(m_pdfView->pageNavigator(), &QPdfPageNavigator::currentPageChanged,
                this, [this](int)
                { updateNavigationUi(); });

        connect(m_prevPageButton, &QToolButton::clicked, this, [this]()
                {
            const int pageCount = m_pdfDocument->pageCount();
            const int currentPage = m_pdfView->pageNavigator()->currentPage();
            if (pageCount <= 0 || currentPage <= 0)
                return;
            m_pdfView->pageNavigator()->jump(currentPage - 1, QPointF(0.0, 0.0)); });

        connect(m_nextPageButton, &QToolButton::clicked, this, [this]()
                {
            const int pageCount = m_pdfDocument->pageCount();
            const int currentPage = m_pdfView->pageNavigator()->currentPage();
            if (pageCount <= 0 || currentPage + 1 >= pageCount)
                return;
            m_pdfView->pageNavigator()->jump(currentPage + 1, QPointF(0.0, 0.0)); });

        connect(m_zoomInButton, &QToolButton::clicked, this, [this]()
                {
            m_pdfView->setZoomMode(QPdfView::ZoomMode::Custom);
            m_pdfView->setZoomFactor(qMin(8.0, m_pdfView->zoomFactor() * 1.15)); });

        connect(m_zoomOutButton, &QToolButton::clicked, this, [this]()
                {
            m_pdfView->setZoomMode(QPdfView::ZoomMode::Custom);
            m_pdfView->setZoomFactor(qMax(0.1, m_pdfView->zoomFactor() / 1.15)); });

        connect(m_fitWidthButton, &QToolButton::clicked, this, [this]()
                { m_pdfView->setZoomMode(QPdfView::ZoomMode::FitToWidth); });

        connect(m_fitPageButton, &QToolButton::clicked, this, [this]()
                { m_pdfView->setZoomMode(QPdfView::ZoomMode::FitInView); });
#else
        m_fallbackLabel = new QLabel(tr("PDF preview unavailable\n\nInstall Qt6 Pdf/PdfWidgets to enable embedded preview."), this);
        m_fallbackLabel->setAlignment(Qt::AlignCenter);
        m_fallbackLabel->setStyleSheet("QLabel { color: gray; font-size: 13px; }");

        auto *layout = new QVBoxLayout(this);
        layout->addWidget(m_fallbackLabel);
        setLayout(layout);
#endif

        clear();
    }

    bool PreviewWidget::loadPdf(const QString &pdfPath)
    {
        QFileInfo fileInfo(pdfPath);
        if (!fileInfo.exists())
        {
            emit pdfLoadFailed(tr("PDF file not found: %1").arg(pdfPath));
            return false;
        }

#if TEXLOOM_HAS_QT_PDF
        m_pendingPdfPath = pdfPath;
        const QPdfDocument::Error error = m_pdfDocument->load(pdfPath);
        if (error != QPdfDocument::Error::None)
        {
            m_pendingPdfPath.clear();
            m_currentPdf.clear();
            const QString message = tr("Failed to load PDF: %1").arg(static_cast<int>(error));
            setStatusMessage(message);
            emit pdfLoadFailed(message);
            return false;
        }

        // Completion is reported from the statusChanged(Ready) callback.
#else
        m_currentPdf = pdfPath;
        if (m_fallbackLabel)
        {
            m_fallbackLabel->setText(tr("PDF Preview (fallback mode):\n\n%1\n\nInstall Qt6 Pdf/PdfWidgets for embedded rendering.")
                                         .arg(fileInfo.fileName()));
        }
        emit pdfLoaded(pdfPath);
#endif

        return true;
    }

    void PreviewWidget::clear()
    {
#if TEXLOOM_HAS_QT_PDF
        m_pdfDocument->close();
        m_pendingPdfPath.clear();
        m_currentPdf.clear();
        updateNavigationUi();
        setStatusMessage(tr("No PDF loaded"));
#else
        m_currentPdf.clear();
        if (m_fallbackLabel)
        {
            m_fallbackLabel->setText(tr("PDF preview unavailable\n\nInstall Qt6 Pdf/PdfWidgets to enable embedded preview."));
        }
#endif
    }

    void PreviewWidget::updateNavigationUi()
    {
#if TEXLOOM_HAS_QT_PDF
        const int pageCount = m_pdfDocument->pageCount();
        const int rawCurrentPage = m_pdfView->pageNavigator()->currentPage();
        const int currentPage = qMax(0, rawCurrentPage);

        const bool hasPages = pageCount > 0;
        m_prevPageButton->setEnabled(hasPages && rawCurrentPage > 0);
        m_nextPageButton->setEnabled(hasPages && currentPage + 1 < pageCount);

        if (hasPages)
        {
            m_pageLabel->setText(tr("Page %1 / %2").arg(currentPage + 1).arg(pageCount));
        }
        else
        {
            m_pageLabel->setText(tr("Page 0 / 0"));
        }

        m_zoomInButton->setEnabled(hasPages);
        m_zoomOutButton->setEnabled(hasPages);
        m_fitWidthButton->setEnabled(hasPages);
        m_fitPageButton->setEnabled(hasPages);
#endif
    }

    void PreviewWidget::setStatusMessage(const QString &message)
    {
        if (m_statusLabel)
        {
            m_statusLabel->setText(message);
        }
    }

} // namespace texloom
