#!/bin/bash

# 编译脚本

flag=""

help() {
    echo "./compile [argv]
        argv可取:
            verbose:打印详细信息"
}

# 参数是要执行的内容
echo_and_eval() {
    echo $1
    eval $1
}

# 仅make
if [[ $1 == "make" ]]
then
    echo_and_eval "cd build && make && cd -"
    exit
fi

for arg in $*;
do
    if [[ ${arg} =~ "help" ]]
    then
        help
        exit
    elif [[ ${arg} =~ "verbose" ]]
    then
        flag+=" VERBOSE=1"
    fi
done

echo_and_eval "[ -d "build" ] || mkdir build"

# 编译命令
echo_and_eval "cd build && cmake .. && make -j4 ${flag}"



