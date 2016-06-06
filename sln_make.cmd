mkdir msvc12

cd msvc12

cmake -G "Visual Studio 12" -Wno-dev -DWITH_EXAMPLES=1 ^
	-DBOOST_INCLUDEDIR="C:\SDK\boost\Boost_1.57\include\boost-1_57" ^
	-DBOOST_LIBRARYDIR="C:\SDK\boost\Boost_1.57\lib" ^
	-DPROTOBUF_INCLUDE_DIR="C:\SDK\ProtoBuf\ProtoBuf_2.4.1\include" ^
	-DPROTOBUF_LIBRARY_DEBUG="C:\SDK\ProtoBuf\ProtoBuf_2.4.1\lib\libprotobuf-vc120-mt-sgd.lib" ^
	-DPROTOBUF_LIBRARY="C:\SDK\ProtoBuf\ProtoBuf_2.4.1\lib\libprotobuf-vc120-mt-s.lib" ^
	-DPROTOBUF_PROTOC_EXECUTABLE="C:\SDK\ProtoBuf\ProtoBuf_2.4.1\bin\protoc.exe" ^
	-DOPENSSL_ROOT_DIR="C:\SDK\OpenSSL_\1.0.2g\static\win32" ^
	-DLUA_SRC="C:/SDK/lua/src" ^
	 ../

cd ../


