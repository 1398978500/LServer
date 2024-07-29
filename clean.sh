#!/bin/bash

help() {
    echo "./clean all 清理包括cmake的所有缓存 cmake文件有修改时要用此命令"
}

# 参数是要执行的内容
echo_and_eval() {
    echo $1
    eval $1
}


if [[ $1 =~ "help" ]]
then
    help
    exit
elif [[ $1 == "all" ]]
then
    echo_and_eval "rm ./build -r"
    exit
else
    echo_and_eval "cd build && make clean && cd -"
    exit
fi


