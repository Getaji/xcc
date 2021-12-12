handleSIGINT()
{
  printf "\n"
  exit 0
}

trap handleSIGINT SIGINT

xccHash=`md5sum xcc`
replHash=`md5sum xcc-repl.sh`;
echo "xcc (C subset compiler) ${xccHash:0:8}"
echo "xcc-repl (use gcc) ${replHash:0:8}"
printf "> "
while read line
do
  ./xcc "$line" > tmp.s &&
  gcc -o tmp samplefn.o tmp.s &&
  ./tmp
  echo "$?"
  printf "> "
done
