# onix

## bochs-gdb + vscode 调试

原生的 bochs 汇编调试工具太弱了，只有在调用汇编临界点的时候有些作用，比如任务调度时栈的信息，但是内核绝大部分代码还是使用 C 语言编写的，所以还是使用 C 的调试方式比较好。因此在写文件系统的时候，我用 C 模拟了底层汇编函数，但是总的来说还是和内核有些脱节。皇天不负有心人，让我找到了调试内核 C 代码的工具。下面开始介绍。

### bochs-gdb

由于使用的是 Archlinux，编译 bochs 都会遇到编译错误，所以使用了 AUR 的 bochs-gdb 包，但是最新的还是编译不过，所以就使用了 bochs-2.6.11，以下附上 `PKGBUILD`

```sh
# Maintainer: Jacob Garby <j4cobgarby@gmail.com>
# Contributor: Peter Vanusanik <admin@en-circle.com>
# stolen from bochs PKGBUILD in normal repos, but modified

pkgname=bochs-gdb
pkgver=2.6.11
pkgrel=3
pkgdesc="A portable x86 PC emulation software package with gdbstub"
arch=('x86_64')
url="http://bochs.sourceforge.net/"
license=('LGPL')
depends=('gcc-libs' 'libxrandr' 'libxpm' 'gtk2' 'bochs')
source=("http://downloads.sourceforge.net/sourceforge/bochs/bochs-$pkgver.tar.gz")
md5sums=('61dbf6d5c0384712e1f3e51e88381b4c')

prepare() {
    cd "$srcdir/bochs-$pkgver"
    # 4.X kernel is basically 3.20
    sed -i 's/2\.6\*|3\.\*)/2.6*|3.*|4.*)/' configure*
}

build() {
    cd "$srcdir/bochs-$pkgver"

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
        --enable-long-phy-address \
        --enable-pcidev \
        --enable-usb \
        --enable-gdb-stub 
        
    sed -i 's/^LIBS = /LIBS = -lpthread/g' Makefile
    make -j 1
}

package() {
    cd "$srcdir/bochs-$pkgver"
    make DESTDIR="$pkgdir" install
    install -Dm644 .bochsrc "$pkgdir/etc/bochsrc-sample.txt"
    
    cd "$pkgdir/usr/bin/"
    mv bochs bochs-gdb-a20
    rm bximage
    cd "$pkgdir/usr/"
    rm -rfv share
    cd "$pkgdir"
    rm -rfv etc
}
```

由于内核是 32 位的，所以去掉了一些选项，主要是 `--enable-x86-64` 以及依赖该选项的其他选项，然后执行编译安装

```sh
makepkg -si
```

### vscode 

vscode 中需要安装插件 **Native Debug**

然后写入调试配置

```json
{
    "type": "gdb",
    "request": "attach",
    "name": "onix gdb",
    "executable": "${workspaceFolder}/build/kernel.bin",
    "target": "localhost:1234",
    "remote": true,
    "cwd": "${workspaceFolder}/build",
    "gdbpath": "/usr/bin/gdb",
    "autorun": []
}
```

### 启动 bochs-gdb

```sh
bochs-gdb-a20 -f bochsrc.gdb -q
```

需要在 `bochsrc.gdb` 中打开 gdbstub 选项

```text
gdbstub: enabled=1, port=1234, text_base=0, data_base=0, bss_base=0
```

然后就可以愉快的玩耍了。

## 参考资料

- <https://aur.archlinux.org/packages/bochs-gdb/>
- <https://blog.csdn.net/zxb4221v/article/details/38701151>