cmake_minimum_required(VERSION 3.14.4)
project(mhash)

set (CMAKE_C_STANDARD 11)

set(mhash_lib_sources
	lib/adler32.c
	lib/crc32.c
	lib/gosthash.c
	lib/haval.c
	lib/keygen_asis.c
	lib/keygen.c
	lib/keygen.h
	lib/keygen_hex.c
	lib/keygen_mcrypt.c
	lib/keygen_s2k.c
	lib/libdefs.h
	lib/md2.c
	lib/md4.c
	lib/md5.c
	lib/mhash_adler32.h
	lib/mhash.c
	lib/mhash_crc32.h
	lib/mhash_gost.h
	lib/mhash_haval.h
	lib/mhash_int.h
	lib/mhash_md2.h
	lib/mhash_md4.h
	lib/mhash_md5.h
	lib/mhash_ripemd.h
	lib/mhash_sha1.h
	lib/mhash_sha256.h
	lib/mhash_sha256_sha224.h
	lib/mhash_sha512_sha384.h
	lib/mhash_snefru.h
	lib/mhash_tiger.h
	lib/mhash_whirlpool.h
	lib/ripemd.c
	lib/sha1.c
	lib/sha256.c
	lib/sha256_sha224.c
	lib/sha512_sha384.c
	lib/snefru.c
	lib/stdfns.c
	lib/tiger.c
	lib/tiger_sboxes.c
	lib/whirlpool.c
	include/mutils/mglobal.h
	include/mutils/mhash.h
	include/mutils/mincludes.h
	include/mutils/mtypes.h
	include/mutils/mutils.h)

add_library(mhash ${mhash_lib_sources})

if(WIN32)
	set(mhash_config_source include/mutils/mhash_config.win32.h)
elseif(APPLE)
	set(mhash_config_source include/mutils/mhash_config.osx.h)
elseif(UNIX)
	set(mhash_config_source include/mutils/mhash_config.linux.h)
endif()

configure_file(${mhash_config_source} mutils/mhash_config.h COPYONLY)
target_sources(mhash PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/mutils/mhash_config.h)
target_include_directories(
	mhash
	PUBLIC
		${CMAKE_CURRENT_BINARY_DIR}
		include
	PRIVATE
		lib
)
