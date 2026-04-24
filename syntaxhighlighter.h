#ifndef SYNTAXHIGHLIGHTER_H
#define SYNTAXHIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QRegularExpression>

class SyntaxHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT

public:
    explicit SyntaxHighlighter(QTextDocument *parent = nullptr);

protected:
    void highlightBlock(const QString &text) override;

private:
    struct HighlightingRule
    {
        QRegularExpression pattern;
        QTextCharFormat format;
    };
    QVector<HighlightingRule> highlightingRules;

    // C++高亮规则
    void initCppRules();
    void initPythonRules();
    void initJavaScriptRules();
    void initMarkdownRules();
    void initHTMLRules();
    void initCSSRules();
    void initJavaRules();

    // 检测文件类型
    enum FileType {
        Unknown,
        Cpp,
        Python,
        JavaScript,
        Markdown,
        HTML,
        CSS,
        Java
    };

    FileType detectFileType(const QString &fileName);

    // 格式定义
    QTextCharFormat keywordFormat;
    QTextCharFormat classFormat;
    QTextCharFormat singleLineCommentFormat;
    QTextCharFormat multiLineCommentFormat;
    QTextCharFormat quotationFormat;
    QTextCharFormat functionFormat;
    QTextCharFormat numberFormat;
    QTextCharFormat preprocessorFormat;
    QTextCharFormat markdownHeaderFormat;
    QTextCharFormat markdownBoldFormat;
    QTextCharFormat markdownItalicFormat;
    QTextCharFormat markdownCodeFormat;
};

#endif // SYNTAXHIGHLIGHTER_H
