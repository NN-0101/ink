#include "linenumbereditor.h"
#include <QFontMetrics>
#include <QDebug>
#include <QTextStream>
#include <QScrollBar>
#include <QResizeEvent>
#include <QPainter>
#include <QTextBlock>
#include <QApplication>
#include <QFontDatabase>
#include <QFileInfo>
#include <QRegularExpression>

// 默认值定义
namespace {
const QColor DEFAULT_LINE_NUMBER_COLOR = QColor(120, 120, 120);
const QColor DEFAULT_LINE_NUMBER_BG_COLOR = QColor(250, 250, 250);
const QColor DEFAULT_CURRENT_LINE_COLOR = QColor(240, 245, 255);
const QColor DEFAULT_INDENT_GUIDE_COLOR = QColor(220, 220, 220);
const int DEFAULT_TAB_STOP_SPACES = 4;
const int DEFAULT_FONT_SIZE = 12;
const int LINE_NUMBER_MARGIN = 6;
}

LineNumberEditor::LineNumberEditor(QWidget *parent)
    : QPlainTextEdit(parent)
    , m_lineNumberArea(new LineNumberArea(this))
    , m_highlighter(nullptr)
    , m_lineNumberDigits(1)
    , m_lineNumberColor(DEFAULT_LINE_NUMBER_COLOR)
    , m_lineNumberBgColor(DEFAULT_LINE_NUMBER_BG_COLOR)
    , m_currentLineColor(DEFAULT_CURRENT_LINE_COLOR)
    , m_showIndentationGuides(true)
    , m_indentationGuideColor(DEFAULT_INDENT_GUIDE_COLOR)
    , m_tabStopSpaces(DEFAULT_TAB_STOP_SPACES)
    , m_spaceWidth(0.0)
    , m_syntaxHighlightEnabled(true)
    , m_cachedLineNumberWidth(-1)
    , m_cachedBlockCount(0)
{
    // 设置等宽字体
    QFont font = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    font.setPointSize(DEFAULT_FONT_SIZE);
    setFont(font);

    // 计算字体度量
    calculateFontMetrics();

    // 设置Tab宽度
    updateTabStopDistance();

    // 连接信号槽
    connect(this, &LineNumberEditor::blockCountChanged,
            this, &LineNumberEditor::updateLineNumberAreaWidth);
    connect(this, &LineNumberEditor::updateRequest,
            this, &LineNumberEditor::updateLineNumberArea);
    connect(this, &LineNumberEditor::cursorPositionChanged,
            this, &LineNumberEditor::highlightCurrentLine);
    connect(this->document(), &QTextDocument::contentsChange,
            this, [this](int, int, int) {
                m_cachedLineNumberWidth = -1; // 使宽度缓存失效
            });

    // 初始设置
    setTabChangesFocus(false);
    setWordWrapMode(QTextOption::NoWrap);
    updateLineNumberAreaWidth(0);
    highlightCurrentLine();
}

LineNumberEditor::~LineNumberEditor()
{
    // 语法高亮器会自动删除，因为它是QPlainTextEdit的子对象
}

void LineNumberEditor::setupHighlighter()
{
    if (!m_syntaxHighlightEnabled || m_fileName.isEmpty()) {
        if (m_highlighter) {
            delete m_highlighter;
            m_highlighter = nullptr;
        }
        return;
    }

    // 如果高亮器不存在或文件类型改变了，重新创建高亮器
    QFileInfo fileInfo(m_fileName);
    QString suffix = fileInfo.suffix().toLower();
    QString fileType = detectFileType(suffix);

    // 检查是否需要创建新的高亮器
    bool needNewHighlighter = true;
    if (m_highlighter) {
        QString currentFileType = document()->property("fileType").toString();
        if (currentFileType == fileType) {
            needNewHighlighter = false;
        }
    }

    if (needNewHighlighter) {
        // 删除旧的高亮器
        if (m_highlighter) {
            delete m_highlighter;
            m_highlighter = nullptr;
        }

        // 创建新的高亮器
        m_highlighter = new SyntaxHighlighter(document());

        // 设置文件类型属性
        document()->setProperty("fileName", m_fileName);
        document()->setProperty("fileType", fileType);

        // 重新高亮
        m_highlighter->rehighlight();

        // 触发视图更新
        viewport()->update();
    }
}

QString LineNumberEditor::detectFileType(const QString &suffix)
{
    // C/C++文件
    if (suffix == "c" || suffix == "cpp" || suffix == "cc" || suffix == "cxx" ||
        suffix == "h" || suffix == "hh" || suffix == "hpp" || suffix == "hxx") {
        return "cpp";
    }

    // Python文件
    if (suffix == "py" || suffix == "pyw") {
        return "python";
    }

    // Java文件
    if (suffix == "java") {
        return "java";
    }

    // JavaScript/TypeScript文件
    if (suffix == "js" || suffix == "jsx" || suffix == "ts" || suffix == "tsx") {
        return "javascript";
    }

    // HTML文件
    if (suffix == "html" || suffix == "htm" || suffix == "xhtml") {
        return "html";
    }

    // CSS文件
    if (suffix == "css" || suffix == "scss" || suffix == "sass" || suffix == "less") {
        return "css";
    }

    // Markdown文件
    if (suffix == "md" || suffix == "markdown") {
        return "markdown";
    }

    // 其他编程语言
    if (suffix == "cs") return "csharp";
    if (suffix == "go") return "go";
    if (suffix == "rs") return "rust";
    if (suffix == "php") return "php";
    if (suffix == "rb") return "ruby";
    if (suffix == "swift") return "swift";
    if (suffix == "kt" || suffix == "kts") return "kotlin";
    if (suffix == "lua") return "lua";
    if (suffix == "r") return "r";
    if (suffix == "sh" || suffix == "bash") return "shell";

    // 配置文件
    if (suffix == "xml") return "xml";
    if (suffix == "json") return "json";
    if (suffix == "yml" || suffix == "yaml") return "yaml";
    if (suffix == "toml") return "toml";
    if (suffix == "ini" || suffix == "conf" || suffix == "cfg") return "ini";

    // 纯文本文件
    if (suffix == "txt" || suffix == "text") {
        return "text";
    }

    // 默认返回text
    return "text";
}

void LineNumberEditor::setFileName(const QString &fileName)
{
    if (m_fileName != fileName) {
        m_fileName = fileName;

        // 更新语法高亮
        if (m_syntaxHighlightEnabled) {
            setupHighlighter();
        }

        // 根据文件类型决定是否显示缩进参考线
        QFileInfo fileInfo(fileName);
        QString suffix = fileInfo.suffix().toLower();

        // 对编程语言文件显示缩进参考线
        static const QStringList programmingLanguages = {
            "c", "cpp", "cc", "cxx", "h", "hpp", "hxx",
            "py", "pyw",
            "java",
            "js", "jsx", "ts", "tsx",
            "cs",
            "go",
            "rs",
            "php",
            "rb",
            "swift",
            "kt", "kts",
            "lua",
            "r",
            "sh", "bash",
            "html", "htm", "xhtml",
            "css", "scss", "sass", "less",
            "xml",
            "json"
        };

        if (programmingLanguages.contains(suffix)) {
            setShowIndentationGuides(true);
        } else {
            setShowIndentationGuides(false);
        }

        // 触发重绘
        viewport()->update();
    }
}

void LineNumberEditor::setSyntaxHighlightEnabled(bool enabled)
{
    if (m_syntaxHighlightEnabled != enabled) {
        m_syntaxHighlightEnabled = enabled;

        if (enabled && !m_highlighter && !m_fileName.isEmpty()) {
            setupHighlighter();
        } else if (!enabled && m_highlighter) {
            delete m_highlighter;
            m_highlighter = nullptr;
            viewport()->update();
        }
    }
}

void LineNumberEditor::lineNumberAreaPaintEvent(QPaintEvent *event)
{
    if (!event || !m_lineNumberArea) return;

    QPainter painter(m_lineNumberArea);
    painter.setRenderHint(QPainter::Antialiasing);

    // 填充背景
    painter.fillRect(event->rect(), m_lineNumberBgColor);

    // 绘制边框
    painter.setPen(QColor(200, 200, 200));
    painter.drawLine(m_lineNumberArea->width() - 1, event->rect().top(),
                     m_lineNumberArea->width() - 1, event->rect().bottom());

    // 准备绘制行号
    painter.setPen(m_lineNumberColor);
    painter.setFont(font());

    // 获取可见区域的信息
    QTextBlock block = firstVisibleBlock();
    if (!block.isValid()) return;

    int blockNumber = block.blockNumber();
    int top = qRound(blockBoundingGeometry(block).translated(contentOffset()).top());
    int bottom = top + qRound(blockBoundingRect(block).height());

    // 只绘制可见行
    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            drawLineNumber(painter, block, top);
        }

        block = block.next();
        if (!block.isValid()) break;

        top = bottom;
        bottom = top + qRound(blockBoundingRect(block).height());
        blockNumber++;
    }
}

int LineNumberEditor::lineNumberAreaWidth() const
{
    if (m_cachedLineNumberWidth >= 0 &&
        m_cachedBlockCount == blockCount()) {
        return m_cachedLineNumberWidth;
    }

    m_cachedBlockCount = blockCount();
    m_cachedLineNumberWidth = calculateLineNumberAreaWidth();
    return m_cachedLineNumberWidth;
}

qreal LineNumberEditor::calculateLineNumberAreaWidth() const
{
    int digits = 1;
    int max = qMax(1, blockCount());

    // 计算需要的位数
    while (max >= 10) {
        max /= 10;
        digits++;
    }

    QFontMetrics fm(font());
    int charWidth = fm.horizontalAdvance('9');

    // 宽度 = 左边距 + 数字宽度 + 右边距
    return 2 * LINE_NUMBER_MARGIN + digits * charWidth + LINE_NUMBER_MARGIN;
}

void LineNumberEditor::drawLineNumber(QPainter &painter, const QTextBlock &block, int y)
{
    int lineNumber = block.blockNumber() + 1;
    QString number = QString::number(lineNumber);

    QRect numberRect(0, y, m_lineNumberArea->width() - LINE_NUMBER_MARGIN,
                     fontMetrics().height());

    // 对齐方式：右对齐、垂直居中
    painter.drawText(numberRect, Qt::AlignRight | Qt::AlignVCenter, number);
}

void LineNumberEditor::drawIndentationGuides(QPainter &painter, const QRect &rect)
{
    painter.save();
    painter.setRenderHint(QPainter::Antialiasing, false);

    // 设置虚线样式
    QPen pen(m_indentationGuideColor, 1);
    pen.setStyle(Qt::DotLine);
    painter.setPen(pen);

    // 获取文本起点
    QTextCursor cursor = textCursor();
    cursor.movePosition(QTextCursor::StartOfLine);
    QRect startRect = cursorRect(cursor);
    qreal startX = startRect.left();

    // 获取可见文本块
    QTextBlock block = firstVisibleBlock();
    QPointF contentOffset = this->contentOffset();
    qreal top = blockBoundingGeometry(block).translated(contentOffset).top();
    qreal bottom = top + blockBoundingRect(block).height();

    // 用于跟踪代码块深度
    CodeBlockInfo blockInfo;

    // 绘制每个可见块的缩进参考线
    while (block.isValid() && top <= rect.bottom()) {
        if (block.isVisible() && bottom >= rect.top()) {
            QString text = block.text();

            // 分析当前行的代码块状态
            CodeBlockInfo lineInfo = blockInfo; // 复制当前状态
            bool hasCodeBeforeComment = false;
            bool hasCodeBlockInLine = false;
            int openingBracePos = -1;

            // 扫描当前行
            for (int i = 0; i < text.length(); ++i) {
                QChar ch = text.at(i);

                // 处理字符串
                if (!lineInfo.inComment) {
                    if (!lineInfo.inString && (ch == '\'' || ch == '"')) {
                        lineInfo.inString = true;
                        lineInfo.stringChar = ch.toLatin1();
                        continue;
                    } else if (lineInfo.inString && ch == lineInfo.stringChar) {
                        // 检查是否是转义字符
                        if (i > 0 && text.at(i-1) == '\\') {
                            continue;
                        }
                        lineInfo.inString = false;
                        continue;
                    }
                }

                // 处理注释
                if (!lineInfo.inString) {
                    // C风格注释开始
                    if (!lineInfo.inComment && i+1 < text.length() &&
                        ch == '/' && text.at(i+1) == '*') {
                        lineInfo.inComment = true;
                        i++;
                        continue;
                    }
                    // C风格注释结束
                    else if (lineInfo.inComment && i+1 < text.length() &&
                             ch == '*' && text.at(i+1) == '/') {
                        lineInfo.inComment = false;
                        i++;
                        continue;
                    }
                    // C++单行注释
                    else if (!lineInfo.inComment && i+1 < text.length() &&
                             ch == '/' && text.at(i+1) == '/') {
                        // 在注释开始前检查是否有代码块
                        if (blockInfo.depth > 0) {
                            hasCodeBeforeComment = true;
                        }
                        break; // 跳过行剩余部分
                    }
                }

                // 不在注释或字符串中，检查代码块
                if (!lineInfo.inString && !lineInfo.inComment) {
                    if (ch == '{' || ch == '[' || ch == '(') {
                        blockInfo.depth++;
                        lineInfo.depth++;
                        hasCodeBlockInLine = true;

                        // 记录开括号位置（用于后面计算缩进）
                        if (openingBracePos == -1) {
                            openingBracePos = i;
                        }
                    } else if (ch == '}' || ch == ']' || ch == ')') {
                        if (blockInfo.depth > 0) blockInfo.depth--;
                        if (lineInfo.depth > 0) lineInfo.depth--;
                        hasCodeBlockInLine = true;
                    }
                }
            }

            // 决定是否绘制缩进参考线
            bool shouldDrawGuides = false;
            int indentLevel = 0;

            if (blockInfo.depth > 0) {
                // 在当前代码块内部
                shouldDrawGuides = true;
                indentLevel = blockInfo.depth;

                // 如果有开括号在行内，使用行内深度而不是累积深度
                if (hasCodeBlockInLine && openingBracePos != -1) {
                    // 对于开括号所在行，使用括号后的缩进
                    indentLevel = lineInfo.depth;
                }
            }

            // 或者：如果注释前有代码块，也绘制
            if (hasCodeBeforeComment) {
                shouldDrawGuides = true;
                indentLevel = blockInfo.depth;
            }

            // 绘制缩进参考线
            if (shouldDrawGuides && indentLevel > 0) {
                int column = 0;
                bool inIndent = true;
                bool foundNonWhitespace = false;

                for (int i = 0; i < text.length() && inIndent; ++i) {
                    QChar ch = text.at(i);

                    // 跳过字符串和注释中的字符
                    if (!foundNonWhitespace) {
                        if (ch == ' ') {
                            column++;

                            // 在Tab停止位置绘制参考线
                            if (column % m_tabStopSpaces == 0) {
                                qreal x = startX + column * m_spaceWidth;
                                painter.drawLine(QLineF(x, top, x, bottom));
                            }
                        }
                        else if (ch == '\t') {
                            // Tab键：绘制所有经过的Tab停止线
                            int nextTabStop = ((column / m_tabStopSpaces) + 1) * m_tabStopSpaces;

                            for (int tabCol = column + 1; tabCol <= nextTabStop; tabCol++) {
                                if (tabCol % m_tabStopSpaces == 0) {
                                    qreal x = startX + tabCol * m_spaceWidth;
                                    painter.drawLine(QLineF(x, top, x, bottom));
                                }
                            }

                            column = nextTabStop;
                        }
                        else if (!ch.isSpace()) {
                            foundNonWhitespace = true;
                            inIndent = false;
                        }
                    } else {
                        inIndent = false;
                    }
                }
            }

            // 更新代码块状态（用于下一行）
            blockInfo = lineInfo;
        }

        block = block.next();
        if (!block.isValid()) break;

        top = bottom;
        bottom = top + blockBoundingRect(block).height();
    }

    painter.restore();
}

void LineNumberEditor::updateCodeBlockState(const QString &text, CodeBlockInfo &info)
{
    for (int i = 0; i < text.length(); ++i) {
        QChar ch = text.at(i);

        // 处理字符串
        if (!info.inComment) {
            if (!info.inString && (ch == '\'' || ch == '"')) {
                info.inString = true;
                info.stringChar = ch.toLatin1();
                continue;
            } else if (info.inString && ch == info.stringChar) {
                // 检查是否是转义字符
                if (i > 0 && text.at(i-1) == '\\') {
                    // 转义字符，不结束字符串
                    continue;
                }
                info.inString = false;
                continue;
            }
        }

        // 处理注释（不在字符串中时）
        if (!info.inString) {
            // C风格注释
            if (!info.inComment && i+1 < text.length() &&
                ch == '/' && text.at(i+1) == '*') {
                info.inComment = true;
                i++; // 跳过下一个字符
                continue;
            } else if (info.inComment && i+1 < text.length() &&
                       ch == '*' && text.at(i+1) == '/') {
                info.inComment = false;
                i++; // 跳过下一个字符
                continue;
            }

            // C++单行注释
            if (!info.inComment && i+1 < text.length() &&
                ch == '/' && text.at(i+1) == '/') {
                // 单行注释，跳过本行剩余部分
                break;
            }

            // 更新代码块深度
            if (!info.inComment) {
                if (ch == '{' || ch == '[' || ch == '(') {
                    info.depth++;
                } else if (ch == '}' || ch == ']' || ch == ')') {
                    if (info.depth > 0) {
                        info.depth--;
                    }
                }
            }
        }
    }
}

void LineNumberEditor::deleteCurrentLine()
{
    QTextCursor cursor = textCursor();

    // 保存当前行的行号和列号
    int currentLine = cursor.blockNumber();
    int currentColumn = cursor.columnNumber();

    // 移动到行首
    cursor.movePosition(QTextCursor::StartOfLine);

    // 选择到行尾（包括换行符）
    cursor.movePosition(QTextCursor::EndOfLine, QTextCursor::KeepAnchor);

    // 如果当前行不是最后一行，也选中换行符
    if (!cursor.atEnd()) {
        cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
    }

    // 删除选中的文本
    cursor.removeSelectedText();

    // 如果文档不为空，调整光标位置
    if (!document()->isEmpty()) {
        // 移动到被删除行的下一行（如果存在）或上一行
        if (cursor.blockNumber() < document()->blockCount() - 1) {
            cursor.movePosition(QTextCursor::StartOfLine);
        }
    }

    // 设置光标
    setTextCursor(cursor);
}

// 添加大小写转换方法
void LineNumberEditor::convertCase()
{
    QTextCursor cursor = textCursor();

    // 如果没有选中文本，尝试选中当前单词
    if (!cursor.hasSelection()) {
        cursor.select(QTextCursor::WordUnderCursor);
    }

    QString selectedText = cursor.selectedText();
    if (selectedText.isEmpty()) {
        return;
    }

    // 判断当前选中的文本是什么格式
    QString convertedText;

    if (isAllUpperCase(selectedText)) {
        // 全大写 -> 首字母大写
        convertedText = toTitleCase(selectedText.toLower());
    } else if (isAllLowerCase(selectedText)) {
        // 全小写 -> 全大写
        convertedText = selectedText.toUpper();
    } else if (isTitleCase(selectedText)) {
        // 首字母大写 -> 全小写
        convertedText = selectedText.toLower();
    } else {
        // 混合大小写 -> 全小写（作为默认）
        convertedText = selectedText.toLower();
    }

    // 替换文本并保持选中状态
    int start = cursor.selectionStart();
    int end = cursor.selectionEnd();

    cursor.insertText(convertedText);

    // 重新选中刚转换的文本
    cursor.setPosition(start);
    cursor.setPosition(end, QTextCursor::KeepAnchor);
    setTextCursor(cursor);
}

// 辅助方法：检查是否全大写
bool LineNumberEditor::isAllUpperCase(const QString &text)
{
    for (const QChar &ch : text) {
        if (ch.isLetter() && !ch.isUpper()) {
            return false;
        }
    }
    return true;
}

// 辅助方法：检查是否全小写
bool LineNumberEditor::isAllLowerCase(const QString &text)
{
    for (const QChar &ch : text) {
        if (ch.isLetter() && !ch.isLower()) {
            return false;
        }
    }
    return true;
}

// 辅助方法：检查是否首字母大写（每个单词首字母大写）
bool LineNumberEditor::isTitleCase(const QString &text)
{
    QStringList words = text.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
    if (words.isEmpty()) return false;

    for (const QString &word : words) {
        if (word.isEmpty()) continue;

        // 检查首字母是否大写
        if (!word[0].isUpper()) {
            return false;
        }

        // 检查其余字母是否小写
        for (int i = 1; i < word.length(); ++i) {
            if (word[i].isLetter() && !word[i].isLower()) {
                return false;
            }
        }
    }
    return true;
}

// 辅助方法：转换为首字母大写格式（每个单词首字母大写）
QString LineNumberEditor::toTitleCase(const QString &text)
{
    QStringList words = text.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
    QStringList result;

    for (QString word : words) {
        if (word.isEmpty()) continue;

        // 将单词转换为小写，然后将首字母大写
        word = word.toLower();
        word[0] = word[0].toUpper();
        result.append(word);
    }

    return result.join(" ");
}

void LineNumberEditor::calculateFontMetrics()
{
    QFontMetrics fm(font());
    m_spaceWidth = fm.horizontalAdvance(' ');

    // 确保宽度有效
    if (m_spaceWidth <= 0) {
        m_spaceWidth = fm.averageCharWidth();
    }
}

void LineNumberEditor::updateTabStopDistance()
{
    if (m_spaceWidth > 0) {
        setTabStopDistance(m_tabStopSpaces * m_spaceWidth);
    }
}

void LineNumberEditor::resizeEvent(QResizeEvent *event)
{
    QPlainTextEdit::resizeEvent(event);

    if (m_lineNumberArea) {
        QRect cr = contentsRect();
        int width = lineNumberAreaWidth();
        m_lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), width, cr.height()));
    }
}

void LineNumberEditor::paintEvent(QPaintEvent *event)
{
    // 先绘制文本（包含语法高亮）
    QPlainTextEdit::paintEvent(event);

    // 再绘制缩进参考线（在文本之上）
    if (m_showIndentationGuides && event) {
        QPainter painter(viewport());
        drawIndentationGuides(painter, event->rect());
    }
}

void LineNumberEditor::updateLineNumberAreaWidth(int /*newBlockCount*/)
{
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void LineNumberEditor::updateLineNumberArea(const QRect &rect, int dy)
{
    if (!m_lineNumberArea) return;

    if (dy != 0) {
        m_lineNumberArea->scroll(0, dy);
    } else {
        m_lineNumberArea->update(0, rect.y(), m_lineNumberArea->width(), rect.height());
    }

    // 如果更新整个区域，重新计算宽度
    if (rect.contains(viewport()->rect())) {
        updateLineNumberAreaWidth(0);
    }
}

void LineNumberEditor::keyPressEvent(QKeyEvent *event)
{
    // 处理 Ctrl+D 删除整行
    if (event->key() == Qt::Key_D && event->modifiers() == Qt::ControlModifier) {
        deleteCurrentLine();
        event->accept();
        return;
    }

    // 处理 Ctrl+U 大小写转换
    if (event->key() == Qt::Key_U && event->modifiers() == Qt::ControlModifier) {
        convertCase();
        event->accept();
        return;
    }

    // 其他按键交给父类处理
    QPlainTextEdit::keyPressEvent(event);
}

void LineNumberEditor::highlightCurrentLine()
{
    if (isReadOnly()) {
        setExtraSelections({});
        return;
    }

    QList<QTextEdit::ExtraSelection> extraSelections;
    QTextEdit::ExtraSelection selection;

    selection.format.setBackground(m_currentLineColor);
    selection.format.setProperty(QTextFormat::FullWidthSelection, true);
    selection.cursor = textCursor();
    selection.cursor.clearSelection();

    extraSelections.append(selection);
    setExtraSelections(extraSelections);
}

void LineNumberEditor::setShowIndentationGuides(bool show)
{
    if (m_showIndentationGuides != show) {
        m_showIndentationGuides = show;
        if (show) {
            viewport()->update();
        }
    }
}

void LineNumberEditor::setIndentationGuideColor(const QColor &color)
{
    if (m_indentationGuideColor != color) {
        m_indentationGuideColor = color;
        if (m_showIndentationGuides) {
            viewport()->update();
        }
    }
}

void LineNumberEditor::setTabWidth(int width)
{
    if (width > 0 && width <= 8 && width != m_tabStopSpaces) {
        m_tabStopSpaces = width;
        updateTabStopDistance();

        if (m_showIndentationGuides) {
            viewport()->update();
        }
    }
}

void LineNumberEditor::setLineNumberColor(const QColor &color)
{
    if (m_lineNumberColor != color) {
        m_lineNumberColor = color;
        if (m_lineNumberArea) {
            m_lineNumberArea->update();
        }
    }
}

void LineNumberEditor::setCurrentLineColor(const QColor &color)
{
    if (m_currentLineColor != color) {
        m_currentLineColor = color;
        highlightCurrentLine();
    }
}

void LineNumberEditor::setLineNumberBackgroundColor(const QColor &color)
{
    if (m_lineNumberBgColor != color) {
        m_lineNumberBgColor = color;
        if (m_lineNumberArea) {
            m_lineNumberArea->update();
        }
    }
}
