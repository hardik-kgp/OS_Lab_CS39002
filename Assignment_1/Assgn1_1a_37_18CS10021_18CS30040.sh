if [[ $1 =~ "[0-9][0-9,]*" ]]; then echo "Error"; fi #doubt
gcd(){
    until test 0 -eq "$2";do set -- "$2" "$(($1 % $2))";done;if [ 0 -gt "$1" ];then echo "$((- $1))";else  echo "$1"; fi
}
IFS=',' read -a array <<< $1
for i in ${array[@]};do array[0]=$(gcd ${array[0]} $i) ;done;echo ${array[0]}