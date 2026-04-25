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
class FileManage;

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

    // 右键菜单槽函数
    void showFileTreeContextMenu(const QPoint &pos);
    void showEditorContextMenu(const QPoint &pos);
    void removeFileFromTree();
    void openFileLocation();
    void openCmdAtFile();
    void openGitBashAtFile();

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
    bool maybeSave();
    QString getCurrentFilePath() const;
    QPlainTextEdit* getCurrentEditor() const;
    QString getSelectedFilePath() const;
    bool isExecutableFile(const QString &filePath) const;

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

    // 菜单
    QMenu *fileMenu;
    QMenu *editMenu;
    QMenu *viewMenu;
    QMenu *helpMenu;

    // 上下文菜单
    QMenu *fileTreeContextMenu;
    QMenu *editorContextMenu;

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

    // 右键菜单动作
    QAction *removeFileAct;
    QAction *openFileLocationAct;
    QAction *openCmdAct;
    QAction *openGitBashAct;

    FindReplaceDialog *m_findReplaceDialog;
    FileManage *m_fileManage;

    QAction *aboutAct;

    // 状态栏标签
    QLabel *statusPosition;
    QLabel *statusFileInfo;

    // 状态
    bool previewVisible;

    // 上下文菜单状态
    QString m_contextMenuFilePath;
};

#endif // MAINWINDOW_H
