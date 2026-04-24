#include "mainwindow.h"
#include "linenumbereditor.h"
#include "findreplacedialog.h"
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
#include <QShortcut>

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QStringConverter>
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
{
    // 设置窗口图标
    setWindowIcon(QIcon(":/icons/icon"));
    // 设置窗口属性
    setWindowTitle("ink - 无标题");
    resize(1200, 800);

    // 初始化图标映射
    initIconMap();

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
    QSettings settings("ink", "Settings");
    restoreGeometry(settings.value("geometry").toByteArray());
    mainSplitter->restoreState(settings.value("splitterState").toByteArray());
    previewVisible = settings.value("previewVisible", false).toBool();
    togglePreviewAct->setChecked(previewVisible);

    // 初始状态：没有打开的文件
    updateWindowTitle();
    updateStatusBar();
    updateEditActions();

    // 创建查找替换对话框（延迟创建）
    m_findReplaceDialog = new FindReplaceDialog(this);

    // 加载上次打开的文件列表
    loadFileList();
}

MainWindow::~MainWindow()
{
    // 保存设置
    QSettings settings("ink", "Settings");
    settings.setValue("geometry", saveGeometry());
    settings.setValue("splitterState", mainSplitter->saveState());
    settings.setValue("previewVisible", previewVisible);

    // 保存文件列表
    saveFileList();

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
    toolbarWidget->setFixedHeight(25);  // 设置与右侧导航栏相同的高度
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
        "    border-radius: 2px;"
        "    padding: 1px;"
        "}"
        "QPushButton:hover {"
        "    background-color: #d0d0d0;"  // 悬停时显示背景
        "}"
        "QPushButton:pressed {"
        "    background-color: #c0c0c0;"  // 按下时显示背景
        "}"
        );

    expandBtn = new QPushButton(toolbarWidget);
    expandBtn->setIcon(QIcon(":/icons/expand"));
    expandBtn->setFixedSize(24, 20);
    expandBtn->setStyleSheet(
        "QPushButton {"
        "    background-color: transparent;"
        "    border: none;"
        "    border-radius: 2px;"
        "    padding: 1px;"
        "}"
        "QPushButton:hover {"
        "    background-color: #d0d0d0;"
        "}"
        "QPushButton:pressed {"
        "    background-color: #c0c0c0;"
        "}"
        );

    foldBtn = new QPushButton(toolbarWidget);
    foldBtn->setIcon(QIcon(":/icons/fold"));
    foldBtn->setFixedSize(24, 20);
    foldBtn->setStyleSheet(
        "QPushButton {"
        "    background-color: transparent;"
        "    border: none;"
        "    border-radius: 2px;"
        "    padding: 1px;"
        "}"
        "QPushButton:hover {"
        "    background-color: #d0d0d0;"
        "}"
        "QPushButton:pressed {"
        "    background-color: #c0c0c0;"
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
    tabWidget->setStyleSheet("QTabBar::tab { height: 25px; }");

    centerLayout->addWidget(tabWidget);

    // 预览面板（初始隐藏）
    previewPanel = new QTextEdit(mainSplitter);
    previewPanel->setReadOnly(true);
    previewPanel->setVisible(previewVisible);
    previewPanel->setMinimumWidth(300);

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
        "    spacing: 2px;"
        "    background-color: #f0f0f0;"
        "    border: none;"
        "    border-bottom: 1px solid #d0d0d0;"
        "}"
        "QToolBar QToolButton {"
        "    padding: 2px;"
        "    border-radius: 2px;"
        "    margin: 1px;"
        "}"
        "QToolBar QToolButton:hover {"
        "    background-color: #d0d0d0;"
        "}"
        "QToolBar QToolButton:pressed {"
        "    background-color: #c0c0c0;"
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

    statusBar()->addWidget(statusPosition, 1);
    statusBar()->addWidget(statusFileInfo, 2);
}

void MainWindow::initFileList()
{
    // 初始为空，不需要添加任何项目
    fileTree->clear();
    filePathMap.clear();

    // 设置文件树样式 - 移除标题后的样式
    fileTree->setStyleSheet(
        "QTreeWidget {"
        "    border: none;"
        "    background-color: #ffffff;"
        "    outline: 0;"
        "    font-size: 11pt;"
        "}"
        "QTreeWidget::item {"
        "    padding: 3px 2px;"  // 更紧凑的间距
        "    border-bottom: 1px solid transparent;"
        "    min-height: 20px;"    // 设置最小高度
        "    max-height: 20px;"    // 设置最大高度
        "}"
        "QTreeWidget::item:selected {"
        "    background-color: #D1D1D1;"
        "    color: #000000;"
        "    border: none;"
        "}"
        "QTreeWidget::item:hover:!selected {"
        "    background-color: #EAEAEA;"
        "    color: #000000;"
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
        );
}

void MainWindow::initConnections()
{
    // 文件操作
    connect(newAct, &QAction::triggered, this, &MainWindow::newFile);

    // 使用lambda解决openFile名称冲突
    connect(openAct, &QAction::triggered, this, [this]() {
        QString filePath = QFileDialog::getOpenFileName(this, "打开文件",
                                                        QDir::homePath(),
                                                        "所有文件 (*.*);;"
                                                        "文本文件 (*.txt);;"
                                                        "C++文件 (*.cpp *.h *.hpp);;"
                                                        "Markdown文件 (*.md);;"
                                                        "Python文件 (*.py);;"
                                                        "JavaScript文件 (*.js)");
        if (!filePath.isEmpty()) {
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

// 查找或创建父目录项
QTreeWidgetItem* MainWindow::findOrCreateParentItem(const QString &dirPath)
{
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

            // 为目录项设置文件夹图标
            newItem->setIcon(0, QIcon(":/icons/folder"));

            currentParent = newItem;
        }
    }

    return currentParent;
}

// 定位当前文件
void MainWindow::locateCurrentFile()
{
    QString filePath = getCurrentFilePath();
    if (!filePath.isEmpty()) {
        selectFileInTree(filePath);
    }
}

// 展开所有文件
void MainWindow::expandAllFiles()
{
    fileTree->expandAll();
}

// 折叠所有文件
void MainWindow::collapseAllFiles()
{
    fileTree->collapseAll();
}

// 在树中选择文件
void MainWindow::selectFileInTree(const QString &filePath)
{
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
        // 如果找不到，可能是因为文件还未添加到树中
        qDebug() << "File not found in tree:" << filePath;
    }
}

// 递归查找文件项
QTreeWidgetItem* MainWindow::findFileItemInTree(QTreeWidgetItem *parent, const QString &filePath, const QString &displayName)
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

// 递归清理空目录
void MainWindow::cleanupEmptyDirectories(QTreeWidgetItem *item)
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

// 初始化图标映射
void MainWindow::initIconMap()
{
    // 清空现有映射
    iconMap.clear();

    // 文本文件
    iconMap["txt"] = ":/icons/txt";
    iconMap["text"] = ":/icons/text";
    iconMap["md"] = ":/icons/markdown";
    iconMap["markdown"] = ":/icons/markdown";

    // C/C++ 文件
    iconMap["c"] = ":/icons/cpp";
    iconMap["cpp"] = ":/icons/cpp";
    iconMap["cc"] = ":/icons/cpp";
    iconMap["cxx"] = ":/icons/cpp";
    iconMap["h"] = ":/icons/h";
    iconMap["hh"] = ":/icons/h";
    iconMap["hpp"] = ":/icons/h";
    iconMap["hxx"] = ":/icons/h";

    // 脚本语言
    iconMap["py"] = ":/icons/python";
    iconMap["pyw"] = ":/icons/python";
    iconMap["js"] = ":/icons/js";
    iconMap["jsx"] = ":/icons/js";
    iconMap["ts"] = ":/icons/js";
    iconMap["tsx"] = ":/icons/js";
    iconMap["java"] = ":/icons/java";
    iconMap["class"] = ":/icons/java";

    // Web 文件
    iconMap["html"] = ":/icons/html";
    iconMap["htm"] = ":/icons/html";
    iconMap["xhtml"] = ":/icons/html";
    iconMap["css"] = ":/icons/css";
    iconMap["scss"] = ":/icons/css";
    iconMap["sass"] = ":/icons/css";
    iconMap["less"] = ":/icons/css";

    // 数据文件
    iconMap["xml"] = ":/icons/xml";
    iconMap["json"] = ":/icons/json";
    iconMap["yml"] = ":/icons/yaml";
    iconMap["yaml"] = ":/icons/yaml";
    iconMap["toml"] = ":/icons/toml";
    iconMap["ini"] = ":/icons/ini";
    iconMap["conf"] = ":/icons/ini";
    iconMap["cfg"] = ":/icons/ini";

    // 脚本文件
    iconMap["sh"] = ":/icons/sh";
    iconMap["bash"] = ":/icons/sh";
    iconMap["zsh"] = ":/icons/sh";
    iconMap["bat"] = ":/icons/bat";
    iconMap["cmd"] = ":/icons/bat";
    iconMap["ps1"] = ":/icons/ps1";

    // 其他编程语言（根据需要添加）
    iconMap["go"] = ":/icons/go";
    iconMap["rs"] = ":/icons/rust";
    iconMap["php"] = ":/icons/php";
    iconMap["rb"] = ":/icons/ruby";
    iconMap["swift"] = ":/icons/swift";
    iconMap["kt"] = ":/icons/kotlin";
}

// 从树中移除文件
void MainWindow::removeFileFromTree(const QString &filePath)
{
    QFileInfo fileInfo(filePath);
    QString displayName = fileInfo.fileName();  // 注意：这里应该使用完整文件名，不是去掉后缀的

    // 使用完整路径查找并移除文件项
    QTreeWidgetItem *fileItem = findFileItemInTree(fileTree->invisibleRootItem(), filePath, displayName);
    if (fileItem) {
        QTreeWidgetItem *parent = fileItem->parent();
        if (parent) {
            parent->removeChild(fileItem);
            delete fileItem;

            // 从映射中移除
            filePathMap.remove(filePath);

            // 递归清理空目录
            cleanupEmptyDirectories(parent);
        }
    }
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


    return editor;
}

// 工具函数：移除文件后缀
QString MainWindow::removeFileExtension(const QString &fileName)
{
    int dotIndex = fileName.lastIndexOf('.');
    if (dotIndex != -1) {
        return fileName.left(dotIndex);
    }
    return fileName;
}

// 添加文件到左侧树形列表
void MainWindow::addFileToList(const QString &filePath)
{
    QFileInfo fileInfo(filePath);
    QString displayName = fileInfo.fileName();  // 保持显示名为文件名
    QString dirPath = fileInfo.absolutePath();

    // 获取父目录项
    QTreeWidgetItem *parentItem = findOrCreateParentItem(dirPath);

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

    // 根据文件类型设置图标（保持原有代码）
    if (fileInfo.isDir()) {
        fileItem->setIcon(0, QIcon(":/icons/folder"));
    } else {
        QString suffix = fileInfo.suffix().toLower();
        QString iconPath;
        if (iconMap.contains(suffix)) {
            iconPath = iconMap[suffix];
        } else {
            iconPath = ":/icons/file";
        }
        fileItem->setIcon(0, QIcon(iconPath));
    }

    // 保存映射关系 - 使用完整路径作为键，避免文件名冲突
    filePathMap[filePath] = displayName;  // 修改映射关系

    // 展开父目录
    parentItem->setExpanded(true);
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
        removeFileFromTree(filePath);
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

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "错误", "无法打开文件: " + filePath);
        return;
    }

    QTextStream in(&file);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    in.setCodec("UTF-8");
#else
    in.setEncoding(QStringConverter::Utf8);
#endif
    QString content = in.readAll();
    file.close();

    QPlainTextEdit *editor = createEditor();
    editor->setPlainText(content);
    editor->document()->setModified(false);
    editor->setProperty("filePath", filePath);

    // 如果是LineNumberEditor，设置文件名以启用语法高亮
    LineNumberEditor *lineNumberEditor = qobject_cast<LineNumberEditor*>(editor);
    if (lineNumberEditor) {
        lineNumberEditor->setFileName(filePath);
    }

    // 根据文件类型设置是否显示缩进参考线
    if (filePath.endsWith(".cpp") || filePath.endsWith(".h") ||
        filePath.endsWith(".py") || filePath.endsWith(".java") ||
        filePath.endsWith(".js") || filePath.endsWith(".cs")) {
        lineNumberEditor->setShowIndentationGuides(true);
    } else {
        lineNumberEditor->setShowIndentationGuides(false);
    }

    QFileInfo fileInfo(filePath);
    int index = tabWidget->addTab(editor, fileInfo.fileName());
    tabWidget->setCurrentIndex(index);

    // 添加到左侧文件树
    addFileToList(filePath);

    // 在树中选中该文件
    selectFileInTree(filePath);

    // 更新最近文件列表
    m_recentFiles.removeAll(filePath);  // 移除旧的
    m_recentFiles.prepend(filePath);     // 添加到开头

    // 限制数量
    while (m_recentFiles.size() > MAX_RECENT_FILES) {
        m_recentFiles.removeLast();
    }

    // 立即保存
    saveFileList();

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

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "错误", "无法保存文件: " + filePath);
        return false;
    }

    QTextStream out(&file);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    out.setCodec("UTF-8");
#else
    out.setEncoding(QStringConverter::Utf8);
#endif
    out << editor->toPlainText();
    file.close();

    editor->document()->setModified(false);
    saveAct->setEnabled(false);

    updateWindowTitle();
    statusBar()->showMessage("文件保存成功", 2000);
    return true;
}

bool MainWindow::saveFileAs()
{
    QPlainTextEdit *editor = getCurrentEditor();
    if (!editor) return false;

    QString filePath = QFileDialog::getSaveFileName(this, "另存为",
                                                    QDir::homePath(),
                                                    "所有文件 (*.*);;"
                                                    "文本文件 (*.txt);;"
                                                    "C++文件 (*.cpp *.h *.hpp);;"
                                                    "Markdown文件 (*.md)");
    if (filePath.isEmpty()) {
        return false;
    }

    editor->setProperty("filePath", filePath);
    QFileInfo fileInfo(filePath);
    tabWidget->setTabText(tabWidget->currentIndex(), fileInfo.fileName());

    // 设置文件名以启用语法高亮
    LineNumberEditor *lineNumberEditor = qobject_cast<LineNumberEditor*>(editor);
    if (lineNumberEditor) {
        lineNumberEditor->setFileName(filePath);
    }

    // 添加到左侧文件树
    addFileToList(filePath);

    return saveFile();
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

    // 如果文件已关闭，从树中移除对应的项
    // if (!filePath.isEmpty()) {
    //     removeFileFromTree(filePath);
    //     m_recentFiles.removeAll(filePath);
    // }

    // 更新按钮状态
    updateButtonStates();

    updateWindowTitle();
    updateStatusBar();
    updateEditActions();

    // 保存更新后的文件列表
    saveFileList();
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
            selectFileInTree(absolutePath);
            return;
        }
    }

    // 打开文件
    QFile file(absolutePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "错误", "无法打开文件: " + absolutePath);
        return;
    }

    QTextStream in(&file);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    in.setCodec("UTF-8");
#else
    in.setEncoding(QStringConverter::Utf8);
#endif
    QString content = in.readAll();
    file.close();

    QPlainTextEdit *editor = createEditor();
    editor->setPlainText(content);
    editor->document()->setModified(false);
    editor->setProperty("filePath", absolutePath);

    // 如果是LineNumberEditor，设置文件名以启用语法高亮
    LineNumberEditor *lineNumberEditor = qobject_cast<LineNumberEditor*>(editor);
    if (lineNumberEditor) {
        lineNumberEditor->setFileName(absolutePath);
    }

    // 根据文件类型设置是否显示缩进参考线
    if (absolutePath.endsWith(".cpp") || absolutePath.endsWith(".h") ||
        absolutePath.endsWith(".py") || absolutePath.endsWith(".java") ||
        absolutePath.endsWith(".js") || absolutePath.endsWith(".cs")) {
        lineNumberEditor->setShowIndentationGuides(true);
    } else {
        lineNumberEditor->setShowIndentationGuides(false);
    }

    int index = tabWidget->addTab(editor, fileInfo.fileName());
    tabWidget->setCurrentIndex(index);

    // 添加到左侧文件树
    addFileToList(absolutePath);

    // 在树中选中该文件
    selectFileInTree(absolutePath);

    saveAct->setEnabled(false);
    saveAsAct->setEnabled(true);
    closeAct->setEnabled(true);

    updateWindowTitle();
    updateStatusBar();

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
        setWindowTitle("ink");
        return;
    }

    QPlainTextEdit *editor = getCurrentEditor();
    if (!editor) {
        setWindowTitle("ink");
        return;
    }

    QString filePath = getCurrentFilePath();
    QString title = "ink";

    if (filePath.isEmpty()) {
        title += " - 无标题";
    } else {
        title += " - " + removeFileExtension(QFileInfo(filePath).fileName());
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
        statusFileInfo->setText(removeFileExtension(info.fileName()) + " | " +
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
        selectFileInTree(filePath);  // 使用完整路径定位
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
    ret = QMessageBox::warning(this, "ink",
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

// 添加文件列表保存函数
void MainWindow::saveFileList()
{
    QSettings settings("ink", "Settings");

    // 获取当前所有打开的文件
    QStringList currentFiles;
    for (int i = 0; i < tabWidget->count(); ++i) {
        QPlainTextEdit *editor = qobject_cast<QPlainTextEdit*>(tabWidget->widget(i));
        if (editor) {
            QString filePath = editor->property("filePath").toString();
            if (!filePath.isEmpty() && QFile::exists(filePath)) {
                currentFiles.append(filePath);
            }
        }
    }

    // 合并现有文件列表，去重并保持最近的文件在前面
    QStringList allFiles = currentFiles;
    for (const QString &file : m_recentFiles) {
        if (!allFiles.contains(file) && QFile::exists(file)) {
            allFiles.append(file);
        }
    }

    // 限制数量
    while (allFiles.size() > MAX_RECENT_FILES) {
        allFiles.removeLast();
    }

    // 保存
    settings.setValue("recentFiles", allFiles);
    qDebug() << "Saved files:" << allFiles.size();
}

// 添加文件列表加载函数
void MainWindow::loadFileList()
{
    QSettings settings("ink", "Settings");

    // 读取最近打开的文件列表
    m_recentFiles = settings.value("recentFiles").toStringList();

    qDebug() << "Loading files:" << m_recentFiles.size();

    // 清除现有的文件树
    fileTree->clear();
    filePathMap.clear();

    // 重新添加文件到树中
    for (const QString &filePath : m_recentFiles) {
        if (QFile::exists(filePath)) {
            addFileToList(filePath);
            qDebug() << "Added to tree:" << filePath;
        }
    }

    // 展开所有目录
    fileTree->expandAll();
}
