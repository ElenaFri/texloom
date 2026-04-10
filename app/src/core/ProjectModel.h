#pragma once

#include <QObject>
#include <QString>
#include <QStringList>
#include <QJsonObject>

namespace texloom
{

    /**
     * @brief Core data model for a TexLoom project
     *
     * Manages project state including file list, settings, and metadata.
     * Uses signals to notify UI components of changes.
     */
    class ProjectModel : public QObject
    {
        Q_OBJECT

    public:
        explicit ProjectModel(QObject *parent = nullptr);
        ~ProjectModel() override = default;

        // Non-copyable and non-movable
        ProjectModel(const ProjectModel &) = delete;
        ProjectModel &operator=(const ProjectModel &) = delete;
        ProjectModel(ProjectModel &&) = delete;
        ProjectModel &operator=(ProjectModel &&) = delete;

        // Project management
        bool createProject(const QString &name, const QString &path);
        bool loadProject(const QString &projectFilePath);
        bool saveProject();
        bool saveProjectAs(const QString &newPath);
        void closeProject();

        // File management
        void addFile(const QString &filePath);
        void removeFile(const QString &filePath);
        QStringList files() const;

        // Project properties
        [[nodiscard]] QString projectName() const noexcept { return m_projectName; }
        [[nodiscard]] QString projectPath() const noexcept { return m_projectPath; }
        [[nodiscard]] QString projectFilePath() const noexcept { return m_projectFilePath; }
        [[nodiscard]] bool isModified() const noexcept { return m_isModified; }
        [[nodiscard]] bool isOpen() const noexcept { return m_isOpen; }

        // Settings
        [[nodiscard]] QString templateName() const noexcept { return m_templateName; }
        void setTemplateName(const QString &name);

        [[nodiscard]] QString bibliographyFile() const noexcept { return m_bibliographyFile; }
        void setBibliographyFile(const QString &path);

    signals:
        void projectOpened(const QString &projectPath);
        void projectClosed();
        void projectModified();
        void projectSaved();

        void fileAdded(const QString &filePath);
        void fileRemoved(const QString &filePath);

        void errorOccurred(const QString &message);

    private:
        // Serialization
        QJsonObject toJson() const;
        bool fromJson(const QJsonObject &json);

        // Project state
        QString m_projectName;
        QString m_projectPath;     // Directory containing the project
        QString m_projectFilePath; // Path to .texloom file
        QStringList m_files;       // Markdown files in project
        bool m_isModified = false;
        bool m_isOpen = false;

        // Settings
        QString m_templateName;
        QString m_bibliographyFile;

        void setModified();
    };

} // namespace texloom
