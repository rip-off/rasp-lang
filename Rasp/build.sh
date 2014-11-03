if [ -f rasp ]; then
	rm rasp
fi
clang++ -std=c++11 -Wall -Werror *.cpp -ferror-limit=3 -o rasp 2>&1 | head
