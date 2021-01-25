#!/bin/bash
SHELL_FOLDER=$(cd "$(dirname "${0}")";pwd)/
LIBRARY=${SHELL_FOLDER}lib/

ulimit -n 65536
ldconfig -n ${LIBRARY}
export LD_LIBRARY_PATH=${LIBRARY}

cd ${LIBRARY}
for file in *.so.*
do
	if [[ ${file}"x" != "x" ]]; then
		realname=`echo ${file} | rev |cut -d '/' -f 1 | rev`
		libname=`echo ${realname} |cut -d '.' -f 1`
		if [ ! -f ${libname}.so ]; then
			ln -sf ${realname} ${libname}.so
		fi
	fi
done

cd ${SHELL_FOLDER}
chmod +x HlsDownloader
gdb ./HlsDownloader
