# export LD_LIBRARY_PATH=/usr/local/OpenVDB/lib

all: 
	g++ -pthread -O3 -DNDEBUG -I . -I .. -isystem /usr/include -isystem /usr/include/OpenEXR  -isystem /usr/include -DOPENVDB_USE_GLFW_3 -o openvdb openvdbnearest.cpp -I . \
		-std=c++11 -I/usr/local/OpenVDB/include -L/usr/local/OpenVDB/lib -l:libopenvdb.so.3.0.0 \
	    -ldl -lm -lz -Wl,-rpath,/usr/lib/x86_64-linux-gnu -L/usr/lib/x86_64-linux-gnu -lHalf -Wl,-rpath,/usr/lib/x86_64-linux-gnu -L/usr/lib/x86_64-linux-gnu -ltbb -Wl,-rpath,/usr/lib/x86_64-linux-gnu -L/usr/lib/x86_64-linux-gnu -lboost_iostreams -lboost_system  -Wl,-rpath,/usr/lib/x86_64-linux-gnu -L/usr/lib/x86_64-linux-gnu -lrt -ljemalloc
