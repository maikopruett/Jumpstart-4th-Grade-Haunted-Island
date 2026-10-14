#!/bin/sh
# Re-run the Ghidra headless analysis of 4G.EXE and export decompiled C to re/out/.
# Requires Ghidra 12 (GHIDRA_HOME) and JDK 21. The binary must use the Borland
# C++ cspec -- Ghidra auto-detects Delphi, which hides all cdecl stack params.
# Names from re/names.tsv are applied before export.
set -e
cd "$(dirname "$0")/.."
GHIDRA_HOME=${GHIDRA_HOME:-$HOME/ghidra/ghidra_12.1.3_PUBLIC}
export JAVA_HOME=${JAVA_HOME:-/opt/homebrew/opt/openjdk@21/libexec/openjdk.jdk/Contents/Home}
mkdir -p re/proj re/out
touch re/names.tsv
"$GHIDRA_HOME/support/analyzeHeadless" re/proj JS4G \
  -import game/win/SUPPORT/CMN/4G/4G.EXE -overwrite \
  -processor x86:LE:32:default -cspec borlandcpp \
  -scriptPath re/scripts -preScript EnableParamId.java \
  -postScript ApplyNames.java "$PWD/re/names.tsv" \
  -postScript ExportDecomp.java "$PWD/re/out" > re/out/headless.log 2>&1
grep -E "ApplyNames|Export done|ERROR" re/out/headless.log || true
