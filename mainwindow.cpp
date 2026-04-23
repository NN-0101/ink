#include "mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QMessageBox>
#include <QHeaderView>
#include <QFont>
#include <QFontDatabase>
#include <QApplication>
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QKeySequence>
#include <QIcon>

/**
 * @brief 主窗口构造函数
 * @param parent 父窗口指针
 * 
 * 构造函数初始化界面，设置窗口标题和大小。
 * 调用 setupUI() 创建界面，createFileTree() 添加示例文件。
 */
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_splitter(nullptr)
    , m_fileTree(nullptr)
    , m_textEdit(nullptr)
    , m_menuBar(nullptr)
    , m_fileMenu(nullptr)
    , m_editMenu(nullptr)
    , m_viewMenu(nullptr)
    , m_helpMenu(nullptr)
    , m_mainToolBar(nullptr)
    , m_formatToolBar(nullptr)
    , m_newAction(nullptr)
    , m_openAction(nullptr)
    , m_saveAction(nullptr)
    , m_saveAsAction(nullptr)
    , m_printAction(nullptr)
    , m_exitAction(nullptr)
    , m_undoAction(nullptr)
    , m_redoAction(nullptr)
    , m_cutAction(nullptr)
    , m_copyAction(nullptr)
    , m_pasteAction(nullptr)
    , m_findAction(nullptr)
    , m_replaceAction(nullptr)
    , m_selectAllAction(nullptr)
    , m_wordWrapAction(nullptr)
    , m_showToolBarAction(nullptr)
    , m_showStatusBarAction(nullptr)
    , m_aboutAction(nullptr)
    , m_aboutQtAction(nullptr)
{
    setupUI();          // 创建用户界面
    createFileTree();   // 创建文件树并添加示例文件
    setWindowTitle(tr("Ink - 文本编辑器"));  // 设置窗口标题
    resize(1200, 800);  // 设置窗口初始大小
}

/**
 * @brief 主窗口析构函数
 * 
 * 清理资源。目前没有需要特殊清理的资源，预留用于未来扩展。
 */
MainWindow::~MainWindow() 
{
    // 清理资源 - 目前没有需要特殊清理的资源
}

/**
 * @brief 初始化用户界面
 * 
 * 创建主窗口的核心界面组件：
 * 1. 创建菜单栏和工具栏
 * 2. 创建中心部件和主布局
 * 3. 水平分割器分隔左右面板
 * 4. 左侧面板：文件树控件，显示打开的文件
 * 5. 右侧面板：文本编辑控件，用于编辑文件内容
 * 6. 连接信号槽：文件树项点击事件
 */
void MainWindow::setupUI()
{
    // 创建菜单栏和工具栏
    createMenuBar();
    createToolBar();
    
    // 创建中心部件
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    
    // 创建主布局 - 水平布局
    QHBoxLayout *mainLayout = new QHBoxLayout(centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);  // 移除边距，使分割器占满整个窗口
    
    // 创建水平分割器，用于分隔左右面板
    m_splitter = new QSplitter(Qt::Horizontal, centralWidget);
    
    // ==== 左侧面板：文件树 ====
    QWidget *leftPanel = new QWidget(m_splitter);
    QVBoxLayout *leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->setContentsMargins(5, 5, 5, 5);  // 设置边距
    
    // 创建文件树控件
    m_fileTree = new QTreeWidget(leftPanel);
    m_fileTree->setHeaderLabel(tr("打开的文件"));  // 设置标题
    m_fileTree->setColumnCount(1);                 // 单列显示
    m_fileTree->setAnimated(true);                 // 启用动画效果
    m_fileTree->setIndentation(15);                // 设置缩进
    m_fileTree->setSortingEnabled(false);          // 禁用排序
    m_fileTree->setContextMenuPolicy(Qt::CustomContextMenu);  // 启用自定义上下文菜单
    m_fileTree->header()->setStretchLastSection(true);        // 拉伸最后一列
    m_fileTree->setMinimumWidth(250);              // 设置最小宽度
    
    leftLayout->addWidget(m_fileTree);
    leftPanel->setLayout(leftLayout);
    
    // ==== 右侧面板：文本编辑器 ====
    QWidget *rightPanel = new QWidget(m_splitter);
    QVBoxLayout *rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->setContentsMargins(0, 0, 0, 0);  // 无边距
    
    // 创建文本编辑控件
    m_textEdit = new QTextEdit(rightPanel);
    m_textEdit->setAcceptRichText(false);          // 禁用富文本，只接受纯文本
    m_textEdit->setLineWrapMode(QTextEdit::WidgetWidth);  // 按窗口宽度自动换行
    
    // 设置等宽字体，适合代码编辑
    QFont fixedFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    fixedFont.setPointSize(10);                     // 设置字体大小
    m_textEdit->setFont(fixedFont);
    
    rightLayout->addWidget(m_textEdit);
    rightPanel->setLayout(rightLayout);
    
    // 将左右面板添加到分割器
    m_splitter->addWidget(leftPanel);
    m_splitter->addWidget(rightPanel);
    
    // 设置分割比例：左侧1份，右侧3份
    m_splitter->setStretchFactor(0, 1);
    m_splitter->setStretchFactor(1, 3);
    
    // 将分割器添加到主布局
    mainLayout->addWidget(m_splitter);
    
    // 连接信号槽：当文件树项被点击时，调用 onFileItemClicked 函数
    connect(m_fileTree, &QTreeWidget::itemClicked, this, &MainWindow::onFileItemClicked);
}

/**
 * @brief 创建菜单栏
 * 
 * 创建应用程序的菜单栏，包含以下菜单：
 * 1. 文件菜单：新建、打开、保存、另存为、打印、退出
 * 2. 编辑菜单：撤销、重做、剪切、复制、粘贴、查找、替换、全选
 * 3. 视图菜单：自动换行、显示工具栏、显示状态栏
 * 4. 帮助菜单：关于、关于Qt
 */
void MainWindow::createMenuBar()
{
    // 获取主窗口的菜单栏
    m_menuBar = menuBar();
    
    // ==== 文件菜单 ====
    m_fileMenu = m_menuBar->addMenu(tr("文件(&F)"));
    
    m_newAction = new QAction(tr("新建(&N)"), this);
    m_newAction->setShortcut(QKeySequence::New);
    m_newAction->setStatusTip(tr("创建新文件"));
    m_fileMenu->addAction(m_newAction);
    
    m_openAction = new QAction(tr("打开(&O)..."), this);
    m_openAction->setShortcut(QKeySequence::Open);
    m_openAction->setStatusTip(tr("打开文件"));
    m_fileMenu->addAction(m_openAction);
    
    m_fileMenu->addSeparator();
    
    m_saveAction = new QAction(tr("保存(&S)"), this);
    m_saveAction->setShortcut(QKeySequence::Save);
    m_saveAction->setStatusTip(tr("保存文件"));
    m_fileMenu->addAction(m_saveAction);
    
    m_saveAsAction = new QAction(tr("另存为(&A)..."), this);
    m_saveAsAction->setShortcut(QKeySequence::SaveAs);
    m_saveAsAction->setStatusTip(tr("另存为"));
    m_fileMenu->addAction(m_saveAsAction);
    
    m_fileMenu->addSeparator();
    
    m_printAction = new QAction(tr("打印(&P)..."), this);
    m_printAction->setShortcut(QKeySequence::Print);
    m_printAction->setStatusTip(tr("打印文件"));
    m_fileMenu->addAction(m_printAction);
    
    m_fileMenu->addSeparator();
    
    m_exitAction = new QAction(tr("退出(&X)"), this);
    m_exitAction->setShortcut(QKeySequence::Quit);
    m_exitAction->setStatusTip(tr("退出应用程序"));
    m_fileMenu->addAction(m_exitAction);
    
    // ==== 编辑菜单 ====
    m_editMenu = m_menuBar->addMenu(tr("编辑(&E)"));
    
    m_undoAction = new QAction(tr("撤销(&U)"), this);
    m_undoAction->setShortcut(QKeySequence::Undo);
    m_undoAction->setStatusTip(tr("撤销上一次操作"));
    m_undoAction->setEnabled(false);
    m_editMenu->addAction(m_undoAction);
    
    m_redoAction = new QAction(tr("重做(&R)"), this);
    m_redoAction->setShortcut(QKeySequence::Redo);
    m_redoAction->setStatusTip(tr("重做上一次撤销的操作"));
    m_redoAction->setEnabled(false);
    m_editMenu->addAction(m_redoAction);
    
    m_editMenu->addSeparator();
    
    m_cutAction = new QAction(tr("剪切(&T)"), this);
    m_cutAction->setShortcut(QKeySequence::Cut);
    m_cutAction->setStatusTip(tr("剪切选中的文本"));
    m_cutAction->setEnabled(false);
    m_editMenu->addAction(m_cutAction);
    
    m_copyAction = new QAction(tr("复制(&C)"), this);
    m_copyAction->setShortcut(QKeySequence::Copy);
    m_copyAction->setStatusTip(tr("复制选中的文本"));
    m_copyAction->setEnabled(false);
    m_editMenu->addAction(m_copyAction);
    
    m_pasteAction = new QAction(tr("粘贴(&P)"), this);
    m_pasteAction->setShortcut(QKeySequence::Paste);
    m_pasteAction->setStatusTip(tr("粘贴剪贴板中的文本"));
    m_editMenu->addAction(m_pasteAction);
    
    m_editMenu->addSeparator();
    
    m_findAction = new QAction(tr("查找(&F)..."), this);
    m_findAction->setShortcut(QKeySequence::Find);
    m_findAction->setStatusTip(tr("查找文本"));
    m_editMenu->addAction(m_findAction);
    
    m_replaceAction = new QAction(tr("替换(&R)..."), this);
    m_replaceAction->setShortcut(QKeySequence::Replace);
    m_replaceAction->setStatusTip(tr("替换文本"));
    m_editMenu->addAction(m_replaceAction);
    
    m_editMenu->addSeparator();
    
    m_selectAllAction = new QAction(tr("全选(&A)"), this);
    m_selectAllAction->setShortcut(QKeySequence::SelectAll);
    m_selectAllAction->setStatusTip(tr("选择所有文本"));
    m_editMenu->addAction(m_selectAllAction);
    
    // ==== 视图菜单 ====
    m_viewMenu = m_menuBar->addMenu(tr("视图(&V)"));
    
    m_wordWrapAction = new QAction(tr("自动换行(&W)"), this);
    m_wordWrapAction->setCheckable(true);
    m_wordWrapAction->setChecked(true);
    m_wordWrapAction->setStatusTip(tr("启用或禁用自动换行"));
    m_viewMenu->addAction(m_wordWrapAction);
    
    m_viewMenu->addSeparator();
    
    m_showToolBarAction = new QAction(tr("工具栏(&T)"), this);
    m_showToolBarAction->setCheckable(true);
    m_showToolBarAction->setChecked(true);
    m_showToolBarAction->setStatusTip(tr("显示或隐藏工具栏"));
    m_viewMenu->addAction(m_showToolBarAction);
    
    m_showStatusBarAction = new QAction(tr("状态栏(&S)"), this);
    m_showStatusBarAction->setCheckable(true);
    m_showStatusBarAction->setChecked(true);
    m_showStatusBarAction->setStatusTip(tr("显示或隐藏状态栏"));
    m_viewMenu->addAction(m_showStatusBarAction);
    
    // ==== 帮助菜单 ====
    m_helpMenu = m_menuBar->addMenu(tr("帮助(&H)"));
    
    m_aboutAction = new QAction(tr("关于(&A)..."), this);
    m_aboutAction->setStatusTip(tr("显示关于对话框"));
    m_helpMenu->addAction(m_aboutAction);
    
    m_aboutQtAction = new QAction(tr("关于Qt(&Q)..."), this);
    m_aboutQtAction->setStatusTip(tr("显示关于Qt对话框"));
    m_helpMenu->addAction(m_aboutQtAction);
}

/**
 * @brief 创建工具栏
 * 
 * 创建应用程序的工具栏，包含常用功能的按钮：
 * 1. 主工具栏：新建、打开、保存、打印、剪切、复制、粘贴、查找
 * 2. 格式工具栏：自动换行、字体设置等（预留）
 */
void MainWindow::createToolBar()
{
    // 创建主工具栏
    m_mainToolBar = addToolBar(tr("主工具栏"));
    m_mainToolBar->setMovable(true);
    
    // 添加工具栏按钮
    m_mainToolBar->addAction(m_newAction);
    m_mainToolBar->addAction(m_openAction);
    m_mainToolBar->addAction(m_saveAction);
    
    m_mainToolBar->addSeparator();
    
    m_mainToolBar->addAction(m_printAction);
    
    m_mainToolBar->addSeparator();
    
    m_mainToolBar->addAction(m_cutAction);
    m_mainToolBar->addAction(m_copyAction);
    m_mainToolBar->addAction(m_pasteAction);
    
    m_mainToolBar->addSeparator();
    
    m_mainToolBar->addAction(m_findAction);
    
    // 创建格式工具栏（预留，暂时不添加具体内容）
    m_formatToolBar = addToolBar(tr("格式"));
    m_formatToolBar->setMovable(true);
    m_formatToolBar->setVisible(false);  // 默认隐藏格式工具栏
}

/**
 * @brief 创建文件树并添加示例文件
 * 
 * 清空现有文件树，然后添加一些示例文件用于演示。
 * 这些示例文件包括不同类型的文件（C++、Python、Markdown等），
 * 分布在不同的文件夹中，以展示文件树的分层结构。
 */
void MainWindow::createFileTree()
{
    m_fileTree->clear();  // 清空现有文件树
    
    // 模拟一些打开的文件用于演示
    QStringList demoFiles;
    demoFiles << "c:/projects/src/main.cpp"      // C++源文件
              << "c:/projects/src/utils.cpp"     // C++源文件
              << "c:/projects/include/utils.h"   // C++头文件
              << "c:/docs/notes.txt"             // 文本文件
              << "c:/docs/todo.md"               // Markdown文件
              << "c:/temp/test.py"               // Python文件
              << "c:/temp/backup.sh";            // Shell脚本
    
    // 遍历示例文件列表，逐个添加到文件树
    foreach (const QString &filePath, demoFiles) {
        addFileToTree(filePath);
    }
    
    // 展开所有项，方便用户查看
    m_fileTree->expandAll();
}

/**
 * @brief 查找或创建文件夹项
 * @param folderPath 文件夹路径
 * @return 文件夹树项指针
 * 
 * 在文件树中查找指定文件夹路径的项。如果找到，返回该项；
 * 如果未找到，创建一个新的文件夹项并添加到文件树中。
 * 使用 Qt::UserRole 存储文件夹路径作为标识。
 */
QTreeWidgetItem* MainWindow::findOrCreateFolderItem(const QString &folderPath)
{
    // 检查是否已存在该文件夹项
    for (int i = 0; i < m_fileTree->topLevelItemCount(); ++i) {
        QTreeWidgetItem *topItem = m_fileTree->topLevelItem(i);
        // 通过 UserRole 数据比较文件夹路径
        if (topItem->data(0, Qt::UserRole).toString() == folderPath) {
            return topItem;  // 找到现有项，直接返回
        }
    }
    
    // 创建新的文件夹项
    QTreeWidgetItem *folderItem = new QTreeWidgetItem(m_fileTree);
    folderItem->setText(0, folderPath);  // 显示文件夹路径
    folderItem->setData(0, Qt::UserRole, folderPath);  // 存储文件夹路径作为标识
    folderItem->setIcon(0, QApplication::style()->standardIcon(QStyle::SP_DirIcon));  // 设置文件夹图标
    
    return folderItem;  // 返回新创建的文件夹项
}

/**
 * @brief 添加文件到文件树
 * @param filePath 文件路径
 * 
 * 根据文件路径将文件添加到文件树中：
 * 1. 提取文件所在文件夹路径
 * 2. 查找或创建对应的文件夹项
 * 3. 在文件夹项下创建文件项
 * 4. 将文件路径添加到打开文件映射中
 * 
 * 文件树结构：文件夹（顶级）-> 文件（子级）
 */
void MainWindow::addFileToTree(const QString &filePath)
{
    QFileInfo fileInfo(filePath);
    QString folderPath = fileInfo.absolutePath();  // 提取文件夹路径
    
    // 找到或创建文件夹项
    QTreeWidgetItem *folderItem = findOrCreateFolderItem(folderPath);
    
    // 在文件夹项下创建文件项
    QTreeWidgetItem *fileItem = new QTreeWidgetItem(folderItem);
    fileItem->setText(0, fileInfo.fileName());  // 显示文件名
    fileItem->setData(0, Qt::UserRole, filePath);  // 存储完整文件路径作为标识
    fileItem->setIcon(0, QApplication::style()->standardIcon(QStyle::SP_FileIcon));  // 设置文件图标
    
    // 添加到打开文件映射中，初始内容为空
    m_openFiles[filePath] = "";
}

/**
 * @brief 文件树项点击槽函数
 * @param item 被点击的树项
 * @param column 点击的列（未使用）
 * 
 * 当用户在文件树中点击一个项目时触发：
 * 1. 检查点击的是文件项还是文件夹项（文件夹项不处理）
 * 2. 从项目数据中获取文件路径
 * 3. 如果文件内容尚未加载，生成模拟内容
 * 4. 将文件内容显示在文本编辑区域
 * 
 * 模拟内容根据文件类型生成不同的示例代码：
 * - C++文件：生成简单的 Hello World 程序
 * - Python文件：生成 Python 脚本
 * - Markdown文件：生成 Markdown 文档模板
 * - 其他文件：生成通用文本内容
 */
void MainWindow::onFileItemClicked(QTreeWidgetItem *item, int column)
{
    Q_UNUSED(column);  // 列参数未使用
    
    if (!item) return;  // 空指针检查
    
    // 从树项数据中获取文件路径
    QString filePath = item->data(0, Qt::UserRole).toString();
    QFileInfo fileInfo(filePath);
    
    // 如果是文件夹项，不处理（文件夹项没有对应的文件）
    if (fileInfo.isDir() || !fileInfo.isFile()) {
        return;
    }
    
    // 模拟加载文件内容
    QString content;
    if (m_openFiles.contains(filePath)) {
        content = m_openFiles[filePath];  // 获取已缓存的内容
        
        // 如果内容为空，生成模拟内容
        if (content.isEmpty()) {
            // 生成文件头信息
            content = QString("// 文件: %1\n// 路径: %2\n// 最后修改: %3\n\n")
                .arg(fileInfo.fileName())
                .arg(fileInfo.absoluteFilePath())
                .arg(fileInfo.lastModified().toString("yyyy-MM-dd hh:mm:ss"));
            
            // 根据文件类型添加不同的示例内容
            if (fileInfo.suffix() == "cpp" || fileInfo.suffix() == "h") {
                // C++文件：简单的 Hello World 程序
                content += "#include <iostream>\n\nint main() {\n    std::cout << \"Hello, World!\" << std::endl;\n    return 0;\n}\n";
            } else if (fileInfo.suffix() == "py") {
                // Python文件：Python 脚本
                content += "#!/usr/bin/env python3\n\nprint(\"Hello, Python!\")\n";
            } else if (fileInfo.suffix() == "md") {
                // Markdown文件：文档模板
                content += "# " + fileInfo.fileName() + "\n\n这是一个 Markdown 文件。\n\n## 项目说明\n\n- 项目功能\n- 使用方法\n";
            } else {
                // 其他文件：通用文本内容
                content += "这是 " + fileInfo.fileName() + " 文件的内容。\n你可以在这里编辑文本。\n";
            }
            
            // 缓存生成的内容，避免重复生成
            m_openFiles[filePath] = content;
        }
    }
    
    // 更新文本编辑区域：显示文件内容
    m_textEdit->setPlainText(content);
    m_textEdit->setWindowTitle(fileInfo.fileName());  // 更新窗口标题显示文件名
}

/**
 * @brief 更新文件树槽函数
 * 
 * 这个函数目前是一个占位符，用于未来扩展。
 * 可以在这里实现更新文件树的逻辑，例如：
 * 1. 重新扫描已打开的文件
 * 2. 响应外部文件变化（如文件被删除、重命名等）
 * 3. 刷新文件树显示
 * 4. 更新文件状态（如只读、修改状态等）
 * 
 * 当前实现为空，需要根据具体需求添加功能。
 */
void MainWindow::updateFileTree()
{
    // 可以在这里实现更新文件树的逻辑
    // 例如，重新扫描打开的文件或响应外部变化
    // 当前为占位实现，需要时添加具体功能
}
