#include <QtTest/QtTest>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QFile>
#include <QPainter>
#include <QPdfWriter>
#include <QToolButton>
#include "../src/ui/PreviewWidget.h"

using namespace texloom;

class TestPreviewWidget : public QObject
{
    Q_OBJECT

private slots:
    QString createValidPdf(const QString &name)
    {
        const QString path = m_tempDir->path() + "/" + name;
        QPdfWriter writer(path);
        writer.setPageSize(QPageSize(QPageSize::A4));

        QPainter painter(&writer);
        painter.drawText(QPointF(100.0, 100.0), QStringLiteral("TexLoom Test PDF"));
        painter.end();

        return path;
    }

    void init()
    {
        m_widget = new PreviewWidget();
        m_tempDir = new QTemporaryDir();
        QVERIFY(m_tempDir->isValid());
    }

    void cleanup()
    {
        delete m_widget;
        delete m_tempDir;
    }

    // ========== INITIAL STATE ==========

    void testInitialState()
    {
        QVERIFY(m_widget->currentPdf().isEmpty());
    }

    // ========== loadPdf ==========

    void testLoadExistingPdf()
    {
        const QString pdfPath = createValidPdf("test.pdf");

        QSignalSpy spyLoaded(m_widget, &PreviewWidget::pdfLoaded);
        QSignalSpy spyFailed(m_widget, &PreviewWidget::pdfLoadFailed);

        const bool result = m_widget->loadPdf(pdfPath);

        QVERIFY(result);

        if (spyLoaded.count() == 0)
        {
            QVERIFY(spyLoaded.wait(3000));
        }

        QCOMPARE(m_widget->currentPdf(), pdfPath);
        QCOMPARE(spyLoaded.count(), 1);
        QCOMPARE(spyFailed.count(), 0);
        QCOMPARE(spyLoaded.first().at(0).toString(), pdfPath);
    }

    void testLoadNonExistentPdf()
    {
        QString fakePath = m_tempDir->path() + "/does_not_exist.pdf";

        QSignalSpy spyLoaded(m_widget, &PreviewWidget::pdfLoaded);
        QSignalSpy spyFailed(m_widget, &PreviewWidget::pdfLoadFailed);

        bool result = m_widget->loadPdf(fakePath);

        QVERIFY(!result);
        QVERIFY(m_widget->currentPdf().isEmpty());
        QCOMPARE(spyLoaded.count(), 0);
        QCOMPARE(spyFailed.count(), 1);
        QVERIFY(spyFailed.first().at(0).toString().contains("not found"));
    }

    // ========== clear ==========

    void testClearAfterLoad()
    {
        const QString pdfPath = createValidPdf("clear.pdf");

        m_widget->loadPdf(pdfPath);
        QTRY_VERIFY(!m_widget->currentPdf().isEmpty());
        QVERIFY(!m_widget->currentPdf().isEmpty());

        m_widget->clear();

        QVERIFY(m_widget->currentPdf().isEmpty());
    }

    void testClearOnEmptyWidget()
    {
        // Should not crash
        m_widget->clear();
        QVERIFY(m_widget->currentPdf().isEmpty());
    }

    // ========== RELOAD ==========

    void testReloadWithDifferentFile()
    {
        const QString pdfPath1 = createValidPdf("first.pdf");
        const QString pdfPath2 = createValidPdf("second.pdf");

        m_widget->loadPdf(pdfPath1);
        QTRY_COMPARE(m_widget->currentPdf(), pdfPath1);
        QCOMPARE(m_widget->currentPdf(), pdfPath1);

        QSignalSpy spyLoaded(m_widget, &PreviewWidget::pdfLoaded);
        m_widget->loadPdf(pdfPath2);

        if (spyLoaded.count() == 0)
        {
            QVERIFY(spyLoaded.wait(3000));
        }

        QCOMPARE(m_widget->currentPdf(), pdfPath2);
        QCOMPARE(spyLoaded.count(), 1);
    }

    void testNavigationAndZoomControlsExist()
    {
#if TEXLOOM_HAS_QT_PDF
        const QList<QToolButton *> buttons = m_widget->findChildren<QToolButton *>();
        QVERIFY(buttons.size() >= 6);
#else
        const QList<QToolButton *> buttons = m_widget->findChildren<QToolButton *>();
        QCOMPARE(buttons.size(), 0);
#endif
    }

private:
    PreviewWidget *m_widget = nullptr;
    QTemporaryDir *m_tempDir = nullptr;
};

QTEST_MAIN(TestPreviewWidget)
#include "test_preview_widget.moc"
