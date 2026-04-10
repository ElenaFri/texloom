#include <QtTest/QtTest>

class TestExample : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase()
    {
        // Called before the first test function is executed
    }

    void testBasicArithmetic()
    {
        QCOMPARE(1 + 1, 2);
        QCOMPARE(10 - 5, 5);
    }

    void testStringOperations()
    {
        QString str = "TexLoom";
        QCOMPARE(str.length(), 7);
        QVERIFY(str.contains("Loom"));
    }

    void cleanupTestCase()
    {
        // Called after the last test function is executed
    }
};

QTEST_MAIN(TestExample)
#include "test_example.moc"
