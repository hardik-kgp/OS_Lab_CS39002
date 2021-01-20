mkdir 1.b.files.out
for file in $(ls ../1.b.files)
do
    cat "../1.b.files/${file}" | sort -n -o "1.b.files.out/${file}"
done
sort -mnr ../1.b.files/* -o "1.b.out.txt"

#tested but may need to check again the file