upp=$(tr -dc 'A-Z' </dev/urandom|head -c1)
low=$(tr -dc 'a-z' </dev/urandom|head -c1)
dig=$(tr -dc '0-9' </dev/urandom|head -c1)
spe=$(tr -dc '_' </dev/urandom|head -c1)
printf $upp$low$dig$spe$(cat /dev/urandom | tr -dc A-Za-z0-9_ | head -c $((${1:-16}-4)))|sed 's/./&\n/g'|shuf|tr -d "\n"

