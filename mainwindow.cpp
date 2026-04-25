#include "mainwindow.h"
#include "linenumbereditor.h"
#include "findreplacedialog.h"
#include "filemanage.h"
#include <QApplication>
#include <QSplitter>
#include <QTreeWidget>
#include <QTabWidget>
#include <QPlainTextEdit>
#include <QTextEdit>
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QAction>
#include <QFileDialog>
#include <QMessageBox>
#include <QTextStream>
#include <QFileInfo>
#include <QSettings>
#include <QFont>
#include <QFontDatabase>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QHeaderView>
#include <QDir>
#include <QDebug>
#include <QMap>
#include <QIcon>
#include <QDesktopServices>
#include <QShortcut>
#include <QProcess>
#include <QMenu>

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QStringConverter>
#include <qlineedit.h>
#endif

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    // 处理所有键盘事件，不仅仅是编辑器控件
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);

        // 调试输出所有按键
        qDebug() << "Key event - Object:" << obj->metaObject()->className()
                 << "Key:" << keyEvent->key()
                 << "Modifiers:" << keyEvent->modifiers()
                 << "Text:" << keyEvent->text();

        // 处理 Ctrl+F - 无论焦点在哪里都响应
        if (keyEvent->modifiers() == Qt::ControlModifier && keyEvent->key() == Qt::Key_F) {
            qDebug() << "Ctrl+F detected!";
            showFindDialog();
            return true;
        }

        // 处理 Ctrl+R - 无论焦点在哪里都响应
        if (keyEvent->modifiers() == Qt::ControlModifier && keyEvent->key() == Qt::Key_R) {
            qDebug() << "Ctrl+R detected!";
            showReplaceDialog();
            return true;
        }

        // 处理 F3 和 Shift+F3 - 只在编辑器中有焦点时处理
        if (qobject_cast<QPlainTextEdit*>(obj) || qobject_cast<LineNumberEditor*>(obj)) {
            if (keyEvent->key() == Qt::Key_F3 && keyEvent->modifiers() == Qt::NoModifier) {
                qDebug() << "F3 detected in editor!";
                if (m_findReplaceDialog && m_findReplaceDialog->isVisible()) {
                    m_findReplaceDialog->findNextMatch();
                } else {
                    showFindDialog();
                }
                return true;
            }
            else if (keyEvent->key() == Qt::Key_F3 && keyEvent->modifiers() == Qt::ShiftModifier) {
                qDebug() << "Shift+F3 detected in editor!";
                if (m_findReplaceDialog && m_findReplaceDialog->isVisible()) {
                    m_findReplaceDialog->findPrevMatch();
                }
                return true;
            }
        }
    }

    return QMainWindow::eventFilter(obj, event);
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , previewVisible(false)
    , m_findReplaceDialog(nullptr)
    , m_fileManage(new FileManage(this))
    , fileTreeContextMenu(nullptr)
    , editorContextMenu(nullptr)
    , removeFileAct(nullptr)
    , openFileLocationAct(nullptr)
    , openCmdAct(nullptr)
    , openGitBashAct(nullptr)
{
    // 设置窗口图标
    setWindowIcon(QIcon(":/icons/icon1"));
    
    // 设置窗口样式（整体亮色主题）
    setStyleSheet(
        "QMainWindow {"
        "    background-color: #f5f5f5;"
        "}"
        "QMenuBar {"
        "    background-color: #f0f0f0;"
        "    color: #333333;"
        "    border-bottom: 1px solid #d0d0d0;"
        "}"
        "QMenuBar::item {"
        "    padding: 4px 8px;"
        "    background-color: transparent;"
        "}"
        "QMenuBar::item:selected {"
        "    background-color: #e0e0e0;"
        "    border-radius: 2px;"
        "}"
        "QMenuBar::item:pressed {"
        "    background-color: #d0d0d0;"
        "}"
        "QMenu {"
        "    background-color: #f8f8f8;"
        "    color: #333333;"
        "    border: 1px solid #d0d0d0;"
        "    padding: 2px;"
        "}"
        "QMenu::item {"
        "    padding: 4px 20px;"
        "}"
        "QMenu::item:selected {"
        "    background-color: #00A663;"
        "    color: #ffffff;"
        "}"
        "QMenu::separator {"
        "    height: 1px;"
        "    background-color: #d0d0d0;"
        "    margin: 2px 8px;"
        "}"
        "QPushButton {"
        "    color: #333333;"
        "}"
        "QLabel {"
        "    color: #333333;"
        "}"
        );
    
    // 设置窗口属性
    setWindowTitle("INK - 无标题");
    resize(1200, 800);

    // 初始化UI
    initUI();
    initMenuBar();
    initToolBar();
    initStatusBar();
    // 初始化文件列表
    initFileList();
    initConnections();

    // 安装事件过滤器
    installEventFilter(this);

    // 加载设置
    QSettings settings("INK", "Settings");
    restoreGeometry(settings.value("geometry").toByteArray());
    // 从配置加载 Git Bash 路径
    m_gitBashPath = settings.value("GitBashPath", "").toString();
    // 加载上次打开的目录
    m_lastOpenedDir = settings.value("LastOpenedDir", "").toString();

    mainSplitter->restoreState(settings.value("splitterState").toByteArray());
    previewVisible = settings.value("previewVisible", false).toBool();
    togglePreviewAct->setChecked(previewVisible);

    // 初始状态：没有打开的文件
    updateWindowTitle();
    updateStatusBar();
    updateEditActions();

    // 创建查找替换对话框（延迟创建）
    m_findReplaceDialog = new FindReplaceDialog(this);

    // 连接FileManage信号
    connect(m_fileManage, &FileManage::fileTreeUpdated, this, [this]() {
        // 文件树更新后可能需要刷新显示
        fileTree->update();
    });

    connect(m_fileManage, &FileManage::recentFilesUpdated, this, [this](const QStringList &recentFiles) {
        // 可以在这里更新最近文件菜单
        // 暂时不需要实现
    });

    // 从FileManage加载文件列表
    QStringList recentFiles = m_fileManage->loadFileList();
    for (const QString &filePath : recentFiles) {
        if (QFile::exists(filePath)) {
            m_fileManage->addFileToList(filePath, fileTree);
        }
    }
    // 展开所有目录
    fileTree->expandAll();
}

MainWindow::~MainWindow()
{
    // 保存设置
    QSettings settings("INK", "Settings");
    settings.setValue("geometry", saveGeometry());
    settings.setValue("splitterState", mainSplitter->saveState());
    settings.setValue("previewVisible", previewVisible);
    settings.setValue("LastOpenedDir", m_lastOpenedDir);

    // 保存当前打开的文件列表
    QStringList openFiles;
    for (int i = 0; i < tabWidget->count(); ++i) {
        QPlainTextEdit *editor = qobject_cast<QPlainTextEdit*>(tabWidget->widget(i));
        if (editor) {
            QString filePath = editor->property("filePath").toString();
            if (!filePath.isEmpty() && QFile::exists(filePath)) {
                openFiles.append(filePath);
            }
        }
    }
    m_fileManage->saveFileList(openFiles, m_fileManage->getRecentFiles());

    delete m_findReplaceDialog;
}

void MainWindow::initUI()
{
    // 创建主分割器（左右布局）
    mainSplitter = new QSplitter(Qt::Horizontal, this);

    // 左侧面板：文件列表
    leftPanel = new QWidget(mainSplitter);
    QVBoxLayout *leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->setContentsMargins(0, 0, 0, 0);
    leftLayout->setSpacing(0);

    // 文件列表工具栏 - 高度与导航栏保持一致
    QWidget *toolbarWidget = new QWidget(leftPanel);
    toolbarWidget->setFixedHeight(32);  // 设置稍高的工具栏
    toolbarWidget->setStyleSheet("background-color: #f0f0f0; border-bottom: 1px solid #d0d0d0;");

    QHBoxLayout *toolbarLayout = new QHBoxLayout(toolbarWidget);
    toolbarLayout->setContentsMargins(4, 0, 4, 0);  // 减少边距
    toolbarLayout->setSpacing(2);  // 减小按钮间距

    // 创建左侧按钮容器（用于定位、展开、折叠）
    QWidget *leftButtons = new QWidget(toolbarWidget);
    leftButtons->setStyleSheet("background: transparent;");
    QHBoxLayout *leftButtonLayout = new QHBoxLayout(leftButtons);
    leftButtonLayout->setContentsMargins(0, 0, 0, 0);
    leftButtonLayout->setSpacing(2);

    // 创建右侧按钮容器（用于新建、打开）
    QWidget *rightButtons = new QWidget(toolbarWidget);
    rightButtons->setStyleSheet("background: transparent;");
    QHBoxLayout *rightButtonLayout = new QHBoxLayout(rightButtons);
    rightButtonLayout->setContentsMargins(0, 0, 0, 0);
    rightButtonLayout->setSpacing(2);

    // 创建工具栏按钮 - 使用图标
    locateBtn = new QPushButton(toolbarWidget);
    locateBtn->setIcon(QIcon(":/icons/location"));  // 从资源文件加载图标
    locateBtn->setFixedSize(24, 20);  // 固定大小
    locateBtn->setStyleSheet(
        "QPushButton {"
        "    background-color: transparent;"  // 透明背景
        "    border: none;"  // 无边框
        "    border-radius: 4px;"
        "    padding: 4px;"
        "}"
        "QPushButton:hover {"
        "    background-color: #e0e0e0;"  // 浅色悬停背景
        "}"
        "QPushButton:pressed {"
        "    background-color: #d0d0d0;"  // 按下时显示背景
        "}"
        );

    expandBtn = new QPushButton(toolbarWidget);
    expandBtn->setIcon(QIcon(":/icons/expand"));
    expandBtn->setFixedSize(24, 20);
    expandBtn->setStyleSheet(
        "QPushButton {"
        "    background-color: transparent;"
        "    border: none;"
        "    border-radius: 4px;"
        "    padding: 4px;"
        "}"
        "QPushButton:hover {"
        "    background-color: #e0e0e0;"
        "}"
        "QPushButton:pressed {"
        "    background-color: #d0d0d0;"
        "}"
        );

    foldBtn = new QPushButton(toolbarWidget);
    foldBtn->setIcon(QIcon(":/icons/fold"));
    foldBtn->setFixedSize(24, 20);
    foldBtn->setStyleSheet(
        "QPushButton {"
        "    background-color: transparent;"
        "    border: none;"
        "    border-radius: 4px;"
        "    padding: 4px;"
        "}"
        "QPushButton:hover {"
        "    background-color: #e0e0e0;"
        "}"
        "QPushButton:pressed {"
        "    background-color: #d0d0d0;"
        "}"
        );


    // 左侧按钮：定位、展开、折叠（按顺序：定位、折叠、展开）
    leftButtonLayout->addWidget(locateBtn);
    leftButtonLayout->addWidget(foldBtn);
    leftButtonLayout->addWidget(expandBtn);

    // 将左右按钮容器添加到主工具栏布局
    toolbarLayout->addWidget(rightButtons);  // 左侧按钮容器（靠左）
    toolbarLayout->addStretch();           // 弹性空间，将右侧按钮推到最右边
    toolbarLayout->addWidget(leftButtons); // 右侧按钮容器（靠右）

    // 文件树（使用QTreeWidget）
    fileTree = new QTreeWidget(leftPanel);
    fileTree->setHeaderHidden(true);  // 隐藏标题栏，显示空白列表
    fileTree->setAlternatingRowColors(true);
    fileTree->setAnimated(true);
    fileTree->setIndentation(15);

    // 添加到左侧布局
    leftLayout->addWidget(toolbarWidget);
    leftLayout->addWidget(fileTree);

    leftPanel->setMinimumWidth(200);
    leftPanel->setMaximumWidth(350);

    // 中央区域：编辑器标签页
    QWidget *centerPanel = new QWidget(mainSplitter);
    QVBoxLayout *centerLayout = new QVBoxLayout(centerPanel);
    centerLayout->setContentsMargins(0, 0, 0, 0);

    tabWidget = new QTabWidget(centerPanel);
    tabWidget->setTabsClosable(true);
    tabWidget->setMovable(true);
    tabWidget->setDocumentMode(true);
    // 设置标签页的高度，左侧工具栏与其保持一致
    tabWidget->setStyleSheet(
        "QTabWidget::pane {"
        "    border: none;"
        "    background-color: #ffffff;"
        "}"
        "QTabBar::tab {"
        "    background-color: #f0f0f0;"
        "    color: #555555;"
        "    padding: 6px 12px;"
        "    margin-right: 2px;"
        "    border-top-left-radius: 4px;"
        "    border-top-right-radius: 4px;"
        "    border: 1px solid #d0d0d0;"
        "    border-bottom: none;"
        "    min-width: 80px;"
        "    height: 18px;"
        "}"
        "QTabBar::tab:selected {"
        "    background-color: #ffffff;"
        "    color: #00A663;"
        "    border-bottom: 2px solid #00A663;"
        "}"
        "QTabBar::tab:hover:!selected {"
        "    background-color: #e0e0e0;"
        "}"
        "QTabBar::close-button {"
        "    subcontrol-origin: padding;"
        "    subcontrol-position: right;"
        "    padding: 2px;"
        "}"
        "QTabBar::close-button:hover {"
        "    background-color: #e0e0e0;"
        "    border-radius: 2px;"
        "}"
        );

    centerLayout->addWidget(tabWidget);

    // 预览面板（初始隐藏）
    previewPanel = new QTextEdit(mainSplitter);
    previewPanel->setReadOnly(true);
    previewPanel->setVisible(previewVisible);
    previewPanel->setMinimumWidth(300);
    // 设置预览面板样式
    previewPanel->setStyleSheet(
        "QTextEdit {"
        "    background-color: #ffffff;"
        "    color: #333333;"
        "    border: none;"
        "    padding: 10px;"
        "    font-family: 'Segoe UI', 'Microsoft YaHei', monospace;"
        "    font-size: 11pt;"
        "}"
        "QTextEdit:focus {"
        "    outline: none;"
        "}"
        // 滚动条样式
        "QScrollBar:vertical {"
        "    border: none;"
        "    background: #f0f0f0;"
        "    width: 12px;"
        "}"
        "QScrollBar::handle:vertical {"
        "    background: #c0c0c0;"
        "    min-height: 20px;"
        "    border-radius: 6px;"
        "}"
        "QScrollBar::handle:vertical:hover {"
        "    background: #a0a0a0;"
        "}"
        );

    // 添加到分割器
    mainSplitter->addWidget(leftPanel);
    mainSplitter->addWidget(centerPanel);
    mainSplitter->addWidget(previewPanel);

    // 设置分割器比例
    QList<int> sizes;
    if (previewVisible) {
        sizes << 200 << 500 << 300;
    } else {
        sizes << 200 << 600 << 0;
        previewPanel->setVisible(false);
    }
    mainSplitter->setSizes(sizes);

    setCentralWidget(mainSplitter);
}

void MainWindow::initMenuBar()
{
    // 文件菜单
    fileMenu = menuBar()->addMenu("文件(&F)");

    newAct = new QAction("新建(&N)", this);
    newAct->setShortcut(QKeySequence::New);
    fileMenu->addAction(newAct);

    openAct = new QAction("打开(&O)", this);
    openAct->setShortcut(QKeySequence::Open);
    fileMenu->addAction(openAct);

    fileMenu->addSeparator();

    saveAct = new QAction("保存(&S)", this);
    saveAct->setShortcut(QKeySequence::Save);
    saveAct->setEnabled(false);
    fileMenu->addAction(saveAct);

    saveAsAct = new QAction("另存为(&A)", this);
    saveAsAct->setShortcut(QKeySequence::SaveAs);
    saveAsAct->setEnabled(false);
    fileMenu->addAction(saveAsAct);

    fileMenu->addSeparator();

    closeAct = new QAction("关闭(&C)", this);
    closeAct->setShortcut(QKeySequence::Close);
    closeAct->setEnabled(false);
    fileMenu->addAction(closeAct);

    exitAct = new QAction("退出(&X)", this);
    exitAct->setShortcut(QKeySequence::Quit);
    fileMenu->addAction(exitAct);

    // 编辑菜单
    editMenu = menuBar()->addMenu("编辑(&E)");

    undoAct = new QAction("撤销(&U)", this);
    undoAct->setShortcut(QKeySequence::Undo);
    undoAct->setEnabled(false);
    editMenu->addAction(undoAct);

    redoAct = new QAction("重做(&R)", this);
    redoAct->setShortcut(QKeySequence::Redo);
    redoAct->setEnabled(false);
    editMenu->addAction(redoAct);

    editMenu->addSeparator();

    QAction *findAct = new QAction("查找(&F)", this);
    findAct->setShortcut(QKeySequence::Find);
    connect(findAct, &QAction::triggered, this, &MainWindow::showFindDialog);
    editMenu->addAction(findAct);

    QAction *replaceAct = new QAction("替换(&R)", this);
    replaceAct->setShortcut(QKeySequence::Replace);
    connect(replaceAct, &QAction::triggered, this, &MainWindow::showReplaceDialog);
    editMenu->addAction(replaceAct);

    editMenu->addSeparator();

    cutAct = new QAction("剪切(&T)", this);
    cutAct->setShortcut(QKeySequence::Cut);
    cutAct->setEnabled(false);
    editMenu->addAction(cutAct);

    copyAct = new QAction("复制(&C)", this);
    copyAct->setShortcut(QKeySequence::Copy);
    copyAct->setEnabled(false);
    editMenu->addAction(copyAct);

    pasteAct = new QAction("粘贴(&P)", this);
    pasteAct->setShortcut(QKeySequence::Paste);
    editMenu->addAction(pasteAct);

    // 视图菜单
    viewMenu = menuBar()->addMenu("视图(&V)");

    toggleFileListAct = new QAction("文件列表", this);
    toggleFileListAct->setShortcut(QKeySequence("Ctrl+Shift+F"));
    toggleFileListAct->setCheckable(true);
    toggleFileListAct->setChecked(true);
    viewMenu->addAction(toggleFileListAct);

    togglePreviewAct = new QAction("预览面板", this);
    togglePreviewAct->setShortcut(QKeySequence("Ctrl+Shift+P"));
    togglePreviewAct->setCheckable(true);
    togglePreviewAct->setChecked(previewVisible);
    viewMenu->addAction(togglePreviewAct);

    // 帮助菜单
    helpMenu = menuBar()->addMenu("帮助(&H)");

    aboutAct = new QAction("关于(&A)", this);
    helpMenu->addAction(aboutAct);
}

void MainWindow::initToolBar()
{
    QToolBar *toolBar = addToolBar("主工具栏");
    toolBar->setMovable(false);
    toolBar->setIconSize(QSize(20, 20));  // 设置图标大小

    // 设置工具栏样式
    toolBar->setStyleSheet(
        "QToolBar {"
        "    spacing: 4px;"
        "    background-color: #f0f0f0;"
        "    border: none;"
        "    border-bottom: 1px solid #d0d0d0;"
        "    padding: 2px;"
        "}"
        "QToolBar QToolButton {"
        "    padding: 4px 6px;"
        "    border-radius: 4px;"
        "    margin: 1px;"
        "    color: #333333;"
        "}"
        "QToolBar QToolButton:hover {"
        "    background-color: #e0e0e0;"
        "}"
        "QToolBar QToolButton:pressed {"
        "    background-color: #d0d0d0;"
        "}"
        "QToolBar QToolButton:disabled {"
        "    color: #aaaaaa;"
        "}"
        "QToolBar::separator {"
        "    background-color: #d0d0d0;"
        "    width: 1px;"
        "    margin: 4px 6px;"
        "}"
        );

    // 为所有动作设置图标
    newAct->setIcon(QIcon(":/icons/addfile"));
    openAct->setIcon(QIcon(":/icons/openfile"));

    // 使用图标模式添加动作
    toolBar->addAction(newAct);
    toolBar->addAction(openAct);
    toolBar->addSeparator();
    toolBar->addAction(saveAct);
    toolBar->addSeparator();
    toolBar->addAction(undoAct);
    toolBar->addAction(redoAct);
    toolBar->addSeparator();
    toolBar->addAction(cutAct);
    toolBar->addAction(copyAct);
    toolBar->addAction(pasteAct);

    toolBar->addSeparator();

    QAction *findAct = new QAction(QIcon(":/icons/search"), "查找", this);
    connect(findAct, &QAction::triggered, this, &MainWindow::showFindDialog);
    toolBar->addAction(findAct);

    QAction *replaceAct = new QAction(QIcon(":/icons/replace"), "替换", this);
    connect(replaceAct, &QAction::triggered, this, &MainWindow::showReplaceDialog);
    toolBar->addAction(replaceAct);
}

void MainWindow::initStatusBar()
{
    statusPosition = new QLabel("行: 1, 列: 1", this);
    statusFileInfo = new QLabel("就绪", this);

    // 设置状态栏样式
    statusBar()->setStyleSheet(
        "QStatusBar {"
        "    background-color: #f0f0f0;"
        "    color: #555555;"
        "    border-top: 1px solid #d0d0d0;"
        "    padding: 2px;"
        "}"
        "QStatusBar::item {"
        "    border: none;"
        "}"
        );
    
    // 设置状态栏标签样式
    statusPosition->setStyleSheet(
        "QLabel {"
        "    color: #555555;"
        "    background: transparent;"
        "    padding: 0 8px;"
        "    font-size: 10pt;"
        "}"
        );
    statusFileInfo->setStyleSheet(
        "QLabel {"
        "    color: #888888;"
        "    background: transparent;"
        "    padding: 0 8px;"
        "    font-size: 10pt;"
        "    font-style: italic;"
        "}"
        );

    statusBar()->addWidget(statusPosition, 1);
    statusBar()->addWidget(statusFileInfo, 2);
}

void MainWindow::initFileList()
{
    // 初始为空，不需要添加任何项目
    fileTree->clear();

    // 创建文件树上下文菜单
    fileTreeContextMenu = new QMenu(this);
    removeFileAct = new QAction("移除(&R)", this);
    openFileLocationAct = new QAction("打开文件位置(&L)", this);

    // 直接创建第一层级的终端菜单项
    openCmdAct = new QAction("打开 CMD 命令行(&C)", this);
    openGitBashAct = new QAction("打开 Git Bash(&G)", this);

    fileTreeContextMenu->addAction(removeFileAct);
    fileTreeContextMenu->addSeparator();
    fileTreeContextMenu->addAction(openFileLocationAct);
    fileTreeContextMenu->addAction(openCmdAct);
    fileTreeContextMenu->addAction(openGitBashAct);

    // 连接上下文菜单动作
    connect(removeFileAct, &QAction::triggered, this, &MainWindow::removeFileFromTree);
    connect(openFileLocationAct, &QAction::triggered, this, &MainWindow::openFileLocation);
    connect(openCmdAct, &QAction::triggered, this, &MainWindow::openCmdAtFile);
    connect(openGitBashAct, &QAction::triggered, this, &MainWindow::openGitBashAtFile);

    // 设置文件树样式 - 亮色主题
    fileTree->setStyleSheet(
        "QTreeWidget {"
        "    border: none;"
        "    background-color: #ffffff;"
        "    outline: 0;"
        "    font-size: 10pt;"
        "    font-family: 'Segoe UI', 'Microsoft YaHei', sans-serif;"
        "}"
        "QTreeWidget::item {"
        "    padding: 5px 8px;"
        "    border-bottom: 1px solid #E0E0E0;"  /* 每个项目之间的分隔线 */
        "    min-height: 28px;"
        "    max-height: 28px;"
        "    color: #555555;"
        "    background-color: #FFFFFF;"  /* 文件默认白色背景 */
        "}"
        "QTreeWidget::item:selected {"
        "    background-color: #00A663;"
        "    color: #ffffff;"
        "    border: none;"
        "    border-bottom: 1px solid #00A663;"  /* 选中时底部边框也变色 */
        "}"
        "QTreeWidget::item:hover:!selected {"
        "    background-color: #f0f0f0;"
        "    color: #333333;"
        "}"
        "QTreeWidget::item:has-children {"
        "    font-weight: bold;"
        "    color: #333333;"
        "    background-color: #EEEEF0;"  /* 文件夹（有子项的项）使用灰色背景 */
        "}"
        "QTreeWidget::item:focus {"
        "    outline: none;"
        "}"
        // 滚动条样式
        "QScrollBar:vertical {"
        "    border: none;"
        "    background: #f0f0f0;"
        "    width: 10px;"
        "    margin: 0px;"
        "}"
        "QScrollBar::handle:vertical {"
        "    background: #c0c0c0;"
        "    min-height: 20px;"
        "    border-radius: 5px;"
        "}"
        "QScrollBar::handle:vertical:hover {"
        "    background: #a0a0a0;"
        "}"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {"
        "    height: 0px;"
        "}"
        // 分支展开/折叠指示器样式
        "QTreeView::branch:has-children:!has-siblings:closed,"
        "QTreeView::branch:closed:has-children:has-siblings {"
        "    border-image: none;"
        "}"
        "QTreeView::branch:open:has-children:!has-siblings,"
        "QTreeView::branch:open:has-children:has-siblings {"
        "    border-image: none;"
        "}"
        );
}

void MainWindow::initConnections()
{
    // 文件操作
    connect(newAct, &QAction::triggered, this, &MainWindow::newFile);

    // 使用lambda解决openFile名称冲突
    connect(openAct, &QAction::triggered, this, [this]() {
        // 获取上次打开的目录，如果没有则使用用户主目录
        QString startDir = m_lastOpenedDir.isEmpty() ? QDir::homePath() : m_lastOpenedDir;

        QString filePath = QFileDialog::getOpenFileName(this, "打开文件",
                                                        startDir,
                                                        "所有文件 (*.*);;"
                                                        "文本文件 (*.txt);;"
                                                        "C++文件 (*.cpp *.h *.hpp);;"
                                                        "Markdown文件 (*.md);;"
                                                        "Python文件 (*.py);;"
                                                        "JavaScript文件 (*.js)");
        if (!filePath.isEmpty()) {
            // 保存文件所在目录
            QFileInfo fileInfo(filePath);
            m_lastOpenedDir = fileInfo.absolutePath();

            openFileFromPath(filePath);
        }
    });

    connect(saveAct, &QAction::triggered, this, &MainWindow::saveFile);
    connect(saveAsAct, &QAction::triggered, this, &MainWindow::saveFileAs);
    connect(closeAct, &QAction::triggered, this, [this]() { closeTab(); });
    connect(exitAct, &QAction::triggered, qApp, &QApplication::quit);

    // 编辑操作
    connect(undoAct, &QAction::triggered, this, &MainWindow::undo);
    connect(redoAct, &QAction::triggered, this, &MainWindow::redo);
    connect(cutAct, &QAction::triggered, this, &MainWindow::cut);
    connect(copyAct, &QAction::triggered, this, &MainWindow::copy);
    connect(pasteAct, &QAction::triggered, this, &MainWindow::paste);

    // 视图操作
    connect(toggleFileListAct, &QAction::triggered, this, [this](bool checked) {
        toggleFileList(checked);
    });
    connect(togglePreviewAct, &QAction::triggered, this, [this](bool checked) {
        togglePreview(checked);
    });

    // 文件列表工具栏按钮
    connect(locateBtn, &QPushButton::clicked, this, &MainWindow::locateCurrentFile);
    connect(expandBtn, &QPushButton::clicked, this, &MainWindow::expandAllFiles);
    connect(foldBtn, &QPushButton::clicked, this, &MainWindow::collapseAllFiles);

    // 帮助
    connect(aboutAct, &QAction::triggered, this, &MainWindow::showAboutDialog);

    // 文件树 - 使用旧的SIGNAL/SLOT语法避免编译错误
    connect(fileTree, SIGNAL(itemClicked(QTreeWidgetItem*, int)),
            this, SLOT(onFileItemClicked(QTreeWidgetItem*, int)));

    // 标签页
    connect(tabWidget, &QTabWidget::tabCloseRequested, this, &MainWindow::closeTab);
    connect(tabWidget, &QTabWidget::currentChanged, this, &MainWindow::currentTabChanged);

    // 右键菜单连接
    connect(fileTree, &QTreeWidget::customContextMenuRequested, this, &MainWindow::showFileTreeContextMenu);
    fileTree->setContextMenuPolicy(Qt::CustomContextMenu);

    QShortcut *findNextShortcut = new QShortcut(QKeySequence("F3"), this);
    connect(findNextShortcut, &QShortcut::activated, this, [this]() {
        if (m_findReplaceDialog && m_findReplaceDialog->isVisible()) {
            m_findReplaceDialog->findNextMatch();
        } else {
            showFindDialog();
        }
    });
    // Shift+F3 查找上一个
    QShortcut *findPrevShortcut = new QShortcut(QKeySequence("Shift+F3"), this);
    connect(findPrevShortcut, &QShortcut::activated, this, [this]() {
        if (m_findReplaceDialog && m_findReplaceDialog->isVisible()) {
            m_findReplaceDialog->findPrevMatch();
        }
    });
}



// 定位当前文件
void MainWindow::locateCurrentFile()
{
    QString filePath = getCurrentFilePath();
    m_fileManage->locateCurrentFile(filePath, fileTree);
}

// 展开所有文件
void MainWindow::expandAllFiles()
{
    m_fileManage->expandAllFiles(fileTree);
}

// 折叠所有文件
void MainWindow::collapseAllFiles()
{
    m_fileManage->collapseAllFiles(fileTree);
}



QPlainTextEdit* MainWindow::createEditor()
{
    LineNumberEditor *editor = new LineNumberEditor();

    // 连接文本改变信号
    connect(editor, &QPlainTextEdit::textChanged, this, &MainWindow::onTextChanged);
    connect(editor, &QPlainTextEdit::cursorPositionChanged, this, &MainWindow::updateStatusBar);
    connect(editor, &QPlainTextEdit::undoAvailable, undoAct, &QAction::setEnabled);
    connect(editor, &QPlainTextEdit::redoAvailable, redoAct, &QAction::setEnabled);
    connect(editor, &QPlainTextEdit::copyAvailable, cutAct, &QAction::setEnabled);
    connect(editor, &QPlainTextEdit::copyAvailable, copyAct, &QAction::setEnabled);

    // 为编辑器安装事件过滤器
    editor->installEventFilter(this);

    // 设置编辑器的上下文菜单
    editor->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(editor, &QPlainTextEdit::customContextMenuRequested, this, &MainWindow::showEditorContextMenu);

    return editor;
}





// 点击左侧文件树项
void MainWindow::onFileItemClicked(QTreeWidgetItem *item, int column)
{
    Q_UNUSED(column);

    if (!item) return;

    QString filePath = item->data(0, Qt::UserRole).toString();
    if (filePath.isEmpty()) {
        // 目录项，展开/折叠即可
        item->setExpanded(!item->isExpanded());
        return;
    }

    // 检查文件是否存在
    if (!QFile::exists(filePath)) {
        QMessageBox::warning(this, "文件不存在", "文件已不存在或已被移动: " + filePath);
        m_fileManage->removeFileFromTree(filePath, fileTree);
        return;
    }

    // 检查是否已经在标签页中打开 - 使用完整路径比较
    for (int i = 0; i < tabWidget->count(); ++i) {
        QPlainTextEdit *editor = qobject_cast<QPlainTextEdit*>(tabWidget->widget(i));
        if (editor && editor->property("filePath").toString() == filePath) {
            tabWidget->setCurrentIndex(i);
            return;
        }
    }

    // 打开文件
    openFileFromPath(filePath);
}

// 打开文件（重载函数）
void MainWindow::openFileFromPath(const QString &filePath)
{
    // 检查是否已经打开
    for (int i = 0; i < tabWidget->count(); ++i) {
        QPlainTextEdit *editor = qobject_cast<QPlainTextEdit*>(tabWidget->widget(i));
        if (editor && editor->property("filePath").toString() == filePath) {
            tabWidget->setCurrentIndex(i);
            return;
        }
    }

    QPlainTextEdit *editor = createEditor();
    if (!m_fileManage->openFile(filePath, editor)) {
        QMessageBox::warning(this, "错误", "无法打开文件: " + filePath);
        delete editor;
        return;
    }

    // 如果是LineNumberEditor，设置文件名以启用语法高亮
    LineNumberEditor *lineNumberEditor = qobject_cast<LineNumberEditor*>(editor);
    if (lineNumberEditor) {
        lineNumberEditor->setFileName(filePath);
    }

    // 根据文件类型设置是否显示缩进参考线
    QStringList codeExtensions = {
        "cpp", "h", "hpp", "hxx", "c", "cc",
        "py", "pyw", "pyi",
        "java", "js", "jsx", "ts", "tsx",
        "cs", "rs", "go", "swift", "kt", "kts",
        "yaml", "yml", "json", "xml", "sql"
    };

    bool isCodeFile = false;
    for (const QString &ext : codeExtensions) {
        if (filePath.endsWith("." + ext)) {
            isCodeFile = true;
            break;
        }
    }

    if (lineNumberEditor) {
        lineNumberEditor->setShowIndentationGuides(isCodeFile);
    }

    QFileInfo fileInfo(filePath);
    int index = tabWidget->addTab(editor, fileInfo.fileName());
    tabWidget->setCurrentIndex(index);

    // 添加到左侧文件树
    m_fileManage->addFileToList(filePath, fileTree);

    // 在树中选中该文件
    m_fileManage->selectFileInTree(filePath, fileTree);

    // 保存当前打开的文件列表
    QStringList openFiles;
    for (int i = 0; i < tabWidget->count(); ++i) {
        QPlainTextEdit *ed = qobject_cast<QPlainTextEdit*>(tabWidget->widget(i));
        if (ed) {
            QString fp = ed->property("filePath").toString();
            if (!fp.isEmpty()) {
                openFiles.append(fp);
            }
        }
    }
    m_fileManage->saveFileList(openFiles, m_fileManage->getRecentFiles());

    saveAct->setEnabled(false);
    saveAsAct->setEnabled(true);
    closeAct->setEnabled(true);

    updateWindowTitle();
    updateStatusBar();
}

// 槽函数实现
void MainWindow::newFile()
{
    QPlainTextEdit *editor = createEditor();
    int index = tabWidget->addTab(editor, "无标题");
    tabWidget->setCurrentIndex(index);

    // 如果是LineNumberEditor，清除语法高亮
    LineNumberEditor *lineNumberEditor = qobject_cast<LineNumberEditor*>(editor);
    if (lineNumberEditor) {
        lineNumberEditor->setFileName("");
    }

    // 启用保存和关闭按钮
    saveAct->setEnabled(false);
    saveAsAct->setEnabled(true);
    closeAct->setEnabled(true);

    updateWindowTitle();
    updateStatusBar();
}

bool MainWindow::saveFile()
{
    QPlainTextEdit *editor = getCurrentEditor();
    if (!editor) return false;

    QString filePath = editor->property("filePath").toString();
    if (filePath.isEmpty()) {
        return saveFileAs();
    }

    if (m_fileManage->saveFile(editor)) {
        saveAct->setEnabled(false);
        updateWindowTitle();
        statusBar()->showMessage("文件保存成功", 2000);
        return true;
    } else {
        QMessageBox::warning(this, "错误", "无法保存文件: " + filePath);
        return false;
    }
}

bool MainWindow::saveFileAs()
{
    QPlainTextEdit *editor = getCurrentEditor();
    if (!editor) return false;

    // 使用上次打开的目录，如果没有则使用用户主目录
    QString startDir = m_lastOpenedDir.isEmpty() ? QDir::homePath() : m_lastOpenedDir;

    QString filePath = QFileDialog::getSaveFileName(this, "另存为",
                                                    startDir,
                                                    "所有文件 (*.*);;"
                                                    "文本文件 (*.txt);;"
                                                    "C++文件 (*.cpp *.h *.hpp);;"
                                                    "Markdown文件 (*.md)");
    if (filePath.isEmpty()) {
        return false;
    }

    editor->setProperty("filePath", filePath);
    QFileInfo fileInfo(filePath);
    m_lastOpenedDir = fileInfo.absolutePath();
    tabWidget->setTabText(tabWidget->currentIndex(), fileInfo.fileName());

    // 设置文件名以启用语法高亮
    LineNumberEditor *lineNumberEditor = qobject_cast<LineNumberEditor*>(editor);
    if (lineNumberEditor) {
        lineNumberEditor->setFileName(filePath);
    }

    // 添加到左侧文件树
    m_fileManage->addFileToList(filePath, fileTree);

    // 保存文件内容
    if (m_fileManage->writeFileContent(filePath, editor->toPlainText())) {
        editor->document()->setModified(false);
        saveAct->setEnabled(false);
        updateWindowTitle();
        statusBar()->showMessage("文件保存成功", 2000);
        
        // 添加到最近文件列表
        m_fileManage->addToRecentFiles(filePath);
        
        // 保存文件列表
        QStringList openFiles;
        for (int i = 0; i < tabWidget->count(); ++i) {
            QPlainTextEdit *ed = qobject_cast<QPlainTextEdit*>(tabWidget->widget(i));
            if (ed) {
                QString fp = ed->property("filePath").toString();
                if (!fp.isEmpty()) {
                    openFiles.append(fp);
                }
            }
        }
        m_fileManage->saveFileList(openFiles, m_fileManage->getRecentFiles());
        
        return true;
    } else {
        QMessageBox::warning(this, "错误", "无法保存文件: " + filePath);
        return false;
    }
}

void MainWindow::closeTab(int index)
{
    if (index == -1) {
        index = tabWidget->currentIndex();
    }

    if (index < 0) return;

    QPlainTextEdit *editor = qobject_cast<QPlainTextEdit*>(tabWidget->widget(index));
    if (editor && editor->document()->isModified()) {
        if (!maybeSave()) {
            return;
        }
    }

    // 获取文件路径，准备从树中移除
    QString filePath = editor ? editor->property("filePath").toString() : QString();

    tabWidget->removeTab(index);
    delete editor;

    // 更新按钮状态
    updateButtonStates();

    updateWindowTitle();
    updateStatusBar();
    updateEditActions();

    // 保存更新后的文件列表
    QStringList openFiles;
    for (int i = 0; i < tabWidget->count(); ++i) {
        QPlainTextEdit *ed = qobject_cast<QPlainTextEdit*>(tabWidget->widget(i));
        if (ed) {
            QString fp = ed->property("filePath").toString();
            if (!fp.isEmpty()) {
                openFiles.append(fp);
            }
        }
    }
    m_fileManage->saveFileList(openFiles, m_fileManage->getRecentFiles());
}

void MainWindow::updateButtonStates()
{
    bool hasTabs = (tabWidget->count() > 0);
    bool hasEditor = getCurrentEditor() != nullptr;

    // 文件操作按钮
    saveAct->setEnabled(hasEditor && hasTabs);
    saveAsAct->setEnabled(hasEditor && hasTabs);
    closeAct->setEnabled(hasEditor && hasTabs);

    // 如果没有打开的标签页，禁用所有编辑相关操作
    if (!hasTabs || !hasEditor) {
        undoAct->setEnabled(false);
        redoAct->setEnabled(false);
        cutAct->setEnabled(false);
        copyAct->setEnabled(false);
    }
}

void MainWindow::showFindDialog()
{
    qDebug() << "showFindDialog() called";

    if (!m_findReplaceDialog) {
        qDebug() << "Creating new FindReplaceDialog";
        m_findReplaceDialog = new FindReplaceDialog(this);
    }

    QPlainTextEdit *editor = getCurrentEditor();
    qDebug() << "Current editor:" << editor;

    if (editor) {
        m_findReplaceDialog->setTextEdit(editor);
        m_findReplaceDialog->showFind();
    } else {
        QMessageBox msgBox(this);
        msgBox.setWindowTitle("提示");
        msgBox.setText("请先打开一个文件");
        msgBox.setIcon(QMessageBox::Information);
        msgBox.setStandardButtons(QMessageBox::Ok);

        // 使用样式表强制设置标题标签的宽度
        msgBox.setStyleSheet(
            "QLabel#qt_msgbox_label { min-width: 150px; }"
            );

        msgBox.exec();
    }
}

void MainWindow::showReplaceDialog()
{
    qDebug() << "showReplaceDialog() called - ";

    if (!m_findReplaceDialog) {
        qDebug() << "Creating new FindReplaceDialog";
        m_findReplaceDialog = new FindReplaceDialog(this);
    }

    QPlainTextEdit *editor = getCurrentEditor();
    qDebug() << "Current editor:" << editor;

    if (editor) {
        m_findReplaceDialog->setTextEdit(editor);
        m_findReplaceDialog->showReplace();
    } else {
        QMessageBox msgBox(this);
        msgBox.setWindowTitle("提示");
        msgBox.setText("请先打开一个文件");
        msgBox.setIcon(QMessageBox::Information);
        msgBox.setStandardButtons(QMessageBox::Ok);

        // 使用样式表强制设置标题标签的宽度
        msgBox.setStyleSheet(
            "QLabel#qt_msgbox_label { min-width: 150px; }"
            );

        msgBox.exec();
    }

}

void MainWindow::openFileFromCommandLine(const QString &filePath)
{
    // 检查文件是否存在
    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists() || !fileInfo.isFile()) {
        qDebug() << "文件不存在:" << filePath;
        return;
    }

    // 获取绝对路径
    QString absolutePath = fileInfo.absoluteFilePath();

    // 检查文件是否已经在标签页中打开
    for (int i = 0; i < tabWidget->count(); ++i) {
        QPlainTextEdit *editor = qobject_cast<QPlainTextEdit*>(tabWidget->widget(i));
        if (editor && editor->property("filePath").toString() == absolutePath) {
            // 如果已打开，切换到该标签页
            tabWidget->setCurrentIndex(i);

            // 在文件树中定位
            m_fileManage->selectFileInTree(absolutePath, fileTree);
            return;
        }
    }

    // 打开文件 - 直接调用openFileFromPath，它已经集成了FileManage
    openFileFromPath(absolutePath);

    // 确保窗口显示在前台
    raise();
    activateWindow();
}

void MainWindow::undo()
{
    QPlainTextEdit *editor = getCurrentEditor();
    if (editor) editor->undo();
}

void MainWindow::redo()
{
    QPlainTextEdit *editor = getCurrentEditor();
    if (editor) editor->redo();
}

void MainWindow::cut()
{
    QPlainTextEdit *editor = getCurrentEditor();
    if (editor) editor->cut();
}

void MainWindow::copy()
{
    QPlainTextEdit *editor = getCurrentEditor();
    if (editor) editor->copy();
}

void MainWindow::paste()
{
    QPlainTextEdit *editor = getCurrentEditor();
    if (editor) editor->paste();
}

void MainWindow::toggleFileList(bool visible)
{
    leftPanel->setVisible(visible);
    if (!visible) {
        mainSplitter->setSizes(QList<int>() << 0 << 800 << (previewVisible ? 300 : 0));
    } else {
        mainSplitter->setSizes(QList<int>() << 200 << 600 << (previewVisible ? 300 : 0));
    }
}

void MainWindow::togglePreview(bool visible)
{
    previewVisible = visible;
    previewPanel->setVisible(visible);

    if (visible) {
        mainSplitter->setSizes(QList<int>() << 200 << 500 << 300);
    } else {
        mainSplitter->setSizes(QList<int>() << 200 << 600 << 0);
    }
}

void MainWindow::onTextChanged()
{
    QPlainTextEdit *editor = getCurrentEditor();
    if (editor) {
        bool modified = editor->document()->isModified();
        saveAct->setEnabled(modified);

        // 更新窗口标题的修改标记
        updateWindowTitle();

        // 如果是Markdown文件，更新预览
        if (previewVisible) {
            QString filePath = getCurrentFilePath();
            if (filePath.endsWith(".md")) {
                previewPanel->setPlainText(editor->toPlainText());
            }
        }
    }
}

void MainWindow::updateWindowTitle()
{
    if (tabWidget->count() == 0) {
        setWindowTitle("INK");
        return;
    }

    QPlainTextEdit *editor = getCurrentEditor();
    if (!editor) {
        setWindowTitle("INK");
        return;
    }

    QString filePath = getCurrentFilePath();
    QString title = "INK";

    if (filePath.isEmpty()) {
        title += " - 无标题";
    } else {
        title += " - " + m_fileManage->removeFileExtension(QFileInfo(filePath).fileName());
    }

    if (editor->document()->isModified()) {
        title += " *";
    }

    setWindowTitle(title);
}

void MainWindow::updateStatusBar()
{
    if (tabWidget->count() == 0) {
        statusPosition->setText("就绪");
        statusFileInfo->setText("未打开任何文件");
        return;
    }

    QPlainTextEdit *editor = getCurrentEditor();
    if (!editor) {
        statusPosition->setText("就绪");
        statusFileInfo->setText("");
        return;
    }

    QTextCursor cursor = editor->textCursor();
    int line = cursor.blockNumber() + 1;
    int column = cursor.columnNumber() + 1;
    int totalLines = editor->document()->blockCount();

    statusPosition->setText(QString("行: %1, 列: %2, 总行: %3").arg(line).arg(column).arg(totalLines));

    QString filePath = getCurrentFilePath();
    if (filePath.isEmpty()) {
        statusFileInfo->setText("未保存");
    } else {
        QFileInfo info(filePath);
        statusFileInfo->setText(m_fileManage->removeFileExtension(info.fileName()) + " | " +
                                QString::number(info.size() / 1024.0, 'f', 2) + " KB");
    }
}

void MainWindow::currentTabChanged(int index)
{
    // 如果没有标签页，不执行后续操作
    if (index < 0) {
        updateWindowTitle();
        updateStatusBar();
        updateEditActions();
        return;
    }

    updateWindowTitle();
    updateStatusBar();
    updateEditActions();

    // 切换标签时，自动定位到树中的对应文件
    QString filePath = getCurrentFilePath();
    if (!filePath.isEmpty()) {
        m_fileManage->selectFileInTree(filePath, fileTree);  // 使用完整路径定位
    }

    // 更新查找对话框的编辑器
    if (m_findReplaceDialog && m_findReplaceDialog->isVisible()) {
        m_findReplaceDialog->setTextEdit(getCurrentEditor());
    }
}

void MainWindow::updateEditActions()
{
    if (tabWidget->count() == 0) {
        undoAct->setEnabled(false);
        redoAct->setEnabled(false);
        cutAct->setEnabled(false);
        copyAct->setEnabled(false);
        pasteAct->setEnabled(false);
        return;
    }

    QPlainTextEdit *editor = getCurrentEditor();
    if (editor) {
        cutAct->setEnabled(editor->textCursor().hasSelection());
        copyAct->setEnabled(editor->textCursor().hasSelection());
    }
}

void MainWindow::showAboutDialog()
{
    QMessageBox::about(this, "关于 ink",
                       "<h2>ink 文本编辑器</h2>"
                       "<p>版本 0.1.0</p>"
                       "<p>一个简洁、有序的跨平台文本编辑器</p>"
                       "<p>功能特点：</p>"
                       "<ul>"
                       "<li>多标签页编辑</li>"
                       "<li>树形文件列表（按目录组织）</li>"
                       "<li>Markdown预览</li>"
                       "<li>代码高亮（待实现）</li>"
                       "<li>跨平台支持</li>"
                       "</ul>"
                       "<p>基于 Qt " QT_VERSION_STR " 构建</p>");
}

// 工具函数实现
bool MainWindow::maybeSave()
{
    QPlainTextEdit *editor = getCurrentEditor();
    if (!editor || !editor->document()->isModified()) {
        return true;
    }

    QString filePath = editor->property("filePath").toString();
    QString fileName = filePath.isEmpty() ? "无标题" : QFileInfo(filePath).fileName();

    QMessageBox::StandardButton ret;
    ret = QMessageBox::warning(this, "INK",
                               QString("文档 \"%1\" 已被修改，是否保存更改？").arg(fileName),
                               QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);

    if (ret == QMessageBox::Save) {
        return saveFile();
    } else if (ret == QMessageBox::Cancel) {
        return false;
    }

    return true;
}

QString MainWindow::getCurrentFilePath() const
{
    QPlainTextEdit *editor = getCurrentEditor();
    if (editor) {
        return editor->property("filePath").toString();
    }
    return QString();
}

QPlainTextEdit* MainWindow::getCurrentEditor() const
{
    if (tabWidget->count() == 0) {
        return nullptr;
    }
    return qobject_cast<QPlainTextEdit*>(tabWidget->currentWidget());
}

// 获取选中的文件路径
QString MainWindow::getSelectedFilePath() const
{
    QTreeWidgetItem *item = fileTree->currentItem();
    if (item) {
        QString filePath = item->data(0, Qt::UserRole).toString();
        if (!filePath.isEmpty()) {
            return filePath;
        }
    }
    return QString();
}

// 检查是否为可执行文件
bool MainWindow::isExecutableFile(const QString &filePath) const
{
    QFileInfo fileInfo(filePath);
    QString suffix = fileInfo.suffix().toLower();
    
    // Windows 可执行文件
    if (suffix == "exe" || suffix == "bat" || suffix == "cmd" || suffix == "com" || suffix == "msi") {
        return true;
    }
    
    // 脚本文件
    if (suffix == "sh" || suffix == "bash" || suffix == "ps1") {
        return true;
    }
    
    // 检查文件是否具有可执行权限（在 Unix 系统上）
    return fileInfo.isExecutable();
}

// 文件树上下文菜单
void MainWindow::showFileTreeContextMenu(const QPoint &pos)
{
    QTreeWidgetItem *item = fileTree->itemAt(pos);
    if (!item) return;

    QString filePath = item->data(0, Qt::UserRole).toString();
    if (filePath.isEmpty()) return; // 目录项，不显示菜单

    m_contextMenuFilePath = filePath;

    fileTreeContextMenu->exec(fileTree->viewport()->mapToGlobal(pos));
}

// 编辑器上下文菜单（暂时使用文件树的菜单）
void MainWindow::showEditorContextMenu(const QPoint &pos)
{
    QPlainTextEdit *editor = getCurrentEditor();
    if (!editor) return;

    QString filePath = editor->property("filePath").toString();
    if (filePath.isEmpty()) return;

    m_contextMenuFilePath = filePath;

    fileTreeContextMenu->exec(editor->viewport()->mapToGlobal(pos));
}

// 从文件树中移除文件
// 从文件树中移除文件
void MainWindow::removeFileFromTree()
{
    if (m_contextMenuFilePath.isEmpty()) return;

    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "移除文件",
                                  "确定要从文件列表中移除该文件吗？\n此操作不会删除实际文件。",
                                  QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        // 先检查并关闭编辑器中打开的文件
        for (int i = 0; i < tabWidget->count(); ++i) {
            QPlainTextEdit *editor = qobject_cast<QPlainTextEdit*>(tabWidget->widget(i));
            if (editor && editor->property("filePath").toString() == m_contextMenuFilePath) {
                // 如果文件已修改，提示保存
                if (editor->document()->isModified()) {
                    tabWidget->setCurrentIndex(i);
                    if (!maybeSave()) {
                        return;  // 用户取消操作
                    }
                }
                // 关闭标签页
                tabWidget->removeTab(i);
                delete editor;
                break;  // 找到并关闭后退出循环
            }
        }

        // 从文件树中移除
        m_fileManage->removeFileFromTree(m_contextMenuFilePath, fileTree);

        // 更新UI状态
        updateButtonStates();
        updateWindowTitle();
        updateStatusBar();
        updateEditActions();

        // 保存更新后的文件列表
        QStringList openFiles;
        for (int i = 0; i < tabWidget->count(); ++i) {
            QPlainTextEdit *ed = qobject_cast<QPlainTextEdit*>(tabWidget->widget(i));
            if (ed) {
                QString fp = ed->property("filePath").toString();
                if (!fp.isEmpty()) {
                    openFiles.append(fp);
                }
            }
        }
        m_fileManage->saveFileList(openFiles, m_fileManage->getRecentFiles());
    }
}

// 打开文件位置
void MainWindow::openFileLocation()
{
    if (m_contextMenuFilePath.isEmpty()) return;
    
    QFileInfo fileInfo(m_contextMenuFilePath);
    if (!fileInfo.exists()) {
        QMessageBox::warning(this, "错误", "文件不存在或已被移动。");
        return;
    }
    
    qDebug() << "openFileLocation: filePath =" << m_contextMenuFilePath;
    qDebug() << "fileInfo.exists() =" << fileInfo.exists();
    qDebug() << "absoluteFilePath =" << fileInfo.absoluteFilePath();
    qDebug() << "absolutePath =" << fileInfo.absolutePath();
    
#ifdef Q_OS_WINDOWS
    // Windows: 在资源管理器中打开文件所在目录并选中文件
    // explorer /select,"C:\path\to\file with spaces.txt"
    QString nativePath = QDir::toNativeSeparators(fileInfo.absoluteFilePath());
    QString command = "explorer";
    QStringList args;
    args << "/select," << nativePath;  // 分开传递两个参数
    qDebug() << "Windows explorer command: explorer /select,\"" + nativePath + "\"";
    
    // 使用 QProcess::startDetached 启动
    bool success = QProcess::startDetached(command, args);
    if (!success) {
        qDebug() << "Failed to start explorer process";
        // 备选方案：打开目录
        QDesktopServices::openUrl(QUrl::fromLocalFile(fileInfo.absolutePath()));
    }
#elif defined(Q_OS_MAC)
    // macOS 使用 open 命令
    QString command = "open -R \"" + fileInfo.absoluteFilePath() + "\"";
    qDebug() << "macOS open command:" << command;
    QProcess::startDetached(command);
#else
    // Linux 使用文件管理器
    QString dirPath = fileInfo.absolutePath();
    QString command = "xdg-open \"" + dirPath + "\"";
    qDebug() << "Linux xdg-open command:" << command;
    QProcess::startDetached(command);
#endif
}

// 在文件位置打开CMD命令行
void MainWindow::openCmdAtFile()
{
    if (m_contextMenuFilePath.isEmpty()) return;

    QFileInfo fileInfo(m_contextMenuFilePath);
    QString dirPath = fileInfo.absolutePath();

    if (!QDir(dirPath).exists()) {
        QMessageBox::warning(this, "错误", "目录不存在或无法访问。");
        return;
    }

#ifdef Q_OS_WINDOWS
    QString nativePath = QDir::toNativeSeparators(dirPath);

    // 使用 start 命令打开新的 CMD 窗口
    QStringList args;
    args << "/C" << "start" << "cmd.exe" << "/K" << QString("cd /d %1").arg(nativePath);

    QProcess::startDetached("cmd.exe", args);

#elif defined(Q_OS_MAC)
    QString command = "open -a Terminal \"" + dirPath + "\"";
    QProcess::startDetached(command);
#else
    QString terminal = qEnvironmentVariable("TERM", "xterm");
    QString command = QString("%1 -e 'cd \"%2\"; bash'").arg(terminal).arg(dirPath);
    QProcess::startDetached(command);
#endif
}

// 在文件位置打开Git Bash
void MainWindow::openGitBashAtFile()
{
    if (m_contextMenuFilePath.isEmpty()) return;

    QFileInfo fileInfo(m_contextMenuFilePath);
    QString dirPath = fileInfo.absolutePath();

    if (!QDir(dirPath).exists()) {
        QMessageBox::warning(this, "错误", "目录不存在或无法访问。");
        return;
    }

    // 获取 Git Bash 路径
    QString gitBashPath = getGitBashPath();

#ifdef Q_OS_WINDOWS
    // 检查是否配置过 Git Bash 路径
    if (gitBashPath.isEmpty()) {
        // 未配置，提示用户配置
        QMessageBox::StandardButton reply = QMessageBox::question(
            this,
            "配置 Git Bash",
            "您还未配置 Git Bash 路径，是否现在配置？",
            QMessageBox::Yes | QMessageBox::No
            );

        if (reply == QMessageBox::Yes) {
            configureGitBashPath();
            // 重新获取配置后的路径
            gitBashPath = getGitBashPath();
            if (gitBashPath.isEmpty()) {
                return; // 用户取消配置
            }
        } else {
            return;
        }
    }

    // 检查配置的路径是否有效
    if (!QFile::exists(gitBashPath)) {
        QMessageBox::StandardButton reply = QMessageBox::warning(
            this,
            "路径无效",
            QString("配置的 Git Bash 路径无效:\n%1\n\n是否重新配置？").arg(gitBashPath),
            QMessageBox::Yes | QMessageBox::No
            );

        if (reply == QMessageBox::Yes) {
            configureGitBashPath();
            // 重新获取配置后的路径
            gitBashPath = getGitBashPath();
            if (gitBashPath.isEmpty() || !QFile::exists(gitBashPath)) {
                return;
            }
        } else {
            return;
        }
    }

    QString nativePath = QDir::toNativeSeparators(dirPath);

    qDebug() << "Git Bash 路径:" << gitBashPath;
    qDebug() << "目标目录:" << nativePath;

    QStringList args;
    args << "--cd=" + nativePath;

    bool started = QProcess::startDetached(gitBashPath, args);

    qDebug() << "启动结果:" << (started ? "成功" : "失败");

    if (!started) {
        QMessageBox msgBox(this);
        msgBox.setWindowTitle("启动失败");
        msgBox.setText(QString("无法启动 Git Bash\n\n路径: %1\n目录: %2\n\n是否重新配置路径？")
                           .arg(gitBashPath)
                           .arg(nativePath));
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setIcon(QMessageBox::Warning);

        if (msgBox.exec() == QMessageBox::Yes) {
            configureGitBashPath();
        }
    }

#elif defined(Q_OS_MAC)
    QString command = "open -a Terminal \"" + dirPath + "\"";
    QProcess::startDetached(command);
#else
    QString terminal = qEnvironmentVariable("TERM", "xterm");
    QString command = QString("%1 -e 'cd \"%2\"; bash'").arg(terminal).arg(dirPath);
    QProcess::startDetached(command);
#endif
}

// 配置 Git Bash 路径
void MainWindow::configureGitBashPath()
{
    // 默认路径
    QString defaultPath = m_gitBashPath.isEmpty() ?
                              "C:\\0101\\install\\dev\\Git\\git-bash.exe" :
                              m_gitBashPath;

    // 创建对话框
    QDialog dialog(this);
    dialog.setWindowTitle("配置 Git Bash 路径");
    dialog.setFixedSize(500, 120);
    dialog.setStyleSheet(
        "QDialog {"
        "    background-color: #f5f5f5;"
        "}"
        "QLabel {"
        "    color: #333333;"
        "    font-size: 10pt;"
        "}"
        "QLineEdit {"
        "    padding: 5px;"
        "    border: 1px solid #d0d0d0;"
        "    border-radius: 3px;"
        "    background-color: #ffffff;"
        "    color: #333333;"
        "    font-size: 10pt;"
        "}"
        "QPushButton {"
        "    padding: 5px 15px;"
        "    background-color: #00A663;"
        "    color: white;"
        "    border: none;"
        "    border-radius: 3px;"
        "    font-size: 10pt;"
        "}"
        "QPushButton:hover {"
        "    background-color: #008052;"
        "}"
        );

    QVBoxLayout *layout = new QVBoxLayout(&dialog);
    layout->setSpacing(10);
    layout->setContentsMargins(20, 20, 20, 20);

    QLabel *label = new QLabel("请输入 Git Bash (git-bash.exe) 的完整路径：", &dialog);
    layout->addWidget(label);

    QHBoxLayout *inputLayout = new QHBoxLayout();
    QLineEdit *pathEdit = new QLineEdit(defaultPath, &dialog);
    QPushButton *browseBtn = new QPushButton("浏览...", &dialog);
    browseBtn->setFixedWidth(80);
    pathEdit->setFixedHeight(25);

    inputLayout->addWidget(pathEdit);
    inputLayout->addWidget(browseBtn);
    layout->addLayout(inputLayout);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();

    QPushButton *okBtn = new QPushButton("确定", &dialog);
    QPushButton *cancelBtn = new QPushButton("取消", &dialog);
    okBtn->setFixedWidth(80);
    cancelBtn->setFixedHeight(25);
    cancelBtn->setStyleSheet(
        "QPushButton {"
        "    background-color: #f0f0f0;"
        "    color: #333333;"
        "    border: 1px solid #d0d0d0;"
        "}"
        "QPushButton:hover {"
        "    background-color: #e0e0e0;"
        "}"
        );

    buttonLayout->addWidget(okBtn);
    buttonLayout->addWidget(cancelBtn);
    layout->addLayout(buttonLayout);

    // 浏览按钮点击事件
    connect(browseBtn, &QPushButton::clicked, [&]() {
        QString filePath = QFileDialog::getOpenFileName(
            &dialog,
            "选择 Git Bash 可执行文件",
            QFileInfo(pathEdit->text()).absolutePath(),
            "可执行文件 (*.exe);;所有文件 (*.*)"
            );
        if (!filePath.isEmpty()) {
            pathEdit->setText(filePath);
        }
    });

    // 确定按钮
    connect(okBtn, &QPushButton::clicked, &dialog, &QDialog::accept);
    connect(cancelBtn, &QPushButton::clicked, &dialog, &QDialog::reject);

    if (dialog.exec() == QDialog::Accepted) {
        QString newPath = pathEdit->text().trimmed();

        if (!newPath.isEmpty()) {
            // 验证路径
            if (!QFile::exists(newPath)) {
                QMessageBox::warning(this, "路径无效",
                                     QString("指定的文件不存在:\n%1\n\n请确认路径是否正确。").arg(newPath));
                return;
            }

            // 保存到设置
            m_gitBashPath = newPath;
            QSettings settings("INK", "Settings");
            settings.setValue("GitBashPath", m_gitBashPath);
            settings.sync();

            QMessageBox::information(this, "配置成功",
                                     QString("Git Bash 路径已保存:\n%1").arg(m_gitBashPath));
        }
    }
}


// 获取 Git Bash 路径（会检查配置）
QString MainWindow::getGitBashPath()
{
    QSettings settings("INK", "Settings");
    QString savedPath = settings.value("GitBashPath", "").toString();

    // 如果已配置且路径存在
    if (!savedPath.isEmpty() && QFile::exists(savedPath)) {
        return savedPath;
    }

    // 如果已配置但路径不存在
    if (!savedPath.isEmpty() && !QFile::exists(savedPath)) {
        return savedPath; // 返回已保存的路径，让调用者处理错误
    }

    return QString(); // 未配置
}

