# ink ✒️

> 纯粹的文本编辑器，写给干净的文字。

## 缘起

2024 年底，Notepad++ 作者在软件中植入辱华政治言论，令众多中文用户感到愤怒与失望。与此同时，其替代品 Notepad-- 在功能与体验上又差强人意。

作为一名程序员，与其等待别人做出改变，不如自己动手。

## 初衷

- 🚫 **拒绝政治绑架** —— 代码是代码，工具是工具，干净纯粹。
- 🛠️ **技术练手项目** —— 借此学习 [Qt] 的实际应用。
- 🎯 **够用就好** —— 不做大而全，只做轻而美。打开即写，写完即走。

## 为什么叫 ink？

墨水，书写最本质的载体。没有多余的花哨，回归文字本身。四个字母，简洁如它的名字。

## 特性

- 📝 **多标签页编辑** - 支持同时打开多个文件
- 🌳 **树形文件列表** - 按目录结构组织打开的文件
- 🔍 **强大的查找替换** - 支持正则表达式、高亮所有匹配
- 🎨 **语法高亮** - 支持 C/C++、Python、Java、JavaScript、Markdown、HTML、CSS 等
- 📏 **行号显示** - 清晰的左侧行号区域
- 📐 **缩进参考线** - 智能显示代码块的缩进层级
- 👁️ **Markdown 预览** - 实时预览 Markdown 文件
- ⚡ **高效快捷键** - Ctrl+D 删除整行，Ctrl+U 大小写转换
- ⌨️ **全局快捷键** - Ctrl+F/R 全局查找替换，F3/Shift+F3 查找下一个/上一个
- 💾 **会话管理** - 自动保存和恢复最近打开的文件

## 安装

### 前提条件

- Qt 5 或 Qt 6
- CMake 3.16 或更高版本
- C++17 编译器

### 从源码编译

```bash
# 克隆仓库（如果是从Git获取）
git clone <repository-url>
cd ink

# 创建构建目录
mkdir build
cd build

# 配置项目
cmake ..

# 编译
cmake --build .

# 运行
./ink
```

### Windows 平台

```bash
# 使用 Visual Studio 或 MinGW
# 确保 Qt 环境已配置
cmake -G "Visual Studio 17 2022" ..
# 或者
cmake -G "MinGW Makefiles" ..

# 然后使用 Visual Studio 打开 .sln 文件或运行 make
```

## 使用方法

### 基本操作

1. **打开文件** - 菜单栏「文件」→「打开」或工具栏打开按钮
2. **新建文件** - Ctrl+N 或菜单栏「文件」→「新建」
3. **保存文件** - Ctrl+S 或菜单栏「文件」→「保存」
4. **关闭标签页** - Ctrl+W 或点击标签页关闭按钮

### 编辑功能

1. **查找** - Ctrl+F 或菜单栏「编辑」→「查找」
2. **替换** - Ctrl+R 或菜单栏「编辑」→「替换」
3. **删除整行** - Ctrl+D
4. **大小写转换** - Ctrl+U
5. **查找下一个** - F3
6. **查找上一个** - Shift+F3

### 视图控制

1. **显示/隐藏文件列表** - Ctrl+Shift+F 或菜单栏「视图」→「文件列表」
2. **显示/隐藏预览面板** - Ctrl+Shift+P 或菜单栏「视图」→「预览面板」

## 项目结构

```
ink/
├── CMakeLists.txt          # CMake 构建配置
├── main.cpp               # 程序入口
├── mainwindow.cpp/h       # 主窗口
├── linenumbereditor.cpp/h # 带行号的编辑器
├── findreplacedialog.cpp/h # 查找替换对话框
├── syntaxhighlighter.cpp/h # 语法高亮器
└── README.md              # 项目说明
```

## 开发

### 添加新的语法高亮支持

1. 在 `syntaxhighlighter.h` 中添加新的文件类型枚举
2. 在 `syntaxhighlighter.cpp` 的 `detectFileType()` 函数中添加文件后缀识别
3. 在 `highlightBlock()` 函数中添加新的 case 分支
4. 实现对应的 `initXxxRules()` 函数

### 自定义图标

项目使用 Qt 资源系统管理图标。图标文件位于资源文件中，可以通过 `:/icons/xxx` 路径引用。

## 许可证

本项目采用 MIT 许可证。详见 LICENSE 文件（待添加）。

## 贡献

欢迎提交 Issue 和 Pull Request！

## 待办事项

- [ ] 添加更多编程语言的语法高亮支持
- [ ] 实现代码折叠功能
- [ ] 添加主题切换支持
- [ ] 实现插件系统
- [ ] 优化性能和大文件处理
- [ ] 添加自动补全功能
