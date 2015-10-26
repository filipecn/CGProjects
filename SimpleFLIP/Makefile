# export LD_LIBRARY_PATH=/usr/local/OpenVDB/lib
all: 
	g++ -pthread -O3 -DNDEBUG -I . -I .. -isystem /usr/include -isystem /usr/include/OpenEXR  -isystem /usr/include -DOPENVDB_USE_GLFW_3 -I . \
	-std=c++11 -I/usr/local/OpenVDB/include \
	-std=c++11 -g \
	-I/home/filipecn/ClionProjects/Aergia/src \
	-I/home/filipecn/ClionProjects/Aergia/src/GUI \
	-I/home/filipecn/ClionProjects/Aergia/src/GUI/UI \
	-I/home/filipecn/ClionProjects/Aergia/src/GUI/Graphics \
	-I/home/filipecn/ClionProjects/Aergia/src/Common \
	-I/home/filipecn/ClionProjects/Aergia/src/Graphics \
	-I/home/filipecn/ClionProjects/Aergia/src/Graphics/Rendering \
	-I/home/filipecn/ClionProjects/Aergia/src/Graphics/Helpers \
	-I/home/filipecn/ClionProjects/Aergia/src/Graphics/Scene \
	-I/home/filipecn/ClionProjects/Aergia/src/Math \
	-I/home/filipecn/ClionProjects/Aergia/src/IO \
	-I/home/filipecn/ClionProjects/Aergia/src/IO/Resources \
	-I/usr/local/include/GLFW \
	-I/usr/include/eigen3/ \
	testparticleset.cpp \
	-L/usr/local/OpenVDB/lib -l:libopenvdb.so.3.0.0 \
	-ldl -lm -lz -Wl,-rpath,/usr/lib/x86_64-linux-gnu -L/usr/lib/x86_64-linux-gnu -lHalf -Wl,-rpath,/usr/lib/x86_64-linux-gnu -L/usr/lib/x86_64-linux-gnu -ltbb -Wl,-rpath,/usr/lib/x86_64-linux-gnu -L/usr/lib/x86_64-linux-gnu -lboost_iostreams -lboost_system  -Wl,-rpath,/usr/lib/x86_64-linux-gnu -L/usr/lib/x86_64-linux-gnu -lrt -ljemalloc \
	-L/home/filipecn/ClionProjects/Aergia/build -lAergia \
	-lglfw3  -lGL -lGLU -lX11 -lXxf86vm -lXrandr -lpthread -lXi -lm -lXext -lGLEW -lXmu \
	-L/usr/local/lib -lSOIL
