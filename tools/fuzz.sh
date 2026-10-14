#!/bin/sh
# Random-input stress test of module $1 with the sanitizer build.
# usage: tools/fuzz.sh MODULE [COUNT] [INTERVAL_MS] [SEED]
M=$1; N=${2:-300}; I=${3:-150}; SEED=${4:-1}
OUT=/tmp/js4fuzz/m$M
rm -rf $OUT; mkdir -p $OUT/save
cat > $OUT/script.txt <<EOS
wait 1200
click 320 240
wait 600
click 320 240
wait 600
click 320 240
wait 600
click 320 240
wait 2500
type Fuzzer
char 13
wait 5000
snap start
fuzz $N $I
snap end
exit
EOS
cd "$(dirname "$0")/.."
JS4_QUIET=1 JS4_FUZZ_SEED=$SEED JS4_SAVE_DIR=$OUT/save JS4_JUMP=$M JS4_SCRIPT=$OUT/script.txt JS4_SNAP_DIR=$OUT \
  SDL_AUDIO_DRIVER=dummy ASAN_OPTIONS=detect_leaks=0 UBSAN_OPTIONS=print_stacktrace=1 \
  perl -e 'alarm 600; exec @ARGV' "./build-asan/JumpStart 4th Grade.app/Contents/MacOS/JumpStart 4th Grade" "$PWD/game/win" \
  > $OUT/log.txt 2>&1
echo "module $M exit=$? $(grep -c 'runtime error' $OUT/log.txt) ubsan, $(grep -c 'ERROR: AddressSanitizer' $OUT/log.txt) asan, $(grep -c 'fatal\|unported\|segment access\|unported code' $OUT/log.txt) fatal"
