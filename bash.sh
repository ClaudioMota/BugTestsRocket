set -e 2
test=$(build/tests/test)
if [ $? != 2 ]; then
  exit 1
fi