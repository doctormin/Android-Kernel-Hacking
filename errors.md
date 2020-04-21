# 以此纪念我在prj1中踩的坑
## helloWorld体验阶段
+ ndk-build报错  ---- 原来是jni目录命名成了JNI 
## ptree module阶段
+ 代码补全问题，#include<linux/init.c>就是不被识别
+ make失败 ---- 没有添加ndk/toolchain/..的环境变量，添加完了并重启之后就ok了
+ 