#!/bin/bash 
if [ -z $1 ]; then
echo "Usage  : sh $0 username passwd"
echo "Example: sh $0 root 'passwd!@#2015'" 
fi 

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
#读取一行数据，用i做形参
cat ./hostlist.txt |while read i 
do
USER=$1
#从'hostlist.txt'文件中寻找IP
IP=`echo $i | awk '{print $1}'`
#echo " $# -- $1 --  $2"
#如果有一个参数,则从'hostlist.txt'文件中寻找密码
if [ $# -eq 1 ];then
    PASSWORD=`echo $i | awk '{print $2}'`
#如果有两个参数,把第二个参数当作密码
elif [ $# -eq 2 ];then
    PASSWORD=$2
else
    exit 
fi
sendSSHKey $USER $IP $PASSWORD
done 