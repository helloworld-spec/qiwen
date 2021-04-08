压缩解压缩命令.tar解包：tar xvf xxx.tar
打包：tar cvf xxx.tar DirName
（注：tar是打包，不是压缩！）
.gz解压1：gunzip FileName.gz
解压2：gzip -d FileName.gz
压缩：gzip FileName
 .tar.gz 和 .tgz解压：tar zxvf FileName.tar.gz
压缩：tar zcvf FileName.tar.gz DirName
.bz2解压1：bzip2 -d FileName.bz2
解压2：bunzip2 FileName.bz2
压缩： bzip2 -z FileName
.tar.bz2解压：tar jxvf FileName.tar.bz2
压缩：tar jcvf FileName.tar.bz2 DirName
.bz解压1：bzip2 -d FileName.bz
解压2：bunzip2 FileName.bz
.tar.bz解压：tar jxvf FileName.tar.bz
压缩：未知
.Z解压：uncompress FileName.Z
压缩：compress FileName
 .tar.Z解压：tar Zxvf FileName.tar.Z
压缩：tar Zcvf FileName.tar.Z DirName
.zip解压：unzip FileName.zip
压缩：zip FileName.zip DirName
.rar解压：rar x FileName.rar
压缩：rar a FileName.rar DirName
.lha解压：lha -e FileName.lha
压缩：lha -a FileName.lha FileName
.rpm解包：rpm2cpio FileName.rpm | cpio -div

