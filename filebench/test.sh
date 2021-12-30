. ./../config
. ./../comm

FS=(eulerfs ext2-dax ext4 ext4dax nova)
TESTCASE=(varmail-n10k webproxy-n1M-d1K webserver-n1m-d1k fileserver-n100k-f16k fileserver-n100k fileserver-n500k-f1k)
NJ=(1 4 8 12 16)
COUNT=3

usage()
{
	cat <<-EOF
	Usage: $0 [-f fs|-t testcase|-n numjobs|-c testcount]

	-f fs: test filesystem, default ${FS[@]}
	-t testcase, default ${TESTCASE[@]}
	-n num: numjobs, default ${NJ[@]}
	-c testcount: default $COUNT
EOF
}

__fail()
{
	echo $*
	exit 1
}

while getopts f:t:n:c: opt
do
	case $opt in
	f)
		FS=("$OPTARG")
		;;
	t)
		TESTCASE=("$OPTARG")
		;;
	n)
		NJ=("$OPTARG")
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
echo "test case:        ${TESTCASE[@]}"
echo "test numjobs:     ${NJ[@]}"
echo "test count:       $COUNT"
echo "############################"

for t in ${TESTCASE[@]}; do
for fs in ${FS[@]}; do
for j in ${NJ[@]}; do
	echo "------ test $t, fs $fs, numjobs $j ------"
	for ((i=0; i<COUNT; ++i)); do
		echo "------ count $i ------"
		__mount $fs || __fail "mount $fs failed"
		numactl --cpunodebind=0	filebench -f config/${t}-${j}.f
		umount $TEST_DEV
	done
done
done
done
