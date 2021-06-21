# 对内核补丁 [[V3,0/2] sched/fair: Fallback to sched-idle CPU in absence of idle CPUs](https://lore.kernel.org/patchwork/cover/1094197/) 测试
-------

## <center>环境</center>
```
vizdl@ubuntu:~/make/linux-master/linux$ uname -a
Linux ubuntu 5.4.0-73-generic #82~18.04.1-Ubuntu SMP Fri Apr 16 15:10:02 UTC 2021 x86_64 x86_64 x86_64 GNU/Linux
```
由于条件有限,采用了虚拟机进行测试
```
sudo apt install libncurses5 libncurses5-dev libssl-dev \
build-essential openssl zlibc minizip \
libidn11-dev libidn11 bison flex libelf-dev -y
```
内核编译与安装可能需要的依赖库安装。
```
sudo apt install gcc make git -y
```
内核编译依赖工具安装
```
sudo git clone https://gitee.com/mirrors/linux.git
cd linux
```
在`gitee`上拉取`linux`项目,并进入项目。
```
sudo git checkout -b fall-back-vk-before
sudo git reset 84ec3a0787086fcd25f284f59b3aa01fd6fc0a5d --hard
```
以`master`分支为基础创建并切换至分支`fall-back-vk-before`,再将分支 `fall-back-vk-before`硬重置至补丁提交之前。
## <center>配置</center>
```
sudo cp /boot/config-5.4.0-42-generic ./.config
```
将测试机`/boot/config*`配置拷贝到项目内。
```
sudo make menuconfig
```
通过窗口进行配置
## <center>编译</center>
```
sudo make bzImage -j128
```
多线程编译镜像
```
sudo make modules -j128
```
编译内核模块
## <center>安装</center>
```
sudo make modules_install
```
安装内核动态加载模块
```
make install 
```
安装内核