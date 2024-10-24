# EDA-Program
这个分支是我们项目文件的地址

## 仓库说明

这个分支是用来存储项目的代码（```.cpp```和```.h```）文件。

由于源文件、框架依赖项以及一些设置信息没办法上传(怕破环不同成员的本地环境)。我们将程序的执行结果(截图)和目前进度都放在这个```README```文档里面

## 项目结构

最初项目的分支结构如下：

```
/wxDrawingApp
|-- main.cpp             // 应用程序入口和主窗口
|-- MainFrame.h          // 主窗口类的头文件
|-- MainFrame.cpp        // 主窗口类的实现文件
|-- DrawingPanel.h       // 绘图面板类的头文件
|-- DrawingPanel.cpp     // 绘图面板类的实现文件
|-- Sidebar.h            // 工具栏类的头文件
|-- Sidebar.cpp          // 工具栏类的实现文件
|-- Shape.h              // 图形相关定义的头文件
```

## 项目进展截图

![](https://github.com/hock1024always/EDA-Program/blob/Prigram/picture/2024.10.10.png)

## 进展说明(日期表)

#### 2024.10.10

##### 实现了

1. 基本的背景和侧边框的效果
2. 点状背景图，限制图形和线条只能在点状图之间存在
3. 实现了几种```wx```框架中的基础图形，放置和平移

##### 预期解决事件

1. 优化闪频问题
2. 实现鼠标事件线条只能是横平竖直的
3. 实现图标的存储与调用来代替基础图形