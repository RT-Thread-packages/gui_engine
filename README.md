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

## 4、如何使用触摸框架

#### 4.1 开启touch drivers选项

- 在 GUI Engine 中开启`Support touch`配置
- 在peripherals中开启以下配置 并选择合适芯片

```
RT-Thread online package
    peripherals package
        [*] touch drivers
```

### 4.2 触摸设备配置选项说明
- 选择自己的触摸IC,如果没有可以自己添加自己的[芯片适配](https://github.com/jhbdream/ft3x07) 并制作[软件包](https://www.rt-thread.org/document/site/development-guide/package/package)，丰富我们的触摸设备支持

```
Select Touch IC
```

- 设置触摸线程采样率 (单位HZ)

```
Set the touch screen sample rate
```

- 设置IIC总线设备名称

```
Set the IIC bus device name
```

### 4.3 注意事项
- **在源代码中开启宏定义 `DBG_ENABLE` 选项时，会在msh窗口打印触摸坐标点用于测试当前驱动是否正常工作**

- **测试完毕之后，建议关闭此选项重新编译工程，因为在调试信息打印过程中会占用部分处理器资源**

- **使用TOUCH设备依赖于IIC总线 请开启IIC设备支持**

### 4.4 目前支持的触摸芯片

- [x] [FT3X07](https://github.com/jhbdream/ft3x07)

