#include "filemanage.h"
#include <QFile>
#include <QTextStream>
#include <QFileInfo>
#include <QDir>
#include <QSettings>
#include <QMessageBox>
#include <QIcon>
#include <QDebug>

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QStringConverter>
#endif

FileManage::FileManage(QObject *parent)
    : QObject(parent)
    , m_settings(new QSettings("ink", "Settings", this))
{
    initIconMap();
    loadFileList();
}

FileManage::~FileManage()
{
    // m_settings 由Qt自动管理，因为是parent的子对象
}

bool FileManage::openFile(const QString &filePath, QPlainTextEdit *editor)
{
    if (!fileExists(filePath)) {
        qDebug() << "File does not exist:" << filePath;
        return false;
    }

    QString content = readFileContent(filePath);
    if (content.isNull()) {
        return false;
    }

    if (editor) {
        editor->setPlainText(content);
        editor->document()->setModified(false);
        editor->setProperty("filePath", filePath);
    }

    addToRecentFiles(filePath);
    emit fileOpened(filePath, content);
    return true;
}

bool FileManage::saveFile(QPlainTextEdit *editor)
{
    if (!editor) return false;

    QString filePath = editor->property("filePath").toString();
    if (filePath.isEmpty()) {
        return saveFileAs(editor);
    }

    QString content = editor->toPlainText();
    if (writeFileContent(filePath, content)) {
        editor->document()->setModified(false);
        emit fileSaved(filePath);
        return true;
    }
    return false;
}

bool FileManage::saveFileAs(QPlainTextEdit *editor)
{
    // 这个函数不应该被直接调用，因为另存为需要用户交互
    // 保存对话框应该在MainWindow中处理
    // MainWindow应该在获取用户选择的文件路径后，调用writeFileContent
    Q_UNUSED(editor);
    qDebug() << "FileManage::saveFileAs should not be called directly";
    return false;
}

QString FileManage::readFileContent(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Cannot open file for reading:" << filePath;
        return QString();
    }

    QTextStream in(&file);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    in.setCodec("UTF-8");
#else
    in.setEncoding(QStringConverter::Utf8);
#endif
    QString content = in.readAll();
    file.close();

    return content;
}

bool FileManage::writeFileContent(const QString &filePath, const QString &content)
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug() << "Cannot open file for writing:" << filePath;
        return false;
    }

    QTextStream out(&file);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    out.setCodec("UTF-8");
#else
    out.setEncoding(QStringConverter::Utf8);
#endif
    out << content;
    file.close();

    return true;
}

void FileManage::addFileToList(const QString &filePath, QTreeWidget *fileTree)
{
    if (!fileTree) return;

    QFileInfo fileInfo(filePath);
    QString displayName = fileInfo.fileName();
    QString dirPath = fileInfo.absolutePath();

    // 获取父目录项
    QTreeWidgetItem *parentItem = findOrCreateParentItem(dirPath, fileTree);

    // 检查文件是否已经存在 - 使用完整路径检查
    for (int i = 0; i < parentItem->childCount(); ++i) {
        QTreeWidgetItem *child = parentItem->child(i);
        QString storedPath = child->data(0, Qt::UserRole).toString();
        if (storedPath == filePath) {
            return;  // 已存在，不重复添加
        }
    }

    // 创建文件项
    QTreeWidgetItem *fileItem = new QTreeWidgetItem(parentItem);
    fileItem->setText(0, displayName);
    fileItem->setData(0, Qt::UserRole, filePath);  // 存储完整路径

    // 设置文件图标
    if (fileInfo.isDir()) {
        fileItem->setIcon(0, getFolderIcon());
    } else {
        fileItem->setIcon(0, getFileIcon(filePath));
    }

    // 保存映射关系
    m_filePathMap[filePath] = displayName;

    // 展开父目录
    parentItem->setExpanded(true);

    emit fileTreeUpdated();
}

QTreeWidgetItem* FileManage::findOrCreateParentItem(const QString &dirPath, QTreeWidget *fileTree)
{
    if (!fileTree) return nullptr;

    QDir dir(dirPath);
    QString relativePath = dir.absolutePath();

    QTreeWidgetItem *currentParent = fileTree->invisibleRootItem();

    QStringList pathParts;
#if defined(Q_OS_WIN)
    // Windows路径处理
    if (relativePath.contains(':')) {
        // 包含盘符，作为第一级
        QString drive = relativePath.left(relativePath.indexOf(':') + 2);
        pathParts.append(drive);
        relativePath = relativePath.mid(drive.length());
    }
    QStringList parts = relativePath.split('\\', Qt::SkipEmptyParts);
#else
    // Unix-like路径
    if (relativePath.startsWith('/')) {
        pathParts.append("/");
        relativePath = relativePath.mid(1);
    }
    QStringList parts = relativePath.split('/', Qt::SkipEmptyParts);
#endif

    pathParts.append(parts);

    // 逐级查找或创建
    for (const QString &part : pathParts) {
        if (part.isEmpty()) continue;

        bool found = false;
        for (int i = 0; i < currentParent->childCount(); ++i) {
            if (currentParent->child(i)->text(0) == part) {
                currentParent = currentParent->child(i);
                found = true;
                break;
            }
        }

        if (!found) {
            QTreeWidgetItem *newItem = new QTreeWidgetItem(currentParent);
            newItem->setText(0, part);
            newItem->setData(0, Qt::UserRole, QString()); // 目录项不存储完整路径
            newItem->setIcon(0, getFolderIcon());
            currentParent = newItem;
        }
    }

    return currentParent;
}

QTreeWidgetItem* FileManage::findFileItemInTree(QTreeWidgetItem *parent, const QString &filePath, const QString &displayName)
{
    if (!parent) return nullptr;

    // 只检查完整路径是否匹配
    QString storedPath = parent->data(0, Qt::UserRole).toString();
    if (!storedPath.isEmpty() && storedPath == filePath) {
        return parent;
    }

    // 递归查找子项
    for (int i = 0; i < parent->childCount(); ++i) {
        QTreeWidgetItem *child = parent->child(i);
        QTreeWidgetItem *result = findFileItemInTree(child, filePath, displayName);
        if (result) {
            return result;
        }
    }

    return nullptr;
}

void FileManage::removeFileFromTree(const QString &filePath, QTreeWidget *fileTree)
{
    if (!fileTree) return;

    QFileInfo fileInfo(filePath);
    QString displayName = fileInfo.fileName();

    // 使用完整路径查找并移除文件项
    QTreeWidgetItem *fileItem = findFileItemInTree(fileTree->invisibleRootItem(), filePath, displayName);
    if (fileItem) {
        QTreeWidgetItem *parent = fileItem->parent();
        if (parent) {
            parent->removeChild(fileItem);
            delete fileItem;

            // 从映射中移除
            m_filePathMap.remove(filePath);

            // 递归清理空目录
            cleanupEmptyDirectories(parent);
        }
    }

    emit fileTreeUpdated();
}

void FileManage::cleanupEmptyDirectories(QTreeWidgetItem *item)
{
    if (!item) return;

    // 如果是文件项，不处理
    QString storedPath = item->data(0, Qt::UserRole).toString();
    if (!storedPath.isEmpty()) {
        return; // 文件项
    }

    // 如果没有子项且不是根节点，移除该项
    if (item->childCount() == 0 && item->parent()) {
        QTreeWidgetItem *parent = item->parent();
        parent->removeChild(item);
        delete item;

        // 继续清理父级目录
        cleanupEmptyDirectories(parent);
    }
}

void FileManage::selectFileInTree(const QString &filePath, QTreeWidget *fileTree)
{
    if (!fileTree) return;

    QFileInfo fileInfo(filePath);
    QString displayName = fileInfo.fileName();

    // 使用完整路径查找文件项
    QTreeWidgetItem *fileItem = findFileItemInTree(fileTree->invisibleRootItem(), filePath, displayName);
    if (fileItem) {
        fileTree->setCurrentItem(fileItem);
        fileTree->scrollToItem(fileItem);

        // 展开所有父级目录
        QTreeWidgetItem *parent = fileItem->parent();
        while (parent) {
            parent->setExpanded(true);
            parent = parent->parent();
        }
    } else {
        qDebug() << "File not found in tree:" << filePath;
    }
}

void FileManage::expandAllFiles(QTreeWidget *fileTree)
{
    if (fileTree) {
        fileTree->expandAll();
    }
}

void FileManage::collapseAllFiles(QTreeWidget *fileTree)
{
    if (fileTree) {
        fileTree->collapseAll();
    }
}

void FileManage::locateCurrentFile(const QString &filePath, QTreeWidget *fileTree)
{
    if (!filePath.isEmpty() && fileTree) {
        selectFileInTree(filePath, fileTree);
    }
}

void FileManage::saveFileList(const QStringList &openFiles, const QStringList &recentFiles)
{
    // 合并现有文件列表，去重并保持最近的文件在前面
    QStringList allFiles = openFiles;
    for (const QString &file : recentFiles) {
        if (!allFiles.contains(file) && fileExists(file)) {
            allFiles.append(file);
        }
    }

    // 限制数量
    while (allFiles.size() > MAX_RECENT_FILES) {
        allFiles.removeLast();
    }

    // 保存
    m_settings->setValue("recentFiles", allFiles);
    m_recentFiles = allFiles;

    qDebug() << "Saved files:" << allFiles.size();
    emit recentFilesUpdated(allFiles);
}

QStringList FileManage::loadFileList()
{
    m_recentFiles = m_settings->value("recentFiles").toStringList();
    qDebug() << "Loading files:" << m_recentFiles.size();
    emit recentFilesUpdated(m_recentFiles);
    return m_recentFiles;
}

void FileManage::addToRecentFiles(const QString &filePath)
{
    m_recentFiles.removeAll(filePath);
    m_recentFiles.prepend(filePath);

    // 限制数量
    while (m_recentFiles.size() > MAX_RECENT_FILES) {
        m_recentFiles.removeLast();
    }

    emit recentFilesUpdated(m_recentFiles);
}

void FileManage::removeFromRecentFiles(const QString &filePath)
{
    m_recentFiles.removeAll(filePath);
    emit recentFilesUpdated(m_recentFiles);
}

void FileManage::initIconMap()
{
    // 清空现有映射
    m_iconMap.clear();
    
    // 这里可以添加特定文件类型的图标映射
    // 例如：m_iconMap[".txt"] = ":/icons/text";
    // 目前所有文件类型都使用默认文件图标
}

QIcon FileManage::getFileIcon(const QString &filePath) const
{
    QFileInfo fileInfo(filePath);
    QString suffix = fileInfo.suffix().toLower();
    
    // 根据后缀返回不同的图标
    if (m_iconMap.contains("." + suffix)) {
        return QIcon(m_iconMap["." + suffix]);
    }
    
    // 默认文件图标
    return QIcon(":/icons/file");
}

QIcon FileManage::getFolderIcon() const
{
    return QIcon(":/icons/folder");
}

QString FileManage::removeFileExtension(const QString &fileName) const
{
    int dotIndex = fileName.lastIndexOf('.');
    if (dotIndex != -1) {
        return fileName.left(dotIndex);
    }
    return fileName;
}

QString FileManage::getFileNameFromPath(const QString &filePath) const
{
    QFileInfo fileInfo(filePath);
    return fileInfo.fileName();
}

bool FileManage::fileExists(const QString &filePath) const
{
    return QFile::exists(filePath);
}

bool FileManage::isFileOpened(const QString &filePath) const
{
    return m_openFiles.contains(filePath);
}

bool FileManage::hasUnsavedChanges(QPlainTextEdit *editor) const
{
    if (!editor) return false;
    return editor->document()->isModified();
}

bool FileManage::maybeSave(QPlainTextEdit *editor)
{
    if (!editor || !editor->document()->isModified()) {
        return true;
    }

    // 具体的保存对话框应该在MainWindow中处理
    // 这里只返回是否需要保存
    return true;
}
