. ./../config
. ./../comm

FS=(eulerfs ext2-dax ext4 ext4dax nova)
NUM=500000
DIRTEST=0
SYNC=0
COUNT=3

usage()
{
	cat <<-EOF
	Usage: $0 [-f fs|-d|-s|-c testcount|-n file/dir count]

	-f fs: test filesystem, default ${FS[@]}
	-n file/dir count, default $NUM
	-d: enable will test dir, or test file otherwise, default $DIRTEST
	-s: sync, default $SYNC
	-c testcount: default $COUNT
EOF
}

__fail()
{
	echo $*
	exit 1
}

while getopts dsf:n: opt
do
	case $opt in
	f)
		FS=("$OPTARG")
		;;
	n)
		NUM=$OPTARG
		;;
	d)
		DIRTEST=1
		;;	
	s)
		SYNC=1
		;;
	c)
		COUNT=$OPTARG
		;;
	h|?)
		usage
		exit 1
		;;
	esac
done

echo "start filebench test on $TEST_DEV"
echo "############################"
echo "test fs:          ${FS[@]}"
echo "test num:         $NUM"
echo "test dir?         $DIRTEST"
echo "test sync?        $SYNC"
echo "test count:       $COUNT"
echo "############################"

for fs in ${FS[@]}; do
	echo "------ test $fs, dirtest? $DIRTEST, sync? $SYNC ------"
	for ((i=0; i<COUNT; ++i)); do
		echo "------ count $i ------"
		__mount $fs || __fail "mount $fs failed"
		./filetest -n $NUM -s $SYNC -t $DIRTEST -d $MNT_POINT
		umount $TEST_DEV
	done
done
