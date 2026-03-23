#include "EditorNS/lexerfactory.h"

#include "EditorNS/nulllexer.h"

#include <Qsci/qscilexeravs.h>
#include <Qsci/qscilexerbash.h>
#include <Qsci/qscilexerbatch.h>
#include <Qsci/qscilexercmake.h>
#include <Qsci/qscilexercoffeescript.h>
#include <Qsci/qscilexercpp.h>
#include <Qsci/qscilexercsharp.h>
#include <Qsci/qscilexercss.h>
#include <Qsci/qscilexerd.h>
#include <Qsci/qscilexerdiff.h>
#include <Qsci/qscilexeredifact.h>
#include <Qsci/qscilexerfortran.h>
#include <Qsci/qscilexerfortran77.h>
#include <Qsci/qscilexerhtml.h>
#include <Qsci/qscilexeridl.h>
#include <Qsci/qscilexerjava.h>
#include <Qsci/qscilexerjavascript.h>
#include <Qsci/qscilexerjson.h>
#include <Qsci/qscilexerlua.h>
#include <Qsci/qscilexermakefile.h>
#include <Qsci/qscilexermarkdown.h>
#include <Qsci/qscilexermatlab.h>
#include <Qsci/qscilexeroctave.h>
#include <Qsci/qscilexerpascal.h>
#include <Qsci/qscilexerperl.h>
#include <Qsci/qscilexerpo.h>
#include <Qsci/qscilexerpostscript.h>
#include <Qsci/qscilexerpov.h>
#include <Qsci/qscilexerproperties.h>
#include <Qsci/qscilexerpython.h>
#include <Qsci/qscilexerruby.h>
#include <Qsci/qscilexerspice.h>
#include <Qsci/qscilexersql.h>
#include <Qsci/qscilexertcl.h>
#include <Qsci/qscilexertex.h>
#include <Qsci/qscilexerverilog.h>
#include <Qsci/qscilexervhdl.h>
#include <Qsci/qscilexerxml.h>
#include <Qsci/qscilexeryaml.h>

#if QSCINTILLA_VERSION >= QT_VERSION_CHECK(2, 14, 0)
#include <Qsci/qscilexerasm.h>
#include <Qsci/qscilexermasm.h>
#include <Qsci/qscilexernasm.h>
#include <Qsci/qscilexerhex.h>
#include <Qsci/qscilexerintelhex.h>
#include <Qsci/qscilexertekhex.h>
#include <Qsci/qscilexersrec.h>
#endif

namespace EditorNS
{

namespace LexerFactory
{

QsciLexer* CreateLexerForId(QObject *parent, const QString &id)
{
    QsciLexer *lexer =  (id == "avs")         ? new QsciLexerAVS(parent) :          // FIXME: Add to Languages.json
                        (id == "bash")        ? new QsciLexerBash(parent) :
                        (id == "batch")       ? new QsciLexerBatch(parent) :        // FIXME: Add to Languages.json
                        (id == "cmake")       ? new QsciLexerCMake(parent) :
                        (id == "coffescript") ? new QsciLexerCoffeeScript(parent) :
                        (id == "c")           ? new QsciLexerCPP(parent) :    // Use C++ lexer for C
                        (id == "cpp")         ? new QsciLexerCPP(parent) :
                        (id == "csharp")      ? new QsciLexerCSharp(parent) :
                        (id == "css")         ? new QsciLexerCSS(parent) :
                        (id == "d")           ? new QsciLexerD(parent) :
                        (id == "diff")        ? new QsciLexerDiff(parent) :
                        (id == "edifact")     ? new QsciLexerEDIFACT(parent) :      // FIXME: Add to Languages.json
                        (id == "fortran")     ? new QsciLexerFortran(parent) :
                        (id == "fortran77")   ? new QsciLexerFortran77(parent) :    // FIXME: Add to Languages.json
                        (id == "html")        ? new QsciLexerHTML(parent) :
                        (id == "idl")         ? new QsciLexerIDL(parent) :
                        (id == "java")        ? new QsciLexerJava(parent) :
                        (id == "javascript")   ? new QsciLexerJavaScript(parent) :
                        (id == "json")        ? new QsciLexerJSON(parent) :
                        (id == "latex")         ? new QsciLexerTeX(parent) :
                        (id == "lua")         ? new QsciLexerLua(parent) :
                        (id == "makefile")    ? new QsciLexerMakefile(parent) :
                        (id == "markdown")    ? new QsciLexerMarkdown(parent) :
                        (id == "matlab")      ? new QsciLexerMatlab(parent) :       // FIXME: Add to Languages.json
                        (id == "octave")      ? new QsciLexerOctave(parent) :
                        (id == "pascal")      ? new QsciLexerPascal(parent) :
                        (id == "perl")        ? new QsciLexerPerl(parent) :
                        (id == "po")          ? new QsciLexerPO(parent) :           // FIXME: Add to Languages.json
                        (id == "postscript")  ? new QsciLexerPostScript(parent) :   // FIXME: Add to Languages.json
                        (id == "pov")         ? new QsciLexerPOV(parent) :          // FIXME: Add to Languages.json
                        (id == "properties")  ? new QsciLexerProperties(parent) :
                        (id == "python")      ? new QsciLexerPython(parent) :
                        (id == "ruby")        ? new QsciLexerRuby(parent) :
                        (id == "spice")       ? new QsciLexerSpice(parent) :        // FIXME: Add to Languages.json
                        (id == "sql")         ? new QsciLexerSQL(parent) :

                        (id == "tcl")         ? new QsciLexerTCL(parent) :
                        (id == "typescipt")   ? new QsciLexerJavaScript(parent) :   // use JavaScript lexer for TypeScript
                        (id == "verilog")     ? new QsciLexerVerilog(parent) :
                        (id == "vhdl")        ? new QsciLexerVHDL(parent) :
                        (id == "xml")         ? new QsciLexerXML(parent) :
                        (id == "yaml")        ? new QsciLexerYAML(parent) :
#if QSCINTILLA_VERSION >= QT_VERSION_CHECK(2, 14, 0)
                        (id == "masm")        ? new QsciLexerMASM(parent) :         // FIXME: Add to Languages.json
                        (id == "nasm")        ? new QsciLexerNASM(parent) :         // FIXME: Add to Languages.json
                        (id == "intelhex")    ? new QsciLexerIntelHex(parent) :     // FIXME: Add to Languages.json
                        (id == "tekhex")      ? new QsciLexerTekHex(parent) :       // FIXME: Add to Languages.json
                        (id == "srec")        ? new QsciLexerSRec(parent) :         // FIXME: Add to Languages.json
#endif
                                                (QsciLexer*)new NullLexer(parent);

    return lexer;
}

} // namespace LexerFactory

} // namespace EditorNS

