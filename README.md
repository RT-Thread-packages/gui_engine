# GUI Engine

## 1、介绍

GUI engine是一套基本的绘图引擎，由C代码编写而成。

主要的功能包括：
- 基于绘图设备DC的绘图操作，包括点，线，圆，椭圆，多边形（填充）等；
- 各类图像格式加载（从文件系统中加载，需要DFS文件系统）及绘图；
- 各种字体的文本显示；
- GUI的C/S架构及基础的窗口机制，事件框架机制等。

## 2、获取方式

-  Git方式获取：

    git clone https://github.com/RT-Thread-packages/gui_engine.git

- env工具辅助下载：

menuconfig package path：

    RT-Thread online package
        system package
            [*] RT-Thread UI Engine
                [*] Enable UI Engine

## 3、示例介绍

### 3.1 获取示例

* 配置使能示例选项 `Enable the example of Gui Engine`
* 配置包版本选为最新版 `latest`
