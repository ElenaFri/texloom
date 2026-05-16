#include "PreviewWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileInfo>
#include <QPdfDocument>
#include <QPdfPageNavigator>
#include <QPdfView>
#include <QToolButton>

namespace texloom
{

    PreviewWidget::PreviewWidget(QWidget *parent)
        : QWidget(parent)
    {
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

        return true;
    }

    void PreviewWidget::clear()
    {
        m_pdfDocument->close();
        m_pendingPdfPath.clear();
        m_currentPdf.clear();
        updateNavigationUi();
        setStatusMessage(tr("No PDF loaded"));
    }

    void PreviewWidget::updateNavigationUi()
    {
        const int pageCount = m_pdfDocument->pageCount();
        const int currentPage = m_pdfView->pageNavigator()->currentPage();

        const bool hasPages = pageCount > 0;
        m_prevPageButton->setEnabled(hasPages && currentPage > 0);
        m_nextPageButton->setEnabled(hasPages && currentPage + 1 < pageCount);

        if (hasPages)
        {
            m_pageLabel->setText(tr("Page %1 / %2").arg(currentPage + 1).arg(pageCount));
        }
        else
        {
            m_pageLabel->setText(tr("Page - / -"));
        }

        m_zoomInButton->setEnabled(hasPages);
        m_zoomOutButton->setEnabled(hasPages);
        m_fitWidthButton->setEnabled(hasPages);
        m_fitPageButton->setEnabled(hasPages);
    }

    void PreviewWidget::setStatusMessage(const QString &message)
    {
        m_statusLabel->setText(message);
    }

} // namespace texloom
