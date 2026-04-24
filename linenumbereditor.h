#ifndef LINENUMBEREDITOR_H
#define LINENUMBEREDITOR_H

#include "syntaxhighlighter.h"
#include <QWidget>
#include <QPlainTextEdit>
#include <QPainter>
#include <QTextBlock>
#include <QColor>

class LineNumberArea;

/**
 * @brief 带行号、缩进参考线和语法高亮的增强文本编辑器
 */
class LineNumberEditor : public QPlainTextEdit
{
    Q_OBJECT

public:
    explicit LineNumberEditor(QWidget *parent = nullptr);
    ~LineNumberEditor();

    // 行号区域相关
    void lineNumberAreaPaintEvent(QPaintEvent *event);
    int lineNumberAreaWidth() const;

    // 语法高亮相关
    void setFileName(const QString &fileName);
    QString getFileName() const { return m_fileName; }
    SyntaxHighlighter* getHighlighter() const { return m_highlighter; }

    // 缩进参考线相关
    void setShowIndentationGuides(bool show);
    bool isShowingIndentationGuides() const { return m_showIndentationGuides; }

    void setIndentationGuideColor(const QColor &color);
    QColor getIndentationGuideColor() const { return m_indentationGuideColor; }

    void setTabWidth(int width);
    int getTabWidth() const { return m_tabStopSpaces; }

    // 样式设置
    void setLineNumberColor(const QColor &color);
    QColor getLineNumberColor() const { return m_lineNumberColor; }

    void setCurrentLineColor(const QColor &color);
    QColor getCurrentLineColor() const { return m_currentLineColor; }

    void setLineNumberBackgroundColor(const QColor &color);
    QColor getLineNumberBackgroundColor() const { return m_lineNumberBgColor; }

    // 语法高亮控制
    void setSyntaxHighlightEnabled(bool enabled);
    bool isSyntaxHighlightEnabled() const { return m_syntaxHighlightEnabled; }

protected:
    void resizeEvent(QResizeEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private slots:
    void highlightCurrentLine();
    void updateLineNumberAreaWidth(int newBlockCount);
    void updateLineNumberArea(const QRect &rect, int dy);

    void keyPressEvent(QKeyEvent *event) override;

private:
    // 代码块状态结构体
    struct CodeBlockInfo {
        int depth = 0;          // 当前代码块深度
        bool inString = false;  // 是否在字符串中
        bool inComment = false; // 是否在注释中
        char stringChar = 0;    // 字符串起始字符（'或"）
    };

    // 私有方法
    void drawIndentationGuides(QPainter &painter, const QRect &rect);
    void calculateFontMetrics();
    void updateTabStopDistance();
    void setupHighlighter();

    // 代码块状态管理
    void updateCodeBlockState(const QString &text, CodeBlockInfo &info);

    // 删除整行
    void deleteCurrentLine();
    // 大小写转换
    void convertCase();
    // 大小写转换辅助方法
    bool isAllUpperCase(const QString &text);
    bool isAllLowerCase(const QString &text);
    bool isTitleCase(const QString &text);
    QString toTitleCase(const QString &text);

    // 辅助方法
    qreal calculateLineNumberAreaWidth() const;
    void drawLineNumber(QPainter &painter, const QTextBlock &block, int y);
    QString detectFileType(const QString &suffix);

    // 私有成员
    LineNumberArea *m_lineNumberArea;
    SyntaxHighlighter *m_highlighter;
    QString m_fileName;

    // 行号相关
    int m_lineNumberDigits;
    QColor m_lineNumberColor;
    QColor m_lineNumberBgColor;
    QColor m_currentLineColor;

    // 缩进参考线相关
    bool m_showIndentationGuides;
    QColor m_indentationGuideColor;
    int m_tabStopSpaces;
    qreal m_spaceWidth;

    // 语法高亮相关
    bool m_syntaxHighlightEnabled;

    // 性能优化缓存
    mutable int m_cachedLineNumberWidth;
    mutable int m_cachedBlockCount;
};

class LineNumberArea : public QWidget
{
public:
    explicit LineNumberArea(LineNumberEditor *editor)
        : QWidget(editor), m_editor(editor)
    {
        setAttribute(Qt::WA_OpaquePaintEvent);
    }

    QSize sizeHint() const override
    {
        return QSize(m_editor->lineNumberAreaWidth(), 0);
    }

protected:
    void paintEvent(QPaintEvent *event) override
    {
        m_editor->lineNumberAreaPaintEvent(event);
    }

private:
    LineNumberEditor *m_editor;
};

#endif // LINENUMBEREDITOR_H
