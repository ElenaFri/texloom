#include "ProjectTreeWidget.h"
#include <QFileInfo>
#include <QStyle>

namespace texloom
{

    ProjectTreeWidget::ProjectTreeWidget(QWidget *parent)
        : QTreeWidget(parent)
    {
        setHeaderLabel(tr("Project"));
        setColumnCount(1);
        setRootIsDecorated(false);
        setIndentation(14);

        connect(this, &QTreeWidget::itemClicked,
                this, &ProjectTreeWidget::onItemClicked);
        connect(this, &QTreeWidget::itemDoubleClicked,
                this, &ProjectTreeWidget::onItemDoubleClicked);
    }

    void ProjectTreeWidget::setProjectRoot(const QString &name, const QString &path)
    {
        clear();

        m_projectPath = path;
        m_rootItem = new QTreeWidgetItem(this);
        m_rootItem->setText(0, name);
        m_rootItem->setIcon(0, style()->standardIcon(QStyle::SP_DirIcon));
        m_rootItem->setExpanded(true);

        addTopLevelItem(m_rootItem);
    }

    void ProjectTreeWidget::addFile(const QString &filePath)
    {
        if (!m_rootItem)
        {
            return;
        }

        QFileInfo fileInfo(filePath);
        QTreeWidgetItem *fileItem = new QTreeWidgetItem(m_rootItem);
        fileItem->setText(0, fileInfo.fileName());
        fileItem->setData(0, Qt::UserRole, filePath);

        QIcon fileIcon;
        if (fileInfo.suffix().compare("md", Qt::CaseInsensitive) == 0)
        {
            fileIcon = style()->standardIcon(QStyle::SP_FileIcon);
        }
        else if (fileInfo.suffix().compare("tex", Qt::CaseInsensitive) == 0)
        {
            fileIcon = style()->standardIcon(QStyle::SP_FileIcon);
        }
        else if (fileInfo.suffix().compare("bib", Qt::CaseInsensitive) == 0)
        {
            fileIcon = style()->standardIcon(QStyle::SP_FileIcon);
        }
        else
        {
            fileIcon = style()->standardIcon(QStyle::SP_FileIcon);
        }

        fileItem->setIcon(0, fileIcon);
    }

    void ProjectTreeWidget::removeFile(const QString &filePath)
    {
        if (!m_rootItem)
        {
            return;
        }

        for (int i = 0; i < m_rootItem->childCount(); ++i)
        {
            QTreeWidgetItem *item = m_rootItem->child(i);
            if (item->data(0, Qt::UserRole).toString() == filePath)
            {
                delete m_rootItem->takeChild(i);
                break;
            }
        }
    }

    void ProjectTreeWidget::clear()
    {
        QTreeWidget::clear();
        m_rootItem = nullptr;
        m_projectPath.clear();
    }

    void ProjectTreeWidget::onItemClicked(QTreeWidgetItem *item, int column)
    {
        Q_UNUSED(column);

        QString filePath = item->data(0, Qt::UserRole).toString();
        if (!filePath.isEmpty())
        {
            emit fileSelected(filePath);
        }
    }

    void ProjectTreeWidget::onItemDoubleClicked(QTreeWidgetItem *item, int column)
    {
        Q_UNUSED(column);

        QString filePath = item->data(0, Qt::UserRole).toString();
        if (!filePath.isEmpty())
        {
            emit fileDoubleClicked(filePath);
        }
    }

} // namespace texloom
