#include <QtTest/QtTest>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QFile>
#include "../src/ui/PreviewWidget.h"

using namespace texloom;

class TestPreviewWidget : public QObject
{
    Q_OBJECT

private slots:
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
        // Create a dummy file (content doesn't matter, we're not rendering it)
        QString pdfPath = m_tempDir->path() + "/test.pdf";
        QFile f(pdfPath);
        QVERIFY(f.open(QIODevice::WriteOnly));
        f.write("%PDF-1.4 dummy");
        f.close();

        QSignalSpy spyLoaded(m_widget, &PreviewWidget::pdfLoaded);
        QSignalSpy spyFailed(m_widget, &PreviewWidget::pdfLoadFailed);

        bool result = m_widget->loadPdf(pdfPath);

        QVERIFY(result);
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
        QString pdfPath = m_tempDir->path() + "/test.pdf";
        QFile f(pdfPath);
        QVERIFY(f.open(QIODevice::WriteOnly));
        f.write("%PDF");
        f.close();

        m_widget->loadPdf(pdfPath);
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
        QString pdfPath1 = m_tempDir->path() + "/first.pdf";
        QString pdfPath2 = m_tempDir->path() + "/second.pdf";

        QFile f1(pdfPath1);
        QVERIFY(f1.open(QIODevice::WriteOnly));
        f1.write("%PDF");
        f1.close();

        QFile f2(pdfPath2);
        QVERIFY(f2.open(QIODevice::WriteOnly));
        f2.write("%PDF");
        f2.close();

        m_widget->loadPdf(pdfPath1);
        QCOMPARE(m_widget->currentPdf(), pdfPath1);

        QSignalSpy spyLoaded(m_widget, &PreviewWidget::pdfLoaded);
        m_widget->loadPdf(pdfPath2);

        QCOMPARE(m_widget->currentPdf(), pdfPath2);
        QCOMPARE(spyLoaded.count(), 1);
    }

private:
    PreviewWidget *m_widget = nullptr;
    QTemporaryDir *m_tempDir = nullptr;
};

QTEST_MAIN(TestPreviewWidget)
#include "test_preview_widget.moc"
