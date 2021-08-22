./configure --prefix=$PWD/out --host=arm-anykav200-linux-uclibcgnueabi
make
make install


使用时，将preloadable_libiconv.so 重命名为libiconv.so 或者libiconv.so.2.5.0 并且建立libiconv.so的连接

此库已经过裁剪
