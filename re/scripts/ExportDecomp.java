// Headless Ghidra script: dump decompiled C for every function, plus a
// function index and string/xref listing, into the directory given as arg 0.
import ghidra.app.script.GhidraScript;
import ghidra.app.decompiler.*;
import ghidra.program.model.listing.*;
import ghidra.program.model.symbol.*;
import ghidra.program.model.address.*;
import ghidra.program.model.data.*;
import java.io.*;
import java.util.*;

public class ExportDecomp extends GhidraScript {
    @Override
    public void run() throws Exception {
        String outDir = getScriptArgs().length > 0 ? getScriptArgs()[0] : "/tmp";
        String base = currentProgram.getName();
        DecompInterface di = new DecompInterface();
        DecompileOptions opts = new DecompileOptions();
        di.setOptions(opts);
        di.toggleCCode(true);
        di.toggleSyntaxTree(false);
        di.openProgram(currentProgram);

        FunctionManager fm = currentProgram.getFunctionManager();
        try (PrintWriter c = new PrintWriter(new FileWriter(outDir + "/" + base + ".c"));
             PrintWriter idx = new PrintWriter(new FileWriter(outDir + "/" + base + ".functions.tsv"))) {
            idx.println("entry\tname\tsize\tcallers\tcallees");
            for (Function f : fm.getFunctions(true)) {
                if (monitor.isCancelled()) break;
                Set<Function> callers = f.getCallingFunctions(monitor);
                Set<Function> callees = f.getCalledFunctions(monitor);
                idx.printf("%s\t%s\t%d\t%d\t%d%n", f.getEntryPoint(), f.getName(),
                        f.getBody().getNumAddresses(), callers.size(), callees.size());
                DecompileResults r = di.decompileFunction(f, 120, monitor);
                c.printf("// ==== %s @ %s (size %d) ====%n", f.getName(), f.getEntryPoint(), f.getBody().getNumAddresses());
                if (r != null && r.decompileCompleted()) {
                    c.println(r.getDecompiledFunction().getC());
                } else {
                    c.println("// decompile failed: " + (r == null ? "null" : r.getErrorMessage()));
                }
            }
        }
        try (PrintWriter s = new PrintWriter(new FileWriter(outDir + "/" + base + ".strings.tsv"))) {
            s.println("addr\txrefs\tvalue");
            DataIterator it = currentProgram.getListing().getDefinedData(true);
            while (it.hasNext()) {
                Data d = it.next();
                if (!(d.getDataType() instanceof StringDataType || d.getDataType() instanceof TerminatedStringDataType
                      || d.getDataType().getName().toLowerCase().contains("string"))) continue;
                StringBuilder xs = new StringBuilder();
                for (Reference ref : getReferencesTo(d.getAddress())) {
                    Function f = fm.getFunctionContaining(ref.getFromAddress());
                    xs.append(f == null ? ref.getFromAddress().toString() : f.getName()).append(",");
                }
                Object v = d.getValue();
                s.printf("%s\t%s\t%s%n", d.getAddress(), xs, v == null ? "" : v.toString().replace("\n","\\n").replace("\r","\\r").replace("\t","\\t"));
            }
        }
        di.dispose();
        println("Export done: " + outDir);
    }
}
