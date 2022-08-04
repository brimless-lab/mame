* 初始环境: emsdk目录下, 执行`source ./emsdk_env.sh`

* 编译: 同一终端环境, emsdk目录下, 执行: `emmake make -j5 SUBTARGET=1943 SOURCES=src/mame/drivers/1943.cpp REGENIE=1` 

* 运行: 放到emularity对应目录下, 主要分为rom包和模拟器文件(本次编译生成):

  eg. 模拟器文件路径: `emularity/emulators/jsmess`

* 更多问题, 参考notion里面的记录;
