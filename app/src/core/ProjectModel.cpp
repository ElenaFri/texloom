#include "ProjectModel.h"
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QJsonDocument>
#include <QJsonArray>

namespace texloom
{

    ProjectModel::ProjectModel(QObject *parent)
        : QObject(parent)
    {
    }

    bool ProjectModel::createProject(const QString &name, const QString &path)
    {
        if (m_isOpen)
        {
            emit errorOccurred("Close current project before creating a new one");
            return false;
        }

        m_projectName = name;
        m_projectPath = path;
        m_projectFilePath = path + "/" + name + ".texloom";
        m_files.clear();
        m_isOpen = true;
        m_isModified = true;

        if (!saveProject())
        {
            closeProject();
            return false;
        }

        emit projectOpened(m_projectPath);
        return true;
    }

    bool ProjectModel::loadProject(const QString &projectFilePath)
    {
        QFile file(projectFilePath);
        if (!file.open(QIODevice::ReadOnly))
        {
            emit errorOccurred("Cannot open project file: " + projectFilePath);
            return false;
        }

        QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
        file.close();

        if (!fromJson(doc.object()))
        {
            return false;
        }

        m_projectFilePath = projectFilePath;
        QFileInfo fileInfo(projectFilePath);
        m_projectPath = fileInfo.absolutePath();
        m_isOpen = true;
        m_isModified = false;

        emit projectOpened(m_projectPath);
        return true;
    }

    bool ProjectModel::saveProject()
    {
        if (!m_isOpen)
        {
            return false;
        }

        QFile file(m_projectFilePath);
        if (!file.open(QIODevice::WriteOnly))
        {
            emit errorOccurred("Cannot save project file: " + m_projectFilePath);
            return false;
        }

        QJsonDocument doc(toJson());
        file.write(doc.toJson(QJsonDocument::Indented));
        file.close();

        m_isModified = false;
        emit projectSaved();
        return true;
    }

    bool ProjectModel::saveProjectAs(const QString &newPath)
    {
        m_projectFilePath = newPath;
        QFileInfo fileInfo(newPath);
        m_projectPath = fileInfo.absolutePath();
        m_projectName = fileInfo.baseName();

        return saveProject();
    }

    void ProjectModel::closeProject()
    {
        m_projectName.clear();
        m_projectPath.clear();
        m_projectFilePath.clear();
        m_files.clear();
        m_templateName.clear();
        m_bibliographyFile.clear();
        m_isModified = false;
        m_isOpen = false;

        emit projectClosed();
    }

    void ProjectModel::addFile(const QString &filePath)
    {
        if (!m_files.contains(filePath))
        {
            m_files.append(filePath);
            setModified();
            emit fileAdded(filePath);
        }
    }

    void ProjectModel::removeFile(const QString &filePath)
    {
        if (m_files.removeOne(filePath))
        {
            setModified();
            emit fileRemoved(filePath);
        }
    }

    QStringList ProjectModel::files() const
    {
        return m_files;
    }

    void ProjectModel::setTemplateName(const QString &name)
    {
        if (m_templateName != name)
        {
            m_templateName = name;
            setModified();
        }
    }

    void ProjectModel::setBibliographyFile(const QString &path)
    {
        if (m_bibliographyFile != path)
        {
            m_bibliographyFile = path;
            setModified();
        }
    }

    void ProjectModel::setModified()
    {
        if (!m_isModified)
        {
            m_isModified = true;
            emit projectModified();
        }
    }

    QJsonObject ProjectModel::toJson() const
    {
        QJsonObject json;
        json["name"] = m_projectName;
        json["version"] = "1.0";

        QJsonArray filesArray;
        for (const QString &file : m_files)
        {
            filesArray.append(file);
        }
        json["files"] = filesArray;

        if (!m_templateName.isEmpty())
        {
            json["template"] = m_templateName;
        }

        if (!m_bibliographyFile.isEmpty())
        {
            json["bibliography"] = m_bibliographyFile;
        }

        return json;
    }

    bool ProjectModel::fromJson(const QJsonObject &json)
    {
        if (!json.contains("name") || !json.contains("files"))
        {
            emit errorOccurred("Invalid project file format");
            return false;
        }

        m_projectName = json["name"].toString();

        m_files.clear();
        QJsonArray filesArray = json["files"].toArray();
        for (const QJsonValue &value : filesArray)
        {
            m_files.append(value.toString());
        }

        m_templateName = json.value("template").toString();
        m_bibliographyFile = json.value("bibliography").toString();

        return true;
    }

} // namespace texloom
