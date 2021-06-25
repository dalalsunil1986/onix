# onix - 一个操作系统的设计和实现

[本文档地址](https://github.com/StevenBaby/onix)

## 开发环境

---

### bochs-gdb

在 `build/bochs-gdb` 目录执行如下命令，安装 `bochs-gdb`，只针对 Archlinux。

```sh
makepkg -si
```

### qemu

```makefile
.PHONY:qemu
qemu: $(BUILD)/master.img
	qemu-system-i386 \
	-no-reboot \
	-boot c \
	-drive file=$<,format=raw,index=0,media=disk \
	-m 32 \
	-d cpu_reset,int \
	# -monitor stdio \
	# -S	\
	# -gdb tcp::1234 \
```

### VMWare

首先，可使用 `qemu-img` 将 `raw` 硬盘格式转换为 `vmdk` 格式：

```makefile
$(BUILD)/master.vmdk: $(BUILD)/master.img
	qemu-img convert -pO vmdk $< $@
```

然后新建虚拟机选择已存在的磁盘，然后，选择转换好的 `.vmdk` 文件。

如需调试，需要在配置文件 `.vmx` 中加如如下语句：

```
debugStub.listen.guest32.remote = "TRUE"
debugStub.listen.guest64.remote = "TRUE"
monitor.debugOnStartGuest32 = "TRUE"
```

然后可在本地 gdb 远程调试 端口号为 8832

```gdb
target remove :8832
```

---

## 参考资料

如有遗漏，后期再补

### 网页

- <https://wiki.osdev.org/ATA_PIO_Mode>
- <https://wiki.osdev.org/VMware>

### 书籍

- [王爽 - 汇编语言](https://book.douban.com/subject/3037562/)
- [于渊 - Orange'S:一个操作系统的实现](https://book.douban.com/subject/3735649/)
- [[日] 川合秀实 - 30天自制操作系统](https://book.douban.com/subject/11530329/)
- [赵炯 - Linux内核完全注释](https://book.douban.com/subject/1231236/)
- [李忠 & 王晓波 & 余洁 - X86汇编语言](https://book.douban.com/subject/20492528/)
- [郑钢 - 操作系统真象还原](https://book.douban.com/subject/26745156/)
- [Richard Blum - 汇编语言程序设计](https://book.douban.com/subject/1446250/)
- [俞甲子 & 石凡 & 潘爱民 - 程序员的自我修养](https://book.douban.com/subject/3652388/)

