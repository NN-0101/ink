#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTreeWidget>
#include <QTextEdit>
#include <QSplitter>
#include <QMap>
#include <QString>
#include <QMenuBar>
#include <QToolBar>
#include <QAction>
#include <QActionGroup>

/**
 * @brief 主窗口类，实现文本编辑器界面
 * 
 * 这个类实现了文本编辑器的核心界面，包括：
 * 1. 左侧文件树：显示打开的文件，按文件夹分组
 * 2. 右侧文本编辑区域：用于编辑文件内容
 * 3. 分割器：允许用户调整左右面板大小
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param parent 父窗口指针
     */
    MainWindow(QWidget *parent = nullptr);
    
    /**
     * @brief 析构函数
     */
    ~MainWindow();

private slots:
    /**
     * @brief 文件树项点击槽函数
     * @param item 被点击的树项
     * @param column 点击的列
     */
    void onFileItemClicked(QTreeWidgetItem *item, int column);
    
    /**
     * @brief 更新文件树槽函数
     */
    void updateFileTree();

private:
    /**
     * @brief 初始化用户界面
     */
    void setupUI();
    
    /**
     * @brief 创建菜单栏
     */
    void createMenuBar();
    
    /**
     * @brief 创建工具栏
     */
    void createToolBar();
    
    /**
     * @brief 创建文件树，添加示例文件
     */
    void createFileTree();
    
    /**
     * @brief 添加文件到文件树
     * @param filePath 文件路径
     */
    void addFileToTree(const QString &filePath);
    
    /**
     * @brief 查找或创建文件夹项
     * @param folderPath 文件夹路径
     * @return 文件夹树项指针
     */
    QTreeWidgetItem* findOrCreateFolderItem(const QString &folderPath);

    // UI 组件
    QSplitter *m_splitter;              ///< 分割器，分隔左右面板
    QTreeWidget *m_fileTree;            ///< 文件树控件，显示打开的文件
    QTextEdit *m_textEdit;              ///< 文本编辑控件，用于编辑文件内容
    
    // 菜单栏相关
    QMenuBar *m_menuBar;                ///< 菜单栏
    QMenu *m_fileMenu;                  ///< 文件菜单
    QMenu *m_editMenu;                  ///< 编辑菜单
    QMenu *m_viewMenu;                  ///< 视图菜单
    QMenu *m_helpMenu;                  ///< 帮助菜单
    
    // 工具栏相关
    QToolBar *m_mainToolBar;            ///< 主工具栏
    QToolBar *m_formatToolBar;          ///< 格式工具栏
    
    // 动作
    QAction *m_newAction;               ///< 新建文件动作
    QAction *m_openAction;              ///< 打开文件动作
    QAction *m_saveAction;              ///< 保存文件动作
    QAction *m_saveAsAction;            ///< 另存为动作
    QAction *m_printAction;             ///< 打印动作
    QAction *m_exitAction;              ///< 退出动作
    
    QAction *m_undoAction;              ///< 撤销动作
    QAction *m_redoAction;              ///< 重做动作
    QAction *m_cutAction;               ///< 剪切动作
    QAction *m_copyAction;              ///< 复制动作
    QAction *m_pasteAction;             ///< 粘贴动作
    QAction *m_findAction;              ///< 查找动作
    QAction *m_replaceAction;           ///< 替换动作
    QAction *m_selectAllAction;         ///< 全选动作
    
    QAction *m_wordWrapAction;          ///< 自动换行动作
    QAction *m_showToolBarAction;       ///< 显示工具栏动作
    QAction *m_showStatusBarAction;     ///< 显示状态栏动作
    
    QAction *m_aboutAction;             ///< 关于动作
    QAction *m_aboutQtAction;           ///< 关于Qt动作
    
    // 数据
    QMap<QString, QString> m_openFiles; ///< 打开的文件映射：文件路径 -> 文件内容
};
#endif // MAINWINDOW_H
