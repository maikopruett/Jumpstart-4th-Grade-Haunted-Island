// Pre-analysis script: turn on analyzers that recover cdecl parameters and
// find functions only reachable through pointers (callbacks, WinMain).
import ghidra.app.script.GhidraScript;
public class EnableParamId extends GhidraScript {
    @Override
    public void run() throws Exception {
        setAnalysisOption(currentProgram, "Decompiler Parameter ID", "true");
        setAnalysisOption(currentProgram, "Aggressive Instruction Finder", "true");
        println("Decompiler Parameter ID + Aggressive Instruction Finder enabled");
    }
}
