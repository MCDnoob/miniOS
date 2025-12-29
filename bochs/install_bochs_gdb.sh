#!/bin/bash
set -e  # 出错立即退出

# 1. 安装依赖
echo "=== 安装编译依赖 ==="
#sudo apt update
#sudo apt install -y build-essential libx11-dev libxpm-dev libxrandr-dev libgtk2.0-dev wget


# 2. 卸载旧版本（如果存在）
echo "=== 卸载旧版本bochs ==="
if [ -f "bochs-2.7/Makefile" ]; then
    cd bochs-2.7
    sudo make uninstall || true  # 忽略卸载失败（如果没安装过）
    cd ..
fi
# 手动清理残留文件
sudo rm -rf /usr/bin/bochs* /usr/share/bochs || true


# 3. 下载并解压源码
echo "=== 下载并解压bochs源码 ==="
if [ ! -f "bochs-2.7.tar.gz" ]; then
    wget http://downloads.sourceforge.net/sourceforge/bochs/bochs-2.7.tar.gz
fi
tar -zxvf bochs-2.7.tar.gz -C . --overwrite  # 覆盖旧源码


# 4. 配置、编译
echo "=== 配置并编译bochs ==="
cd bochs-2.7
# 适配4.x内核
sed -i 's/2\.6\*|3\.\*)/2.6*|3.*|4.*)/' configure*

# 执行配置（和PKGBUILD参数一致）
./configure \
    --prefix=/usr \
    --without-wx \
    --with-x11 \
    --with-x \
    --with-term \
    --disable-docbook \
    --enable-cpu-level=6 \
    --enable-fpu \
    --enable-3dnow \
    --enable-disasm \
    --enable-long-phy-address \
    --enable-iodebug \
    --enable-x86-debugger \
    --enable-pcidev \
    --enable-usb \
    --enable-all-optimizations \
    --enable-gdb-stub \
    --with-nogui

# 修复链接选项
sed -i 's/^LIBS = /LIBS = -lpthread/g' Makefile

# 多线程编译
make -j$(nproc)


# 5. 安装并清理
echo "=== 安装并清理冗余文件 ==="
sudo make install
# 重命名为bochs-gdb
sudo mv /usr/bin/bochs /usr/bin/bochs-gdb
# 删除不需要的文件
sudo rm -rf /usr/bin/bochs-gdb-a20 /usr/bin/bximage || true


echo "=== 安装完成！可执行bochs-gdb启动 ==="