#ifndef FILEMANAGE_H
#define FILEMANAGE_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QMap>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QPlainTextEdit>

// 前置声明
class QSettings;

class FileManage : public QObject
{
    Q_OBJECT

public:
    explicit FileManage(QObject *parent = nullptr);
    ~FileManage();

    // 文件读写操作
    bool openFile(const QString &filePath, QPlainTextEdit *editor = nullptr);
    bool saveFile(QPlainTextEdit *editor);
    bool saveFileAs(QPlainTextEdit *editor);
    QString readFileContent(const QString &filePath);
    bool writeFileContent(const QString &filePath, const QString &content);

    // 文件树管理
    void addFileToList(const QString &filePath, QTreeWidget *fileTree);
    QTreeWidgetItem* findOrCreateParentItem(const QString &dirPath, QTreeWidget *fileTree);
    QTreeWidgetItem* findFileItemInTree(QTreeWidgetItem *parent, const QString &filePath, const QString &displayName);
    void removeFileFromTree(const QString &filePath, QTreeWidget *fileTree);
    void cleanupEmptyDirectories(QTreeWidgetItem *item);
    void selectFileInTree(const QString &filePath, QTreeWidget *fileTree);
    void expandAllFiles(QTreeWidget *fileTree);
    void collapseAllFiles(QTreeWidget *fileTree);
    void locateCurrentFile(const QString &filePath, QTreeWidget *fileTree);

    // 文件列表持久化
    void saveFileList(const QStringList &openFiles, const QStringList &recentFiles);
    QStringList loadFileList();
    QStringList getRecentFiles() const { return m_recentFiles; }
    void addToRecentFiles(const QString &filePath);
    void removeFromRecentFiles(const QString &filePath);

    // 图标管理
    void initIconMap();
    QIcon getFileIcon(const QString &filePath) const;
    QIcon getFolderIcon() const;

    // 工具函数
    QString removeFileExtension(const QString &fileName) const;
    QString getFileNameFromPath(const QString &filePath) const;
    bool fileExists(const QString &filePath) const;
    bool isFileOpened(const QString &filePath) const;

    // 状态管理
    bool hasUnsavedChanges(QPlainTextEdit *editor) const;
    bool maybeSave(QPlainTextEdit *editor);

    // 最近文件列表最大数量
    static const int MAX_RECENT_FILES = 100;

signals:
    void fileOpened(const QString &filePath, const QString &content);
    void fileSaved(const QString &filePath);
    void fileClosed(const QString &filePath);
    void recentFilesUpdated(const QStringList &recentFiles);
    void fileTreeUpdated();

private:
    // 最近打开的文件列表
    QStringList m_recentFiles;

    // 文件图标映射
    QMap<QString, QString> m_iconMap;

    // 文件路径映射：显示名 -> 完整路径
    QMap<QString, QString> m_filePathMap;

    // 当前打开的文件
    QStringList m_openFiles;

    // 设置
    QSettings *m_settings;
};

#endif // FILEMANAGE_H
