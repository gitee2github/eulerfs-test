. ./../config
. ./../comm

FS=(ext4 ext4dax eulerfs)
SIZE=(65536)
BS=(4096 1048576) #4k 64k 1MB
RW=(read write randread randwrite)
NJ=(1 2 4 8 16)
FSYNC=(1)
DEVIDE=0

usage()
{
	cat <<-EOF
	Usage: $0 [-f fs|-s size|-b bs|-t rw|-n numjobs|-y fsync|-d]

	-f fs: test filesystem, default ${FS[@]}
	-s size: total file size in MB, default ${SIZE[@]}
	-b bs: read / write size, default ${BS[@]}
	-t rw: io type, default ${RW[@]}
	-n num: numjobs, default ${NJ[@]}
	-y fsync: fio fsync=?, default ${FSYNC[@]}
	-d devide: devide size for each thread, default not set
EOF
}

__fail()
{
	echo $*
	exit 1
}

while getopts f:s:b:t:n:y:d opt
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
	t)
		RW=("$OPTARG")
		;;
	n)
		NJ=("$OPTARG")
		;;	
	y)
		FSYNC=("$OPTARG")	
		;;
	d)
		DEVIDE=1
		;;
	h|?)
		usage
		exit 1
		;;
	esac
done

echo "start fio file rw test on $TEST_DEV"
echo "############################"
echo "test fs:          ${FS[@]}"
echo "test file size:   ${SIZE[@]}MB"
echo "test bs:          ${BS[@]}"
echo "test type:        ${RW[@]}"
echo "test numjobs:     ${NJ[@]}"
echo "test fsync:       ${FSYNC[@]}"
echo "############################"

# $1 bs
# $2 numjobs
# $3 size
# $4 rw
# $5 fsync
__fio()
{
	fio \
	-directory=$MNT_POINT/ \
	-name=test \
	-ioengine=psync \
	-bs=$1 \
	-numjobs=$2 \
	-size=${3}MB \
	-rw=$4 \
	-fsync=$5 \
	-cpus_allowed=0-63 \
	-cpus_allowed_policy=split \
	-time_based \
	-ramp_time=5 \
	-runtime=15 \
	-group_reporting \
	-fallocate=none
}

for rw in ${RW[@]}; do
for fsync in ${FSYNC[@]}; do
for size in ${SIZE[@]}; do
for bs in ${BS[@]}; do
for n in ${NJ[@]}; do
for fs in ${FS[@]}; do

	__mount $fs || __fail "mount $fs failed"

	if [ $DEVIDE -ne 0 ]; then
	((size = size / n))
	fi
	echo "----- rw $rw, fs $fs, size $s, bs $bs, numjons $n, fsync $fsync-----"
	__fio $bs $n $size $rw $fsync
	if [ $? -ne 0 ]; then
		umount $TEST_DEV
		__fail "test for $fs failed"
	fi
	umount $TEST_DEV

done
done
done
done
done
done

exit 0
