set $dir=/mnt/ramdisk
set $nfiles=1000
set $meandirwidth=1000000
set $filesize=cvar(type=cvar-gamma,parameters=mean:16384;gamma:1.5)
set $nthreads=4
set $iosize=1m
set $meanappendsize=16k

#define fileset name=bigfileset,path=$dir,size=$filesize,entries=$nfiles,dirwidth=$meandirwidth,prealloc=80
define fileset name=bigfileset,path=$dir,size=$filesize,entries=$nfiles,dirwidth=$meandirwidth,prealloc=0

define process name=filereader,instances=1
{
  thread name=filereaderthread,memsize=10m,instances=$nthreads
  {
#    flowop deletefilesync name=deletefile1sync,filesetname=bigfileset
    flowop createfile name=createfile2,filesetname=bigfileset,fd=1
#    flowop appendfilerand name=appendfilerand2,iosize=$meanappendsize,fd=1
#    flowop fsync name=fsyncfile2,fd=1
    flowop closefile name=closefile2,fd=1
    flowop deletefilesync name=deletefile1sync,filesetname=bigfileset
#    flowop openfile name=openfile3,filesetname=bigfileset,fd=1
#    flowop readwholefile name=readfile3,fd=1,iosize=$iosize
#    flowop appendfilerand name=appendfilerand3,iosize=$meanappendsize,fd=1
#    flowop fsync name=fsyncfile3,fd=1
#    flowop closefile name=closefile3,fd=1
#    flowop openfile name=openfile4,filesetname=bigfileset,fd=1
#    flowop readwholefile name=readfile4,fd=1,iosize=$iosize
#    flowop closefile name=closefile4,fd=1
  }
}

echo  "Varmail Version 3.0 personality successfully loaded"

#run 60
run 5
