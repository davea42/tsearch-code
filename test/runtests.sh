#!/bin/sh 
# Run this in the test directory.
#
savetemps=n
rmtemps() {
  rm -f testpasserrs
  rm -f testpass
  rm -f testfails
}
rmtemps
stsecs=`date '+%s'`

top_blddir=`pwd`/..
if [ x$DWTOPSRCDIR = "x" ]
then
  top_srcdir=$top_blddir
else
  top_srcdir=$DWTOPSRCDIR
fi
srcdir=$top_srcdir/test
loc=`pwd`
ct=0
if [ ! -f $top_blddir/src/redblacksearch ]
then
   echo "Build the search code to test"
   echo "No test done"
   exit 1
fi

echo "Start tsearch tests"
date 
failcount=0
for opts in "--std" "$srcdir/testwords" "$srcdir/sortedwords"
do
  for app  in  $top_blddir/src/binarysearch $top_blddir/src/eppingerdel \
    $top_blddir/src/balancedsearch $top_blddir/src/redblacksearch \
    $top_blddir/src/hashsearch $top_blddir/src/libcsearch 
  do
    name=`basename  $app`
    echo "==== $app $opts ===="
    /usr/bin/time $app $opts  > testout
    grep FAIL <testout >/dev/null
    if [ $? -eq  0 ]
    then
        failcount=`expr $failcount + 1 `
        cat testout
    fi
  done
done
if [ $failcount -ne 0 ]
then
   echo "FAIL count $failcount"
   exit 1
fi
echo PASS
exit 0
