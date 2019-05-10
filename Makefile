all:robot_main

robot_main:
	g++ robot_main.cpp messages.cpp -std=c++11 -lzmq -o robot_main

clean:
	rm -f *.o robot_main
