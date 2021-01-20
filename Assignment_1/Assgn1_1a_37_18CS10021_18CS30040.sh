if [[ $1 =~ "[0-9][0-9,]*" ]]; then echo "Error";exit 1; fi 
gcd(){
    until test 0 -eq "$2";do set -- "$2" "$(($1 % $2))";done;if [ 0 -gt "$1" ];then echo "$((- $1))";else  echo "$1"; fi
}
IFS=',' read -a array <<< $1
cnt=0
for i in ${array[@]};do array[0]=$(gcd ${array[0]} $i);((cnt=cnt+1));done;
if [[ $cnt -gt 10 ]]; then echo "Error"; exit 1 ;fi
echo ${array[0]}
