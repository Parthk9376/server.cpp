
#!/usr/bin/env bash
set -e

g++ -std=c++17 -O2 -pthread server.cpp -o mallseat

echo "MallSeat Finder compiled successfully."
