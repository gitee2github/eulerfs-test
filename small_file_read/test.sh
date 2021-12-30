. ./../config
. ./../comm

FS=(eulerfs ext4dax ext4dax-fc ext4dax-nojournal xfsdax)
SIZE=(4096 65536 2097152) #4k 64k 2MB
BS=(4096 65536 1048576) #4k 64k 1MB
COUNT=10000
ASYNC=0

usage()
{
	cat <<-EOF
	Usage: $0 [-f fs|-s size|-b bs|-c count|-a]

	-f fs: test filesystem, default ${FS[@]}
	-s size: total file size, default ${SIZE[@]}
	-b bs: read / write size, default ${BS[@]}
	-c count: num files to test default $COUNT
	-a : do fsync after each file operation
EOF
}

__fail()
{
	echo $*
	exit 1
}

while getopts f:s:b:c:a opt
do
	case $opt in
	f)
		FS=("$OPTARG")
		;;
	s)
		SIZE=("$OPTARG")
		;;
	b)
		BS=("$OPTARG")
		;;
	c)
		COUNT=$OPTARG
		;;
	a)
		ASYNC=1
		;;
	h|?)
		usage
		exit 1
		;;
	esac
done

echo "start small file rw test on $TEST_DEV"
echo "############################"
echo "test fs:          ${FS[@]}"
echo "test file size:   ${SIZE[@]}"
echo "test bs:          ${BS[@]}"
echo "test count:       $COUNT"
echo "async:            $ASYNC"
echo "############################"

for fs in ${FS[@]}; do
	for size in ${SIZE[@]}; do
		for bs in ${BS[@]}; do
			if [ $bs -eq 0 ] || [ $bs -gt $size ]; then
				continue
			fi

			__mount $fs || __fail "mount $fs failed"
			if [ $ASYNC -eq 1 ]; then
				taskset -c 16 ./read -d $MNT_POINT/test -s $size -b $bs -c $COUNT -a
			else
				taskset -c 16 ./read -d $MNT_POINT/test -s $size -b $bs -c $COUNT
			fi
			if [ $? -ne 0 ]; then
				umount $TEST_DEV
				__fail "read test for $fs failed"
			fi
			umount $TEST_DEV
		done
	done
done

exit 0
