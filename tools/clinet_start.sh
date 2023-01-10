#!/bin/bash
if [ -z $1 ];then
    echo "ERROR:未输入MASTER IP..."	
    exit 1
fi
echo yes | sudo apt-get install nfs-common
mkdir -p /home/dgl/workspace
rm /usr/bin/python3 && ln -s /root/miniconda3/bin/python3 /usr/bin/python3
sudo mount -t nfs $1:/home/dgl/workspace /home/dgl/workspace
ifconfig | grep inet | grep broadcast | cut -c 14-26 >> /home/dgl/workspace/ip_config.txt

if test -e /root/.ssh/id_rsa.pub
then
        echo "密钥已经存在，不再重新生成"
else
        ssh-keygen -t rsa -P '' -f ~/.ssh/id_rsa  #生成密钥
fi