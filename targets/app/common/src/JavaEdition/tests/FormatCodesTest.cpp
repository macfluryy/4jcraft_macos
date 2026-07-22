



#include <cstdio>
#include <cstdint>
#include <cstdlib>
#include <string>

#include "minecraft/util/FormatCodes.h"

namespace {


constexpr uint32_t kPalette[16] = {
    0x000000, 0x0000AA, 0x00AA00, 0x00AAAA, 0xAA0000, 0xAA00AA,
    0xFFAA00, 0xAAAAAA, 0x555555, 0x5555FF, 0x55FF55, 0x55FFFF,
    0xFF5555, 0xFF55FF, 0xFFFF55, 0xFFFFFF,
};
constexpr uint32_t kBase = 0xAAAAAA;  

int g_failures = 0;

void check(const char* what, const std::wstring& input,
           const std::wstring& expected) {
    const std::wstring got = formatCodesToHtml(input, kBase, kPalette);
    if (got != expected) {
        fprintf(stderr, "FAIL: %s\n  in:  %ls\n  got: %ls\n  want:%ls\n", what,
                input.c_str(), got.c_str(), expected.c_str());
        ++g_failures;
        return;
    }
    if (got.find(L'§') != std::wstring::npos) {
        fprintf(stderr, "FAIL: %s: literal formatting code in output\n", what);
        ++g_failures;
        return;
    }
    fprintf(stderr, "ok: %s\n", what);
}

}  

int main() {
    
    check("plain item name", L"Stone",
          L"<font color=\"#AAAAAA\">Stone</font>");

    
    check("empty line", L"", L"<font color=\"#AAAAAA\"></font>");

    
    check("colored name", L"§cEpic Sword",
          L"<font color=\"#AAAAAA\"></font>"
          L"<font color=\"#FF5555\">Epic Sword</font>");

    
    check("mid-line switch", L"§7Damage: §c+7",
          L"<font color=\"#AAAAAA\"></font>"
          L"<font color=\"#AAAAAA\">Damage: </font>"
          L"<font color=\"#FF5555\">+7</font>");

    
    check("RGB nearest", L"§x§F§F§5§5§5§5Hot",
          L"<font color=\"#AAAAAA\"></font>"
          L"<font color=\"#FF5555\">Hot</font>");

    
    check("RGB approx", L"§x§F§E§5§6§5§4Warm",
          L"<font color=\"#AAAAAA\"></font>"
          L"<font color=\"#FF5555\">Warm</font>");

    
    
    check("mixed formatting", L"§o§ait§lem§r done",
          L"<font color=\"#AAAAAA\"><i></i></font>"
          L"<font color=\"#55FF55\">item</font>"
          L"<font color=\"#AAAAAA\"> done</font>");
    check("stripped codes", L"§ka§mb§nc",
          L"<font color=\"#AAAAAA\">abc</font>");

    
    check("reset restores base", L"§4red§rgrey",
          L"<font color=\"#AAAAAA\"></font>"
          L"<font color=\"#AA0000\">red</font>"
          L"<font color=\"#AAAAAA\">grey</font>");

    
    check("italic closed at end", L"§oslanted",
          L"<font color=\"#AAAAAA\"><i>slanted</i></font>");

    
    check("dangling section", L"end§",
          L"<font color=\"#AAAAAA\">end</font>");

    
    check("malformed RGB", L"§x§F§Fok",
          L"<font color=\"#AAAAAA\"></font>"
          L"<font color=\"#FFFFFF\"></font>"
          L"<font color=\"#FFFFFF\">ok</font>");

    
    {
        std::wstring longLine(300, L'x');
        check("long lore", longLine,
              L"<font color=\"#AAAAAA\">" + longLine + L"</font>");
    }

    if (g_failures) {
        fprintf(stderr, "%d failure(s)\n", g_failures);
        return 1;
    }
    fprintf(stderr, "all format-code translation tests passed\n");
    return 0;
}
