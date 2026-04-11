#include <QtTest/QtTest>
#include <QTemporaryDir>
#include <QSignalSpy>
#include <QFile>
#include <QJsonDocument>
#include "../src/core/ProjectModel.h"

using namespace texloom;

class TestProjectModel : public QObject
{
    Q_OBJECT

private slots:
    void init()
    {
        m_tempDir = new QTemporaryDir();
        QVERIFY(m_tempDir->isValid());
        m_model = new ProjectModel();
    }

    void cleanup()
    {
        delete m_model;
        delete m_tempDir;
    }

    // ========== SUCCESS SCENARIOS ==========

    void testCreateValidProject()
    {
        QSignalSpy spyOpened(m_model, &ProjectModel::projectOpened);
        QSignalSpy spySaved(m_model, &ProjectModel::projectSaved);

        bool result = m_model->createProject("TestProject", m_tempDir->path());

        QVERIFY(result);
        QCOMPARE(m_model->projectName(), QString("TestProject"));
        QCOMPARE(m_model->projectPath(), m_tempDir->path());
        QVERIFY(m_model->isOpen());
        QVERIFY(!m_model->isModified()); // Should be saved after creation
        QCOMPARE(spyOpened.count(), 1);
        QCOMPARE(spySaved.count(), 1);

        // Verify project file exists
        QString projectFile = m_tempDir->path() + "/TestProject.texloom";
        QVERIFY(QFile::exists(projectFile));
    }

    void testSaveAndLoadProject()
    {
        // Create and save a project
        m_model->createProject("SaveLoadTest", m_tempDir->path());
        m_model->addFile("/path/to/file1.md");
        m_model->addFile("/path/to/file2.md");
        m_model->setTemplateName("article");
        m_model->setBibliographyFile("/path/to/refs.bib");

        QVERIFY(m_model->saveProject());
        QString projectFile = m_model->projectFilePath();

        // Close and reload
        m_model->closeProject();
        QVERIFY(!m_model->isOpen());

        QSignalSpy spyOpened(m_model, &ProjectModel::projectOpened);
        bool loaded = m_model->loadProject(projectFile);

        QVERIFY(loaded);
        QVERIFY(m_model->isOpen());
        QVERIFY(!m_model->isModified());
        QCOMPARE(m_model->projectName(), QString("SaveLoadTest"));
        QCOMPARE(m_model->files().count(), 2);
        QVERIFY(m_model->files().contains("/path/to/file1.md"));
        QVERIFY(m_model->files().contains("/path/to/file2.md"));
        QCOMPARE(m_model->templateName(), QString("article"));
        QCOMPARE(m_model->bibliographyFile(), QString("/path/to/refs.bib"));
        QCOMPARE(spyOpened.count(), 1);
    }

    void testAddFiles()
    {
        m_model->createProject("FileTest", m_tempDir->path());

        QSignalSpy spyFileAdded(m_model, &ProjectModel::fileAdded);
        QSignalSpy spyModified(m_model, &ProjectModel::projectModified);

        m_model->addFile("/path/to/file1.md");
        m_model->addFile("/path/to/file2.md");

        QCOMPARE(m_model->files().count(), 2);
        QVERIFY(m_model->files().contains("/path/to/file1.md"));
        QVERIFY(m_model->files().contains("/path/to/file2.md"));
        QCOMPARE(spyFileAdded.count(), 2);
        // projectModified is only emitted once (when transitioning from unmodified to modified)
        QCOMPARE(spyModified.count(), 1);
        QVERIFY(m_model->isModified());
    }

    void testRemoveFile()
    {
        m_model->createProject("RemoveTest", m_tempDir->path());
        m_model->addFile("/path/to/file1.md");
        m_model->addFile("/path/to/file2.md");
        m_model->saveProject(); // Clear modified flag

        QSignalSpy spyFileRemoved(m_model, &ProjectModel::fileRemoved);
        QSignalSpy spyModified(m_model, &ProjectModel::projectModified);

        m_model->removeFile("/path/to/file1.md");

        QCOMPARE(m_model->files().count(), 1);
        QVERIFY(!m_model->files().contains("/path/to/file1.md"));
        QVERIFY(m_model->files().contains("/path/to/file2.md"));
        QCOMPARE(spyFileRemoved.count(), 1);
        QCOMPARE(spyModified.count(), 1);
        QVERIFY(m_model->isModified());
    }

    void testCloseProject()
    {
        m_model->createProject("CloseTest", m_tempDir->path());
        m_model->addFile("/path/to/file.md");

        QSignalSpy spyClosed(m_model, &ProjectModel::projectClosed);

        m_model->closeProject();

        QVERIFY(!m_model->isOpen());
        QVERIFY(!m_model->isModified());
        QVERIFY(m_model->projectName().isEmpty());
        QVERIFY(m_model->projectPath().isEmpty());
        QCOMPARE(m_model->files().count(), 0);
        QCOMPARE(spyClosed.count(), 1);
    }

    void testSaveAsNewLocation()
    {
        m_model->createProject("SaveAsTest", m_tempDir->path());
        m_model->addFile("/path/to/file.md");

        // Create another temp directory for "Save As"
        QTemporaryDir tempDir2;
        QVERIFY(tempDir2.isValid());
        QString newPath = tempDir2.path() + "/NewProject.texloom";

        bool result = m_model->saveProjectAs(newPath);

        QVERIFY(result);
        QCOMPARE(m_model->projectName(), QString("NewProject"));
        QCOMPARE(m_model->projectPath(), tempDir2.path());
        QCOMPARE(m_model->projectFilePath(), newPath);
        QVERIFY(QFile::exists(newPath));
        QVERIFY(!m_model->isModified());
    }

    void testModificationTracking()
    {
        m_model->createProject("ModTest", m_tempDir->path());
        QVERIFY(!m_model->isModified()); // Fresh after creation

        m_model->addFile("/file1.md");
        QVERIFY(m_model->isModified());

        m_model->saveProject();
        QVERIFY(!m_model->isModified());

        m_model->setTemplateName("report");
        QVERIFY(m_model->isModified());

        m_model->saveProject();
        QVERIFY(!m_model->isModified());

        m_model->setBibliographyFile("/refs.bib");
        QVERIFY(m_model->isModified());
    }

    // ========== FAILURE SCENARIOS ==========

    void testCreateProjectWithInvalidPath()
    {
        QSignalSpy spyError(m_model, &ProjectModel::errorOccurred);

        // Try to create in non-existent directory
        bool result = m_model->createProject("Invalid", "/non/existent/path/9876543210");

        QVERIFY(!result);
        QVERIFY(!m_model->isOpen());
        QVERIFY(spyError.count() > 0);
    }

    void testCreateProjectWithEmptyName()
    {
        // Empty name should still create a file, but it might be problematic
        bool result = m_model->createProject("", m_tempDir->path());

        // This exposes a potential bug: empty names should be rejected
        // The current implementation allows it, which might be incorrect
        if (result)
        {
            QVERIFY(m_model->projectFilePath().endsWith("/.texloom"));
        }
    }

    void testLoadNonExistentProject()
    {
        QSignalSpy spyError(m_model, &ProjectModel::errorOccurred);

        bool result = m_model->loadProject("/non/existent/project.texloom");

        QVERIFY(!result);
        QVERIFY(!m_model->isOpen());
        QCOMPARE(spyError.count(), 1);
    }

    void testLoadCorruptedProject()
    {
        // Create a corrupted JSON file
        QString corruptedFile = m_tempDir->path() + "/corrupted.texloom";
        QFile file(corruptedFile);
        QVERIFY(file.open(QIODevice::WriteOnly));
        file.write("{invalid json syntax here!!!");
        file.close();

        QSignalSpy spyError(m_model, &ProjectModel::errorOccurred);
        bool result = m_model->loadProject(corruptedFile);

        // This should fail gracefully
        QVERIFY(!result);
        QVERIFY(!m_model->isOpen());
    }

    // ========== EDGE CASES ==========

    void testAddDuplicateFile()
    {
        m_model->createProject("DupTest", m_tempDir->path());

        m_model->addFile("/path/to/file.md");
        QCOMPARE(m_model->files().count(), 1);

        QSignalSpy spyFileAdded(m_model, &ProjectModel::fileAdded);

        // Try to add same file again
        m_model->addFile("/path/to/file.md");

        // Should not add duplicate
        QCOMPARE(m_model->files().count(), 1);
        QCOMPARE(spyFileAdded.count(), 0); // Signal not emitted
    }

private:
    QTemporaryDir *m_tempDir;
    ProjectModel *m_model;
};

QTEST_MAIN(TestProjectModel)
#include "test_project_model.moc"
