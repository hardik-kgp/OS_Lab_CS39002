case ${1##*.} in
    "tar.bz2");&
    "tbz2");&
    "tbz") tar -xvjf $1;;
    "tgz");&
    "tar.gz") tar -xvzf $1;;
    "Z");&
    "gz") gzip -d $1;;
    "zip");&
    "7z");&
    "rar") 7z x $1;;
    "tar") tar -xvf $1;;
    "bz2") bzip2 -d $1;;
    *) echo "Unknown file format: cannot extract"
esac