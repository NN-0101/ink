#include "findreplacedialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>
#include <QPushButton>
#include <QPlainTextEdit>
#include <QTextDocument>
#include <QTextCursor>
#include <QKeyEvent>
#include <QRegularExpression>
#include <QDebug>
#include <QScrollBar>
#include <QApplication>  // 必须添加这个头文件

FindReplaceDialog::FindReplaceDialog(QWidget *parent)
    : QDialog(parent)
    , m_textEdit(nullptr)
    , m_isReplaceMode(false)
    , m_currentMatchIndex(-1)
{
    setWindowTitle("查找");
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setFixedWidth(500);

    // 创建UI组件
    m_findEdit = new QLineEdit(this);
    m_findEdit->setPlaceholderText("查找...");

    m_replaceEdit = new QLineEdit(this);
    m_replaceEdit->setPlaceholderText("替换为...");

    m_caseSensitiveCheck = new QCheckBox("区分大小写(&C)", this);
    m_wholeWordCheck = new QCheckBox("全词匹配(&W)", this);
    m_regexCheck = new QCheckBox("正则表达式(&R)", this);
    m_highlightAllCheck = new QCheckBox("高亮所有匹配(&H)", this);
    m_highlightAllCheck->setChecked(true);

    m_findNextBtn = new QPushButton("查找下一个(&F)", this);
    m_findNextBtn->setDefault(true);

    m_findPrevBtn = new QPushButton("查找上一个(&P)", this);

    m_replaceBtn = new QPushButton("替换(&R)", this);
    m_replaceBtn->setEnabled(false);

    m_replaceAllBtn = new QPushButton("全部替换(&A)", this);
    m_replaceAllBtn->setEnabled(false);

    m_closeBtn = new QPushButton("关闭(&C)", this);

    m_statusLabel = new QLabel(this);
    m_statusLabel->setStyleSheet("color: gray; padding: 2px;");

    // 布局
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // 查找行
    QHBoxLayout *findLayout = new QHBoxLayout;
    findLayout->addWidget(new QLabel("查找(&N):", this));
    findLayout->addWidget(m_findEdit);
    mainLayout->addLayout(findLayout);

    // 替换行（初始隐藏）
    QHBoxLayout *replaceLayout = new QHBoxLayout;
    replaceLayout->addWidget(new QLabel("替换为(&E):", this));
    replaceLayout->addWidget(m_replaceEdit);
    mainLayout->addLayout(replaceLayout);
    m_replaceEdit->setVisible(false);

    // 选项
    QGridLayout *optionsLayout = new QGridLayout;
    optionsLayout->addWidget(m_caseSensitiveCheck, 0, 0);
    optionsLayout->addWidget(m_wholeWordCheck, 0, 1);
    optionsLayout->addWidget(m_regexCheck, 1, 0);
    optionsLayout->addWidget(m_highlightAllCheck, 1, 1);
    mainLayout->addLayout(optionsLayout);

    // 按钮行
    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addWidget(m_findNextBtn);
    buttonLayout->addWidget(m_findPrevBtn);
    buttonLayout->addWidget(m_replaceBtn);
    buttonLayout->addWidget(m_replaceAllBtn);
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_closeBtn);
    mainLayout->addLayout(buttonLayout);

    // 状态栏
    mainLayout->addWidget(m_statusLabel);

    // 连接信号槽 - 修复参数类型不匹配的问题
    connect(m_findEdit, &QLineEdit::textChanged, this, &FindReplaceDialog::onFindTextChanged);
    connect(m_findNextBtn, &QPushButton::clicked, this, &FindReplaceDialog::onFindNext);
    connect(m_findPrevBtn, &QPushButton::clicked, this, &FindReplaceDialog::onFindPrev);
    connect(m_replaceBtn, &QPushButton::clicked, this, &FindReplaceDialog::onReplace);
    connect(m_replaceAllBtn, &QPushButton::clicked, this, &FindReplaceDialog::onReplaceAll);
    connect(m_closeBtn, &QPushButton::clicked, this, &QDialog::close);
    connect(m_highlightAllCheck, &QCheckBox::toggled, this, &FindReplaceDialog::onHighlightAll);

    // 使用 lambda 表达式修复参数类型不匹配的问题
    connect(m_caseSensitiveCheck, &QCheckBox::toggled, this, [this](bool) {
        onFindTextChanged(m_findEdit->text());
    });
    connect(m_wholeWordCheck, &QCheckBox::toggled, this, [this](bool) {
        onFindTextChanged(m_findEdit->text());
    });
    connect(m_regexCheck, &QCheckBox::toggled, this, [this](bool) {
        onFindTextChanged(m_findEdit->text());
    });

    // 安装事件过滤器
    m_findEdit->installEventFilter(this);
    m_replaceEdit->installEventFilter(this);

    updateButtonStates();
}

FindReplaceDialog::~FindReplaceDialog()
{
    clearAllHighlights();
}

void FindReplaceDialog::setTextEdit(QPlainTextEdit *textEdit)
{
    if (m_textEdit == textEdit)
        return;

    // 清除之前的高亮
    clearAllHighlights();

    m_textEdit = textEdit;

    // 如果有选中的文本，自动填入查找框
    if (m_textEdit) {
        QTextCursor cursor = m_textEdit->textCursor();
        if (cursor.hasSelection()) {
            m_findEdit->setText(cursor.selectedText());
            m_findEdit->selectAll();
        }
    }
}

void FindReplaceDialog::showFind()
{
    m_isReplaceMode = false;
    setWindowTitle("查找");
    m_replaceEdit->setVisible(false);
    m_replaceBtn->setVisible(false);
    m_replaceAllBtn->setVisible(false);

    // 调整对话框大小
    adjustSize();

    show();
    activateWindow();
    m_findEdit->setFocus();
    m_findEdit->selectAll();
}

void FindReplaceDialog::showReplace()
{
    m_isReplaceMode = true;
    setWindowTitle("替换");
    m_replaceEdit->setVisible(true);
    m_replaceBtn->setVisible(true);
    m_replaceAllBtn->setVisible(true);

    // 调整对话框大小
    adjustSize();

    show();
    activateWindow();
    m_findEdit->setFocus();
    m_findEdit->selectAll();
}

void FindReplaceDialog::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        close();
    } else if (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return) {
        if (event->modifiers() == Qt::ShiftModifier) {
            onFindPrev();
        } else {
            onFindNext();
        }
    } else {
        QDialog::keyPressEvent(event);
    }
}

void FindReplaceDialog::closeEvent(QCloseEvent *event)
{
    clearAllHighlights();
    QDialog::closeEvent(event);
}

bool FindReplaceDialog::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        if (keyEvent->key() == Qt::Key_Enter || keyEvent->key() == Qt::Key_Return) {
            if (obj == m_findEdit || obj == m_replaceEdit) {
                if (keyEvent->modifiers() == Qt::ShiftModifier) {
                    onFindPrev();
                } else {
                    onFindNext();
                }
                return true;
            }
        }
    }
    return QDialog::eventFilter(obj, event);
}

void FindReplaceDialog::onFindNext()
{
    findNext(true);
}

void FindReplaceDialog::onFindPrev()
{
    findNext(false);
}

void FindReplaceDialog::findNext(bool forward)
{
    if (!m_textEdit || m_findEdit->text().isEmpty()) {
        m_statusLabel->setText("请输入查找内容");
        return;
    }

    QString findText = m_findEdit->text();
    QTextDocument::FindFlags flags;

    if (m_caseSensitiveCheck->isChecked())
        flags |= QTextDocument::FindCaseSensitively;
    if (m_wholeWordCheck->isChecked())
        flags |= QTextDocument::FindWholeWords;
    if (!forward)
        flags |= QTextDocument::FindBackward;

    QTextCursor cursor = m_textEdit->textCursor();

    // 如果当前有选中文本，从选中位置之后开始查找
    if (cursor.hasSelection()) {
        if (forward) {
            cursor.setPosition(cursor.selectionEnd());
        } else {
            cursor.setPosition(cursor.selectionStart());
        }
    }

    QTextCursor found;

    if (m_regexCheck->isChecked()) {
        // 正则表达式查找
        QRegularExpression regex(findText,
                                 m_caseSensitiveCheck->isChecked() ? QRegularExpression::NoPatternOption
                                                                   : QRegularExpression::CaseInsensitiveOption);

        if (m_wholeWordCheck->isChecked()) {
            QString pattern = QString("\\b%1\\b").arg(findText);
            regex.setPattern(pattern);
        }

        found = m_textEdit->document()->find(regex, cursor, flags);
    } else {
        found = m_textEdit->document()->find(findText, cursor, flags);
    }

    if (!found.isNull()) {
        m_textEdit->setTextCursor(found);
        m_textEdit->ensureCursorVisible();

        // 更新状态
        findAllMatches(); // 重新统计所有匹配
        m_statusLabel->setText(QString("找到 %1 个匹配，当前第 %2 个")
                                   .arg(m_currentMatches.size())
                                   .arg(m_currentMatchIndex + 1));
    } else {
        // 未找到，提示并从头开始查找
        m_statusLabel->setText("未找到匹配项");

        // 播放提示音 - 需要包含 QApplication
        QApplication::beep();
    }

    // 更新高亮
    if (m_highlightAllCheck->isChecked()) {
        highlightAllMatches();
    }
}

void FindReplaceDialog::onReplace()
{
    replaceCurrent();
}

void FindReplaceDialog::onReplaceAll()
{
    replaceAll();
}

void FindReplaceDialog::onFindTextChanged(const QString &text)
{
    Q_UNUSED(text);  // 使用 Q_UNUSED 避免未使用参数警告

    updateButtonStates();

    if (m_highlightAllCheck->isChecked()) {
        highlightAllMatches();
    }

    // 更新匹配计数
    findAllMatches();
    if (m_currentMatches.isEmpty()) {
        m_statusLabel->setText("未找到匹配项");
    } else {
        m_statusLabel->setText(QString("找到 %1 个匹配").arg(m_currentMatches.size()));
    }
}

void FindReplaceDialog::onHighlightAll()
{
    if (m_highlightAllCheck->isChecked()) {
        highlightAllMatches();
    } else {
        clearAllHighlights();
    }
}

void FindReplaceDialog::updateButtonStates()
{
    bool hasText = !m_findEdit->text().isEmpty();
    m_findNextBtn->setEnabled(hasText && m_textEdit);
    m_findPrevBtn->setEnabled(hasText && m_textEdit);
    m_replaceBtn->setEnabled(hasText && m_textEdit && m_isReplaceMode);
    m_replaceAllBtn->setEnabled(hasText && m_textEdit && m_isReplaceMode);
}

QList<QTextCursor> FindReplaceDialog::findMatches(const QString &text, bool forward, int startPos)
{
    QList<QTextCursor> matches;
    if (!m_textEdit || text.isEmpty())
        return matches;

    QTextDocument *doc = m_textEdit->document();
    QTextDocument::FindFlags flags;

    if (m_caseSensitiveCheck->isChecked())
        flags |= QTextDocument::FindCaseSensitively;
    if (m_wholeWordCheck->isChecked())
        flags |= QTextDocument::FindWholeWords;

    // 注意：forward 参数在这里没有使用，因为我们需要查找所有匹配
    Q_UNUSED(forward);

    QTextCursor cursor(doc);
    if (startPos >= 0) {
        cursor.setPosition(startPos);
    }

    if (m_regexCheck->isChecked()) {
        QRegularExpression regex(text,
                                 m_caseSensitiveCheck->isChecked() ? QRegularExpression::NoPatternOption
                                                                   : QRegularExpression::CaseInsensitiveOption);

        if (m_wholeWordCheck->isChecked()) {
            QString pattern = QString("\\b%1\\b").arg(text);
            regex.setPattern(pattern);
        }

        QTextCursor found = doc->find(regex, cursor, flags);
        while (!found.isNull()) {
            matches.append(found);
            found = doc->find(regex, found, flags);
        }
    } else {
        QTextCursor found = doc->find(text, cursor, flags);
        while (!found.isNull()) {
            matches.append(found);
            found = doc->find(text, found, flags);
        }
    }

    return matches;
}

void FindReplaceDialog::findAllMatches()
{
    m_currentMatches.clear();
    m_currentMatchIndex = -1;

    if (!m_textEdit || m_findEdit->text().isEmpty())
        return;

    m_currentMatches = findMatches(m_findEdit->text());

    // 找出当前选中的匹配是第几个
    if (!m_currentMatches.isEmpty()) {
        QTextCursor currentCursor = m_textEdit->textCursor();
        int currentPos = currentCursor.selectionStart();

        for (int i = 0; i < m_currentMatches.size(); ++i) {
            if (m_currentMatches[i].selectionStart() == currentPos) {
                m_currentMatchIndex = i;
                break;
            }
        }
    }
}

void FindReplaceDialog::highlightAllMatches()
{
    if (!m_textEdit)
        return;

    clearAllHighlights();

    if (m_findEdit->text().isEmpty())
        return;

    findAllMatches();

    // 创建高亮格式
    QTextEdit::ExtraSelection highlight;
    highlight.format.setBackground(QColor(255, 255, 0, 100)); // 半透明黄色
    highlight.format.setForeground(Qt::black);

    // 当前选中的匹配使用不同的颜色
    QTextEdit::ExtraSelection currentHighlight;
    currentHighlight.format.setBackground(QColor(255, 165, 0, 150)); // 半透明橙色
    currentHighlight.format.setForeground(Qt::black);
    currentHighlight.format.setFontWeight(QFont::Bold);

    QList<QTextEdit::ExtraSelection> selections;

    for (int i = 0; i < m_currentMatches.size(); ++i) {
        highlight.cursor = m_currentMatches[i];

        if (i == m_currentMatchIndex) {
            currentHighlight.cursor = m_currentMatches[i];
            selections.append(currentHighlight);
        } else {
            selections.append(highlight);
        }
    }

    // 保留原有的行高亮
    QList<QTextEdit::ExtraSelection> currentSelections = m_textEdit->extraSelections();
    selections.append(currentSelections);

    m_textEdit->setExtraSelections(selections);
    m_highlightSelections = selections;
}

void FindReplaceDialog::clearAllHighlights()
{
    if (m_textEdit) {
        // 只清除查找高亮，保留行高亮
        QList<QTextEdit::ExtraSelection> currentSelections = m_textEdit->extraSelections();
        // 这里需要更智能的过滤，简单起见先全部清除
        m_textEdit->setExtraSelections(QList<QTextEdit::ExtraSelection>());
    }
    m_highlightSelections.clear();
}

void FindReplaceDialog::replaceCurrent()
{
    if (!m_textEdit || m_findEdit->text().isEmpty())
        return;

    QTextCursor cursor = m_textEdit->textCursor();
    QString replaceText = m_replaceEdit->text();

    if (cursor.hasSelection()) {
        cursor.insertText(replaceText);

        // 查找下一个
        onFindNext();
    } else {
        onFindNext();
    }
}

void FindReplaceDialog::replaceAll()
{
    if (!m_textEdit || m_findEdit->text().isEmpty())
        return;

    QString findText = m_findEdit->text();
    QString replaceText = m_replaceEdit->text();

    findAllMatches();
    int replaceCount = m_currentMatches.size();

    if (replaceCount == 0) {
        m_statusLabel->setText("未找到匹配项");
        return;
    }

    // 从后往前替换，避免位置变化
    QTextCursor cursor = m_textEdit->textCursor();
    int originalPos = cursor.position();

    // 使用 QTextCursor 的合并编辑块
    QTextCursor editCursor(m_textEdit->document());
    editCursor.beginEditBlock();  // 开始编辑块

    for (int i = m_currentMatches.size() - 1; i >= 0; --i) {
        QTextCursor replaceCursor = m_currentMatches[i];
        replaceCursor.insertText(replaceText);
    }

    editCursor.endEditBlock();  // 结束编辑块

    // 恢复光标位置
    if (originalPos < m_textEdit->document()->characterCount()) {
        cursor.setPosition(originalPos);
        m_textEdit->setTextCursor(cursor);
    }

    // 重新查找高亮
    if (m_highlightAllCheck->isChecked()) {
        highlightAllMatches();
    }

    m_statusLabel->setText(QString("已替换 %1 个匹配").arg(replaceCount));
}

QTextCursor FindReplaceDialog::getCurrentSelection()
{
    return m_textEdit ? m_textEdit->textCursor() : QTextCursor();
}

void FindReplaceDialog::findNextMatch()
{
    onFindNext();
}

void FindReplaceDialog::findPrevMatch()
{
    onFindPrev();
}
