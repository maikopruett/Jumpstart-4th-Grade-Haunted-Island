// Apply names from re/names.tsv (addr<TAB>name[<TAB>prototype]).
// Code addresses become functions (created if Ghidra missed them); a C
// prototype in the third column is parsed and applied to the function.
// Data addresses become labels.
import ghidra.app.script.GhidraScript;
import ghidra.app.cmd.function.ApplyFunctionSignatureCmd;
import ghidra.app.util.parser.FunctionSignatureParser;
import ghidra.program.model.address.Address;
import ghidra.program.model.data.FunctionDefinitionDataType;
import ghidra.program.model.listing.*;
import ghidra.program.model.symbol.*;
import java.nio.file.*;

public class ApplyNames extends GhidraScript {
    @Override
    public void run() throws Exception {
        String path = getScriptArgs().length > 0 ? getScriptArgs()[0] : "re/names.tsv";
        int n = 0, created = 0, sigs = 0;
        FunctionSignatureParser parser = new FunctionSignatureParser(currentProgram.getDataTypeManager(), null);
        for (String line : Files.readAllLines(Paths.get(path))) {
            line = line.trim();
            if (line.isEmpty() || line.startsWith("#")) continue;
            String[] f = line.split("\t");
            if (f.length < 2) continue;
            Address a = toAddr(Long.parseLong(f[0], 16));
            String name = f[1];
            var block = currentProgram.getMemory().getBlock(a);
            boolean isCode = block != null && block.getName().equals("CODE");
            if (isCode) {
                Function fn = getFunctionAt(a);
                if (fn == null) {
                    disassemble(a);
                    fn = createFunction(a, name);
                    created++;
                }
                if (fn == null) continue;
                fn.setName(name, SourceType.USER_DEFINED);
                if (f.length > 2 && !f[2].isEmpty()) {
                    try {
                        FunctionDefinitionDataType sig = parser.parse(fn.getSignature(), f[2]);
                        new ApplyFunctionSignatureCmd(a, sig, SourceType.USER_DEFINED).applyTo(currentProgram);
                        sigs++;
                    } catch (Exception e) {
                        println("bad prototype for " + name + ": " + e.getMessage());
                    }
                }
            } else {
                createLabel(a, name, true, SourceType.USER_DEFINED);
            }
            n++;
        }
        println("ApplyNames: " + n + " names, " + created + " functions created, " + sigs + " prototypes");
    }
}
