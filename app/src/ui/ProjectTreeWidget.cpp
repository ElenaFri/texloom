#include "ProjectTreeWidget.h"
#include <QFileInfo>
#include <QStyle>

namespace texloom
{

    ProjectTreeWidget::ProjectTreeWidget(QWidget *parent)
        : QTreeWidget(parent)
    {
        setHeaderLabel(tr("Project"));
        setRootIsDecorated(false);
        setIndentation(10); // Slight indentation to visually distinguish child files from the project root

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

        fileItem->setIcon(0, style()->standardIcon(QStyle::SP_FileIcon));
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
