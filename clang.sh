find ./ -path ./lib -prune  -o -name "*.c" -o -name "*.h"|grep -v "./lib"|xargs clang-format --style=file -i
find ./ -path ./lib -prune  -o -name "*.c" -o -name "*.h"|grep -v "./lib"|xargs dos2unix