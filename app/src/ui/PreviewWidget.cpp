#include "PreviewWidget.h"
#include <QVBoxLayout>
#include <QFileInfo>

namespace texloom
{

    PreviewWidget::PreviewWidget(QWidget *parent)
        : QWidget(parent)
    {
        QVBoxLayout *layout = new QVBoxLayout(this);

        // Placeholder for now
        m_placeholderLabel = new QLabel(tr("PDF Preview\n\n(Preview widget not yet implemented)"), this);
        m_placeholderLabel->setAlignment(Qt::AlignCenter);
        m_placeholderLabel->setStyleSheet("QLabel { color: gray; font-size: 14px; }");

        layout->addWidget(m_placeholderLabel);
        setLayout(layout);
    }

    bool PreviewWidget::loadPdf(const QString &pdfPath)
    {
        QFileInfo fileInfo(pdfPath);
        if (!fileInfo.exists())
        {
            emit pdfLoadFailed(tr("PDF file not found: %1").arg(pdfPath));
            return false;
        }

        m_currentPdf = pdfPath;

        // TODO: Implement actual PDF viewing
        // For now, just show the filename
        m_placeholderLabel->setText(tr("PDF Preview:\n\n%1\n\n(Viewer not yet implemented)")
                                        .arg(fileInfo.fileName()));

        emit pdfLoaded(pdfPath);
        return true;
    }

    void PreviewWidget::clear()
    {
        m_currentPdf.clear();
        m_placeholderLabel->setText(tr("PDF Preview\n\n(Preview widget not yet implemented)"));
    }

} // namespace texloom
