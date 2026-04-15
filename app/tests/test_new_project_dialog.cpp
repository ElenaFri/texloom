#include <QtTest/QtTest>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QTemporaryDir>
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

private slots:
    void testDialogTitle()
    {
        NewProjectDialog d;
        QCOMPARE(d.windowTitle(), tr("New Project"));
    }

    void testFieldsExist()
    {
        NewProjectDialog d;
        QVERIFY(nameEdit(d) != nullptr);
        QVERIFY(locationEdit(d) != nullptr);
        QVERIFY(templateCombo(d) != nullptr);
        QVERIFY(createButton(d) != nullptr);
        QVERIFY(cancelButton(d) != nullptr);
    }

    void testCreateButtonDisabledByDefault()
    {
        NewProjectDialog d;
        QVERIFY(!createButton(d)->isEnabled());
    }

    void testCreateButtonEnabledWithValidInput()
    {
        NewProjectDialog d;
        QTemporaryDir dir;
        QVERIFY(dir.isValid());

        nameEdit(d)->setText("MyProject");
        locationEdit(d)->setText(dir.path());

        QVERIFY(createButton(d)->isEnabled());
    }

    void testCreateButtonDisabledWithEmptyName()
    {
        NewProjectDialog d;
        QTemporaryDir dir;
        QVERIFY(dir.isValid());

        nameEdit(d)->setText("");
        locationEdit(d)->setText(dir.path());

        QVERIFY(!createButton(d)->isEnabled());
    }

    void testCreateButtonDisabledWithEmptyLocation()
    {
        NewProjectDialog d;

        nameEdit(d)->setText("MyProject");
        locationEdit(d)->setText("");

        QVERIFY(!createButton(d)->isEnabled());
    }

    void testCreateButtonDisabledWithInvalidLocation()
    {
        NewProjectDialog d;

        nameEdit(d)->setText("MyProject");
        locationEdit(d)->setText("/nonexistent/path/that/does/not/exist");

        QVERIFY(!createButton(d)->isEnabled());
    }

    void testProjectNameAccessor()
    {
        NewProjectDialog d;
        nameEdit(d)->setText("  Test Project  ");
        QCOMPARE(d.projectName(), QString("Test Project"));
    }

    void testProjectLocationAccessor()
    {
        NewProjectDialog d;
        QTemporaryDir dir;
        QVERIFY(dir.isValid());

        locationEdit(d)->setText(dir.path());
        QCOMPARE(d.projectLocation(), dir.path());
    }

    void testTemplateOptions()
    {
        NewProjectDialog d;
        auto *combo = templateCombo(d);
        QCOMPARE(combo->count(), 3);
        QCOMPARE(combo->itemData(0).toString(), QString("article"));
        QCOMPARE(combo->itemData(1).toString(), QString("report"));
        QCOMPARE(combo->itemData(2).toString(), QString("thesis"));
    }

    void testDefaultTemplate()
    {
        NewProjectDialog d;
        QCOMPARE(d.templateName(), QString("article"));
    }

    void testTemplateSelection()
    {
        NewProjectDialog d;
        templateCombo(d)->setCurrentIndex(2);
        QCOMPARE(d.templateName(), QString("thesis"));
    }

    void testCancelButtonRejects()
    {
        NewProjectDialog d;
        QSignalSpy spy(&d, &QDialog::rejected);
        cancelButton(d)->click();
        QCOMPARE(spy.count(), 1);
    }

    void testCreateButtonAccepts()
    {
        NewProjectDialog d;
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
        NewProjectDialog d;
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
        NewProjectDialog d;
        QTemporaryDir dir;
        QVERIFY(dir.isValid());

        nameEdit(d)->setText("   ");
        locationEdit(d)->setText(dir.path());

        QVERIFY(!createButton(d)->isEnabled());
    }
};

QTEST_MAIN(TestNewProjectDialog)
#include "test_new_project_dialog.moc"
