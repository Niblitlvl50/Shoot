#!/bin/bash
./bin/game_exe.app/Contents/MacOS/game_exe --position 0 0 --size 720 450 --zone 2 &> left_game.log &
./bin/game_exe.app/Contents/MacOS/game_exe --position 720 0 --size 720 450 --zone 3 &> right_game.log &
wait
