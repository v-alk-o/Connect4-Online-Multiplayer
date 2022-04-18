#!/bin/bash

BIN_PATH="$(dirname $(realpath $0))/../bin";
gnome-terminal --geometry=80x25+808+0 -- $BIN_PATH/server 127.0.0.1 4444 && sleep 1;
gnome-terminal --geometry=80x25+0+525   -e "bash -c \"$BIN_PATH/client 127.0.0.1 4444; read line;\"" && sleep 1;
gnome-terminal --geometry=80x25+808+525 -e "bash -c \"$BIN_PATH/client 127.0.0.1 4444; read line;\"";
