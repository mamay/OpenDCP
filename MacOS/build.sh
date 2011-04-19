#!/bin/sh

cdir=`pwd`
cd ../
dir=`pwd`
cd MacOS

echo "compiling OSX10.5 386.."
bld_dir=$dir/build
if [ ! -d $bld_dir ]
then
    echo "Creating build directory $bld_dir"
    mkdir -p $bld_dir
fi 
cd $bld_dir 
cmake -DENABLE_STATIC=ON -DENABLE_XMLSEC=ON -DENABLE_OSX386=ON -DENABLE_OPENMP=ON ../
make
cd $dir

echo "compiling OSX10.6 x86_64..."
bld64_dir=$dir/build64
if [ ! -d $bld64_dir ]
then
    echo "Creating build directory $bld64_dir"
    mkdir -p $bld64_dir
fi
cd $bld64_dir
cmake -DENABLE_STATIC=ON -DENABLE_XMLSEC=ON -DENABLE_OPENMP=ON ../
make
cd $dir

echo "creating universal binaries..."
lipo -create $bld_dir/lib/libopendcp.a $bld64_dir/lib/libopendcp.a -output $cdir/libopendcp.a
lipo -create $bld_dir/bin/opendcp_j2k $bld64_dir/bin/opendcp_j2k -output $cdir/opendcp_j2k 
lipo -create $bld_dir/bin/opendcp_mxf $bld64_dir/bin/opendcp_mxf -output $cdir/opendcp_mxf 
lipo -create $bld_dir/bin/opendcp_xml $bld64_dir/bin/opendcp_xml -output $cdir/opendcp_xml
lipo -create $bld_dir/bin/opendcp_xml_verify $bld64_dir/bin/opendcp_xml_verify -output $cdir/opendcp_xml_verify 
lipo -create $bld_dir/bin/opendcp_largefile $bld64_dir/bin/opendcp_largefile -output $cdir/opendcp_largefile

mv $cdir/libopendcp.a $bld_dir/lib/libopendcp.a
mv $cdir/opendcp_j2k  $bld_dir/bin/opendcp_j2k
mv $cdir/opendcp_mxf  $bld_dir/bin/opendcp_mxf
mv $cdir/opendcp_xml  $bld_dir/bin/opendcp_xml
mv $cdir/opendcp_xml_verify $bld_dir/bin/opendcp_xml_verify
mv $cdir/opendcp_largefile $bld_dir/bin/opendcp_largefile

cd $bld_dir
sudo make install
