set os_version=windows\vs2017\x64\
set dst_base=..\..\..\..\deps\
set out_base=..\..\..\..\bin\

rmdir /Q /S %dst_base%include
rmdir /Q /S %dst_base%lib

mkdir %out_base%
mkdir %dst_base%include
mkdir %dst_base%lib

::Baselib
set src_base=..\..\..\..\..\..\..\..\..\Versions\Baselib\

::libcurl
mkdir %dst_base%include\libcurl
xcopy %src_base%libcurl\v7.74.0\%os_version%include %dst_base%include\libcurl /S /Y /C
xcopy %src_base%libcurl\v7.74.0\%os_version%lib %dst_base%lib /S /Y /C

::PlatformBase
mkdir %dst_base%include\PlatformBase
xcopy %src_base%PlatformBase\v1.0.0\%os_version%include %dst_base%include\PlatformBase /S /Y /C
xcopy %src_base%PlatformBase\v1.0.0\%os_version%lib %dst_base%lib /S /Y /C