#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QShortcut>

// 前置声明
class QSplitter;
class QTreeWidget;
class QTreeWidgetItem;
class QTabWidget;
class QPlainTextEdit;
class QTextEdit;
class QMenu;
class QAction;
class QToolBar;
class QStatusBar;
class QLabel;
class QWidget;
class QPushButton;
class LineNumberEditor;
class FindReplaceDialog;

class MainWindow : public QMainWindow
{
    Q_OBJECT

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    // 右键打开文件
    void openFileFromCommandLine(const QString &filePath);

private slots:
    // 文件操作
    void newFile();
    bool saveFile();
    bool saveFileAs();
    void closeTab(int index = -1);

    // 编辑操作
    void undo();
    void redo();
    void cut();
    void copy();
    void paste();

    // 视图操作
    void toggleFileList(bool visible);
    void togglePreview(bool visible);

    // 文件列表工具栏操作
    void locateCurrentFile();
    void expandAllFiles();
    void collapseAllFiles();

    // 其他槽函数
    void onFileItemClicked(QTreeWidgetItem *item, int column);
    void onTextChanged();
    void updateWindowTitle();
    void updateStatusBar();
    void currentTabChanged(int index);
    void updateEditActions();
    void showAboutDialog();

    // 工具函数
    void updateButtonStates();

    void showFindDialog();
    void showReplaceDialog();

    // 文件列表持久化
    void saveFileList();
    void loadFileList();

private:
    // 初始化函数
    void initUI();
    void initMenuBar();
    void initToolBar();
    void initStatusBar();
    void initFileList();
    void initConnections();

    // 工具函数
    QPlainTextEdit* createEditor();
    void openFileFromPath(const QString &filePath);
    void addFileToList(const QString &filePath);
    bool maybeSave();
    QString getCurrentFilePath() const;
    QPlainTextEdit* getCurrentEditor() const;
    QString removeFileExtension(const QString &fileName);

    // 文件树相关函数
    QTreeWidgetItem* findOrCreateParentItem(const QString &dirPath);
    void selectFileInTree(const QString &filePath);
    void removeFileFromTree(const QString &filePath);

    // 文件树辅助函数
    QTreeWidgetItem* findFileItemInTree(QTreeWidgetItem *parent, const QString &filePath, const QString &displayName);
    void cleanupEmptyDirectories(QTreeWidgetItem *item);

    // 初始化图标映射
    void initIconMap();

    // UI 组件
    QSplitter *mainSplitter;
    QWidget *leftPanel;
    QTreeWidget *fileTree;
    QTabWidget *tabWidget;
    QTextEdit *previewPanel;

    // 文件列表工具栏
    QPushButton *locateBtn;
    QPushButton *expandBtn;
    QPushButton *foldBtn;
    QPushButton *addFileBtn;
    QPushButton *openFileBtn;

    // 菜单
    QMenu *fileMenu;
    QMenu *editMenu;
    QMenu *viewMenu;
    QMenu *helpMenu;

    // 动作
    QAction *newAct;
    QAction *openAct;
    QAction *saveAct;
    QAction *saveAsAct;
    QAction *closeAct;
    QAction *exitAct;

    QAction *undoAct;
    QAction *redoAct;
    QAction *cutAct;
    QAction *copyAct;
    QAction *pasteAct;

    QAction *toggleFileListAct;
    QAction *togglePreviewAct;

    FindReplaceDialog *m_findReplaceDialog;

    QAction *aboutAct;

    // 状态栏标签
    QLabel *statusPosition;
    QLabel *statusFileInfo;

    // 状态
    bool previewVisible;

    // 文件路径映射：显示名 -> 完整路径
    QMap<QString, QString> filePathMap;

    // 文件图标映射
    QMap<QString, QString> iconMap;

    // 最近打开的文件列表
    QStringList m_recentFiles;
    // 最大保存文件数量
    static const int MAX_RECENT_FILES = 100;
};

#endif // MAINWINDOW_H
