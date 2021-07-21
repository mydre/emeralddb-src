AUTOMAKE_OPTIONS = foreign subdir-objects
bin_PROGRAMS = emeralddb edb#项目名字
emeralddb_SOURCES =\
	pmd/pmdMain.cpp pmd/pmdTcpListener.cpp pmd/pmdOptions.cpp \
	pmd/pmd.cpp pmd/pmdEDU.cpp pmd/pmdEDUMgr.cpp pmd/pmdAgent.cpp \
	bson/src/bsonobj.cpp bson/src/util/json.cpp bson/src/oid.cpp \
	bson/lib/nonce.cpp bson/lib/md5.cpp bson/lib/base64.cpp \
	oss/ossSocket.cpp oss/ossPrimitiveFileOp.cpp oss/ossMmapFile.cpp		\
	pd/pd.cpp msg/msg.cpp						\
	dms/dms.cpp	\
	rtn/rtn.cpp

edb_SOURCES =\
	client/edb.cpp client/command.cpp client/commandFactory.cpp \
	bson/src/bsonobj.cpp bson/src/util/json.cpp bson/src/oid.cpp \
	bson/lib/nonce.cpp bson/lib/md5.cpp bson/lib/base64.cpp \
	oss/ossSocket.cpp pd/pd.cpp oss/ossPrimitiveFileOp.cpp \
	msg/msg.cpp

emeralddb_CXXFLAGS=-I../boost -Ibson -Ibson/src -Ibson/lib -Iinclude -D_FILE_OFFSET_BITS=64 -ggdb -Wall -O0
emeralddb_LDADD=-lpthread -lm -lboost_system -lboost_thread -lboost_program_options -lrt
emeralddb_LDFLAGS=-fPIC -rdynamic -L../boost/stage/lib -pthread

edb_CXXFLAGS=-I../boost -Ibson -Ibson/src -Ibson/lib -Iinclude -D_FILE_OFFSET_BITS=64 -ggdb -Wall -O0
edb_LDADD=-lm -lboost_system -lboost_thread -lrt
edb_LDFLAGS=-fPIC -rdynamic -L../boost/stage/lib