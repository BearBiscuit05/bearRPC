#!/bin/bash

function sendSSHKey(){ 
SSH='ssh-copy-id -i' 
HOME_DIR=`cd ~ && pwd` 
KEYSTORE=$HOME_DIR/.ssh/id_rsa.pub 
# $SSH $KEYSTORE ${1}@${2}
# 三个参数是'user'--'ip'--'password'
echo "========$#  -- $1 -- $2 -- $3 ========="
if [ ! $2 ] || [ ! $3 ] ;then
    echo "ip/password not found !" 
    exit
else
expect -c "
    spawn $SSH $KEYSTORE ${1}@${2}
    expect {
    \"*yes/no*\" {send \"yes\r\"; exp_continue}
    \"*password*\" {send \"$3\r\"; exp_continue}
    \"*Password*\" {send \"$3\r\";}
}
"
echo -e "\033[40;32m send sshkey to $2 success \033[0m\n"
fi
} 

echo y | apt install expect
cat /home/dgl/workspace/ip_config.txt |while read IP
do
PASSWORD='Xiangyongan05'
USER='root'
sendSSHKey $USER $IP $PASSWORD
done
