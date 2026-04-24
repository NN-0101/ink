#ifndef FINDREPLACEDIALOG_H
#define FINDREPLACEDIALOG_H

#include <QDialog>
#include <QPlainTextEdit>

class QLineEdit;
class QCheckBox;
class QPushButton;
class QLabel;
class QPlainTextEdit;

class FindReplaceDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FindReplaceDialog(QWidget *parent = nullptr);
    ~FindReplaceDialog();

    // 设置编辑器
    void setTextEdit(QPlainTextEdit *textEdit);

    // 显示查找框
    void showFind();

    // 显示替换框
    void showReplace();

    // 公共接口函数
    void findNextMatch();
    void findPrevMatch();

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void closeEvent(QCloseEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;

private slots:
    void onFindNext();
    void onFindPrev();
    void onReplace();
    void onReplaceAll();
    void onFindTextChanged(const QString &text);
    void onHighlightAll();
    void updateButtonStates();

private:
    // 查找相关
    void findNext(bool forward = true);
    void findAllMatches();
    void highlightAllMatches();
    void clearAllHighlights();
    QList<QTextCursor> findMatches(const QString &text, bool forward = true, int startPos = -1);

    // 替换相关
    void replaceCurrent();
    void replaceAll();

    // 获取当前编辑器的选中范围
    QTextCursor getCurrentSelection();

private:
    QPlainTextEdit *m_textEdit;

    // UI组件
    QLineEdit *m_findEdit;
    QLineEdit *m_replaceEdit;
    QCheckBox *m_caseSensitiveCheck;
    QCheckBox *m_wholeWordCheck;
    QCheckBox *m_regexCheck;
    QCheckBox *m_highlightAllCheck;
    QPushButton *m_findNextBtn;
    QPushButton *m_findPrevBtn;
    QPushButton *m_replaceBtn;
    QPushButton *m_replaceAllBtn;
    QPushButton *m_closeBtn;
    QLabel *m_statusLabel;

    // 状态
    bool m_isReplaceMode;
    QList<QTextEdit::ExtraSelection> m_highlightSelections;
    QList<QTextCursor> m_currentMatches;
    int m_currentMatchIndex;
};

#endif // FINDREPLACEDIALOG_H
