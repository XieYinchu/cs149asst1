import matplotlib.pyplot as plt
import numpy as np

x_axis_data = [2,3,4,5,6,7,8] #x
y_axis_data = [1.88,1.60,2.07,1.94,2.52,2.53,2.88] #y

plt.plot(x_axis_data, y_axis_data, 'b*--', alpha=0.5, linewidth=1, label="view1")#'bo-'表示蓝色实线，数据点实心原点标注
## plot中参数的含义分别是横轴值，纵轴值，线的形状（'s'方块,'o'实心圆点，'*'五角星   ...，颜色，透明度,线的宽度和标签 ，

x_axis_data2 = [2,3,4,5,6,7,8] #x
y_axis_data2 = [1.51,1.88,2.10,2.49,2.83,2.88,3.11] #y

plt.plot(x_axis_data2, y_axis_data2, 'r*--', alpha=0.5, linewidth=1, label="view2")#'bo-'表示蓝色实线，数据点实心原点标注

plt.legend()  #显示上面的label
plt.xlabel('thread') #x_label
plt.ylabel('speedup')#y_label
 
#plt.ylim(-1,1)#仅设置y轴坐标范围
plt.show()
