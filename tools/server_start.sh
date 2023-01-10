#!/bin/bash
echo yes | sudo apt-get install nfs-kernel-server
echo "/home/dgl/workspace  172.16.0.0/12(rw,sync,no_subtree_check)" >> /etc/exports
sudo systemctl restart nfs-kernel-server
rm /usr/bin/python3 && ln -s /root/miniconda3/bin/python3 /usr/bin/python3
ifconfig | grep inet | grep broadcast | cut -c 14-26 >> /home/dgl/workspace/ip_config.txt

if test -e /root/.ssh/id_rsa.pub
then
        echo "密钥已经存在，不再重新生成"
else
        ssh-keygen -t rsa -P '' -f ~/.ssh/id_rsa  #生成密钥
fi