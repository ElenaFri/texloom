#include <QtTest/QtTest>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QTemporaryDir>
#include <QFile>
#include "../src/ui/NewProjectDialog.h"

using namespace texloom;

class TestNewProjectDialog : public QObject
{
    Q_OBJECT

private:
    QLineEdit *nameEdit(NewProjectDialog &d) { return d.findChild<QLineEdit *>(QString(), Qt::FindDirectChildrenOnly); }

    QLineEdit *locationEdit(NewProjectDialog &d)
    {
        auto edits = d.findChildren<QLineEdit *>(QString(), Qt::FindDirectChildrenOnly);
        return edits.size() >= 2 ? edits.at(1) : nullptr;
    }

    QComboBox *templateCombo(NewProjectDialog &d) { return d.findChild<QComboBox *>(); }

    QPushButton *createButton(NewProjectDialog &d)
    {
        for (auto *btn : d.findChildren<QPushButton *>())
        {
            if (btn->text() == "Create")
                return btn;
        }
        return nullptr;
    }

    QPushButton *cancelButton(NewProjectDialog &d)
    {
        for (auto *btn : d.findChildren<QPushButton *>())
        {
            if (btn->text() == "Cancel")
                return btn;
        }
        return nullptr;
    }

    // Create a temp templates directory with .latex files
    QString createTempTemplates(QTemporaryDir &dir, const QStringList &names)
    {
        QString tplDir = dir.path() + "/templates";
        QDir().mkpath(tplDir);
        for (const QString &name : names)
        {
            QFile f(tplDir + "/" + name + ".latex");
            f.open(QIODevice::WriteOnly);
            f.write("% template");
            f.close();
        }
        return tplDir;
    }

private slots:
    void testDialogTitle()
    {
        QTemporaryDir tmpDir;
        QString tplDir = createTempTemplates(tmpDir, {"article", "report"});
        NewProjectDialog d(tplDir);
        QCOMPARE(d.windowTitle(), tr("New Project"));
    }

    void testFieldsExist()
    {
        QTemporaryDir tmpDir;
        QString tplDir = createTempTemplates(tmpDir, {"article"});
        NewProjectDialog d(tplDir);
        QVERIFY(nameEdit(d) != nullptr);
        QVERIFY(locationEdit(d) != nullptr);
        QVERIFY(templateCombo(d) != nullptr);
        QVERIFY(createButton(d) != nullptr);
        QVERIFY(cancelButton(d) != nullptr);
    }

    void testCreateButtonDisabledByDefault()
    {
        QTemporaryDir tmpDir;
        QString tplDir = createTempTemplates(tmpDir, {"article"});
        NewProjectDialog d(tplDir);
        QVERIFY(!createButton(d)->isEnabled());
    }

    void testCreateButtonEnabledWithValidInput()
    {
        QTemporaryDir tmpDir;
        QString tplDir = createTempTemplates(tmpDir, {"article"});
        NewProjectDialog d(tplDir);
        QTemporaryDir dir;
        QVERIFY(dir.isValid());

        nameEdit(d)->setText("MyProject");
        locationEdit(d)->setText(dir.path());

        QVERIFY(createButton(d)->isEnabled());
    }

    void testCreateButtonDisabledWithEmptyName()
    {
        QTemporaryDir tmpDir;
        QString tplDir = createTempTemplates(tmpDir, {"article"});
        NewProjectDialog d(tplDir);
        QTemporaryDir dir;
        QVERIFY(dir.isValid());

        nameEdit(d)->setText("");
        locationEdit(d)->setText(dir.path());

        QVERIFY(!createButton(d)->isEnabled());
    }

    void testCreateButtonDisabledWithEmptyLocation()
    {
        QTemporaryDir tmpDir;
        QString tplDir = createTempTemplates(tmpDir, {"article"});
        NewProjectDialog d(tplDir);

        nameEdit(d)->setText("MyProject");
        locationEdit(d)->setText("");

        QVERIFY(!createButton(d)->isEnabled());
    }

    void testCreateButtonDisabledWithInvalidLocation()
    {
        QTemporaryDir tmpDir;
        QString tplDir = createTempTemplates(tmpDir, {"article"});
        NewProjectDialog d(tplDir);

        nameEdit(d)->setText("MyProject");
        locationEdit(d)->setText("/nonexistent/path/that/does/not/exist");

        QVERIFY(!createButton(d)->isEnabled());
    }

    void testProjectNameAccessor()
    {
        QTemporaryDir tmpDir;
        QString tplDir = createTempTemplates(tmpDir, {"article"});
        NewProjectDialog d(tplDir);
        nameEdit(d)->setText("  Test Project  ");
        QCOMPARE(d.projectName(), QString("Test Project"));
    }

    void testProjectLocationAccessor()
    {
        QTemporaryDir tmpDir;
        QString tplDir = createTempTemplates(tmpDir, {"article"});
        NewProjectDialog d(tplDir);
        QTemporaryDir dir;
        QVERIFY(dir.isValid());

        locationEdit(d)->setText(dir.path());
        QCOMPARE(d.projectLocation(), dir.path());
    }

    void testTemplatesScannedFromDirectory()
    {
        QTemporaryDir tmpDir;
        QString tplDir = createTempTemplates(tmpDir, {"article", "report", "thesis"});
        NewProjectDialog d(tplDir);
        auto *combo = templateCombo(d);
        QCOMPARE(combo->count(), 3);
        QCOMPARE(combo->itemData(0).toString(), QString("article"));
        QCOMPARE(combo->itemData(1).toString(), QString("report"));
        QCOMPARE(combo->itemData(2).toString(), QString("thesis"));
    }

    void testTemplateDisplayNameCapitalized()
    {
        QTemporaryDir tmpDir;
        QString tplDir = createTempTemplates(tmpDir, {"article", "thesis"});
        NewProjectDialog d(tplDir);
        auto *combo = templateCombo(d);
        QCOMPARE(combo->itemText(0), QString("Article"));
        QCOMPARE(combo->itemText(1), QString("Thesis"));
    }

    void testDefaultTemplateIsFirst()
    {
        QTemporaryDir tmpDir;
        QString tplDir = createTempTemplates(tmpDir, {"article", "report"});
        NewProjectDialog d(tplDir);
        QCOMPARE(d.templateName(), QString("article"));
    }

    void testTemplateSelection()
    {
        QTemporaryDir tmpDir;
        QString tplDir = createTempTemplates(tmpDir, {"article", "report", "thesis"});
        NewProjectDialog d(tplDir);
        templateCombo(d)->setCurrentIndex(2);
        QCOMPARE(d.templateName(), QString("thesis"));
    }

    void testEmptyTemplatesDirectory()
    {
        QTemporaryDir tmpDir;
        QString tplDir = tmpDir.path() + "/empty_templates";
        QDir().mkpath(tplDir);
        NewProjectDialog d(tplDir);
        QCOMPARE(templateCombo(d)->count(), 0);
        QCOMPARE(d.templateName(), QString());
    }

    void testNonExistentTemplatesDirectory()
    {
        NewProjectDialog d("/nonexistent/templates/path");
        QCOMPARE(templateCombo(d)->count(), 0);
    }

    void testCancelButtonRejects()
    {
        QTemporaryDir tmpDir;
        QString tplDir = createTempTemplates(tmpDir, {"article"});
        NewProjectDialog d(tplDir);
        QSignalSpy spy(&d, &QDialog::rejected);
        cancelButton(d)->click();
        QCOMPARE(spy.count(), 1);
    }

    void testCreateButtonAccepts()
    {
        QTemporaryDir tmpDir;
        QString tplDir = createTempTemplates(tmpDir, {"article"});
        NewProjectDialog d(tplDir);
        QTemporaryDir dir;
        QVERIFY(dir.isValid());

        nameEdit(d)->setText("Test");
        locationEdit(d)->setText(dir.path());

        QSignalSpy spy(&d, &QDialog::accepted);
        createButton(d)->click();
        QCOMPARE(spy.count(), 1);
    }

    void testValidationOnNameChange()
    {
        QTemporaryDir tmpDir;
        QString tplDir = createTempTemplates(tmpDir, {"article"});
        NewProjectDialog d(tplDir);
        QTemporaryDir dir;
        QVERIFY(dir.isValid());

        locationEdit(d)->setText(dir.path());
        QVERIFY(!createButton(d)->isEnabled());

        nameEdit(d)->setText("A");
        QVERIFY(createButton(d)->isEnabled());

        nameEdit(d)->setText("");
        QVERIFY(!createButton(d)->isEnabled());
    }

    void testWhitespaceOnlyNameInvalid()
    {
        QTemporaryDir tmpDir;
        QString tplDir = createTempTemplates(tmpDir, {"article"});
        NewProjectDialog d(tplDir);
        QTemporaryDir dir;
        QVERIFY(dir.isValid());

        nameEdit(d)->setText("   ");
        locationEdit(d)->setText(dir.path());

        QVERIFY(!createButton(d)->isEnabled());
    }

    void testOnlyLatexFilesScanned()
    {
        QTemporaryDir tmpDir;
        QString tplDir = tmpDir.path() + "/templates";
        QDir().mkpath(tplDir);
        // Create .latex and non-.latex files
        for (const auto &name : {"article.latex", "README.md", "notes.txt"})
        {
            QFile f(tplDir + "/" + name);
            f.open(QIODevice::WriteOnly);
            f.write("content");
            f.close();
        }
        NewProjectDialog d(tplDir);
        QCOMPARE(templateCombo(d)->count(), 1);
        QCOMPARE(templateCombo(d)->itemData(0).toString(), QString("article"));
    }
};

QTEST_MAIN(TestNewProjectDialog)
#include "test_new_project_dialog.moc"
