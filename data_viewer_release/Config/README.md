# 配置文件说明
## 曲线配置
### Figure title: (figure name)
在括号和里面的文字使用你自己需要显示的名字命名（一定注意“:”后面的一个空格），例如如下设置：\
```Figure title: IMU数据```

![figure title](https://github.com/yuan5/Data_viewer/blob/main/image/figure_title.png)

### Line number: (n)
使用一个数字代替括号与括号中的内容，用于设置需要显示的数据有几组，例如如下设置：\
```Line number: 2```

可以显示两组数据，如上图所示的accx和accy

### Slide lenth: (n)
一个数据波形的长度设置，比如如下设置：\
```Slide lenth: 250```

数据滑动窗口是250个数据，也就是窗口中有249个历史数据和一个最新数据组成的波形曲线，很坐标代表的时间\

### Update period: (n)
数据显示更新周期，单位是ms，周期不是硬件定时，所以不是特别准，例如如下设置：\
```Update period: 10```

代表10毫秒更新一组显示数据。
