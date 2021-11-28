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
if [ ! -x $top_blddir/src/readelfobj ]
then
   echo "Build readelfobj and install it as $loc/../src/readelfobj to test"
   echo "No test done"
   exit 1
fi


echo "Start tsearch tests"
date 
x=`pwd` 
if [ ! -f RUNTEST ]
then
   echo "Run RUNTEST in the test directory, not in "
   pwd
   exit 1
fi
cd ../src
make
cd $x
pwd 
echo 
for opts in "--std" "testwords" "sortedwords"
do
  for app  in  $top_blddir/src/binarysearch $top_blddir/src/eppingerdel \
    $top_blddir/src/balancedsearch $top_blddir/src/redblacksearch \
    $top_blddir/src/hashsearch $top_blddir/src/libcsearch 
  do
    name=`basename  $app`
    echo "==== $app $opts ===="
    /usr/bin/time $app $opts  
  done
done
exit 0
