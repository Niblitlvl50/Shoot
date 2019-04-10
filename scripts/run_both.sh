#!/bin/bash
./bin/game --position 0 0 --size 720 450 --zone 2 &> left_game.log &
./bin/game --position 720 0 --size 720 450 --zone 3 &> right_game.log &
wait
