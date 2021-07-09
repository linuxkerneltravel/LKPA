# <center>ubuntu 18.04 编译内核</center>
-------

## <center>环境</center>
```
vizdl@ubuntu:~/make/linux-master/linux$ uname -a
Linux ubuntu 5.4.0-73-generic #82~18.04.1-Ubuntu SMP Fri Apr 16 15:10:02 UTC 2021 x86_64 x86_64 x86_64 GNU/Linux
```
-------
## <center>配置</center>
#### 添加编译工具
```
sudo apt install gcc make -y
```
#### 添加内核编译依赖包
```
sudo apt install libncurses5 libncurses5-dev libssl-dev \
build-essential openssl zlibc minizip \
libidn11-dev libidn11 bison flex libelf-dev -y
```
## <center>编译</center>
#### 查看编译选项
```
make help
```
#### 创建`x86_64`默认配置
```
make x86_64_defconfig
```
#### 使用菜单栏设置内核编译配置
```
make menuconfig
```
#### 编译镜像
```
make bzImage -j128
```
#### 编译模块
```
sudo make modules
```
#### 清理
```
make clean
```
清理大多数编译生成的文件，但会保留config文件等
```
make mrproper
```
清理所有编译生成的文件、config及某些备份文件
```
make distclean
```
清理mrproper、patches以及编辑器备份文件
## <center>安装</center>
#### 安装内核
```
make install
```
#### 安装内核模块
```
sudo make modules_install
```
