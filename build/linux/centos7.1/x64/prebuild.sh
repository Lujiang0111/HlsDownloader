#!/bin/bash
function copy_dependency()
{
	local i
	local array
	
	include_dst_path=../../../../deps/include
	lib_dst_path=../../../../deps/lib
	mkdir -p ${include_dst_path}
	mkdir -p ${lib_dst_path}
	array=($(echo "$@"))
	for((i=1;i<${#array[@]};i+=4))
	do
		echo ${array[i]} ${array[i+1]} ${array[i+2]} ${array[i+3]}
		src_base_path=$1/${array[i]}/v${array[i+1]}/linux/centos7.1/x64
		include_cpy_path=${src_base_path}/include
		lib_cpy_path=${src_base_path}/lib
		#copy include
		if [ "${array[i+2]}" = "true" ]; then
			mkdir -p ${include_dst_path}/${array[i]}
			\cp -r ${include_cpy_path}/* ${include_dst_path}/${array[i]}
		fi
		#copy lib
		if [ "${array[i+3]}" = "true" ]; then
			\cp -r ${lib_cpy_path}/* ${lib_dst_path}/
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
		fi
	done
}

rm -rf ../../../../bin
mkdir -p ../../../../bin
copy_dependency $*
