#!/bin/bash
function copy_dependency()
{
	local i
	local array
	
	lib_dst_path=../../../../bin/lib
	mkdir -p ${lib_dst_path}
	array=($(echo "$@"))
	for((i=1;i<${#array[@]};i+=2))
	do
		src_base_path=$1/${array[i]}/v${array[i+1]}/linux/centos7.1/x64
		include_cpy_path=${src_base_path}/include
		lib_cpy_path=${src_base_path}/lib
		\cp -rf ${lib_cpy_path}/* ${lib_dst_path}/
		for file in ${lib_cpy_path}/*.so.*
		do
			if [ "${file}" = "${lib_cpy_path}/*.so.*" ]; then
				:
			else
				realname=`echo ${file} | rev |cut -d '/' -f 1 | rev`
				libname=`echo ${realname} |cut -d '.' -f 1`
				ln -sf ${realname} ${lib_dst_path}/${libname}.so
			fi
		done
	done
}

rm -rf ../../../../bin/lib
mkdir -p ../../../../bin/lib
copy_dependency $*

\cp -rf run.sh ../../../../bin
\cp -rf debug.sh ../../../../bin