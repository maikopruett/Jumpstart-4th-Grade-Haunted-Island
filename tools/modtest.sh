#!/bin/sh
# Headless smoke test of one module: signs in as "Tester", jumps to module $1
# and saves snapshots to /tmp/js4snap/<module>_N.png. Extra script lines may
# be passed on stdin (appended after the jump).
M=$1
OUT=/tmp/js4snap
mkdir -p $OUT
rm -f $OUT/m${M}_*.png
SCRIPT=$OUT/mod_$M.txt
cat > $SCRIPT <<EOS
wait 1500
click 320 240
wait 700
click 320 240
wait 700
click 320 240
wait 700
click 320 240
wait 2500
type Tester
char 13
wait 6000
snap m${M}_1
EOS
cat >> $SCRIPT
echo "exit" >> $SCRIPT
cd "$(dirname "$0")/.."
JS4_JUMP=$M JS4_SCRIPT=$SCRIPT JS4_SNAP_DIR=$OUT SDL_AUDIO_DRIVER=dummy perl -e 'alarm 120; exec @ARGV' \
  "./build/JumpStart 4th Grade.app/Contents/MacOS/JumpStart 4th Grade" "$PWD/game/win" 2>&1 | grep -v "^\[script\] \(click\|snap\|wait\)\|game data"
