#include "fredlib.h"

#include "fredplatform.h"
#include "sheet.h"

#include "raylib.h"

#include <cstdio>
#include <cstring>
#include <print>


// --------------------------------------------------------------------------------

std::string ChooseFredFileToOpen()
{
    // TODO: non-win32 ----------------

    OPENFILENAME ofn;
    char filenameBuf[260];

    std::memset(filenameBuf, 0, sizeof(filenameBuf));
    std::memset(&ofn, 0, sizeof(ofn));

    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = reinterpret_cast<HWND>(GetWindowHandle());
    ofn.lpstrFile = filenameBuf;
    ofn.nMaxFile = sizeof(filenameBuf);
    ofn.lpstrFilter = "Fredsheet Files\0*.fred;doc.json;*.csv\0";
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    if (GetOpenFileName(&ofn) == TRUE)
    {
        // if we picked the doc.json, we actually meant to open the parent folder
        std::string filename { filenameBuf };
        std::string_view docJson { "doc.json" };
        if (filename.ends_with(docJson))
            filename.erase(end(filename) - size(docJson), end(filename));

        // strip any trailing directory
        while (!filename.empty())
        {
            char c = filename.back();
            if (c == '\\' || c == '/')
                filename.pop_back();
            else
                break;
        }

        return filename;
    }

    return {};
}

// --------------------------------------------------------------------------------

GridSheetPtr ChooseAndOpenSheet()
{
    std::string filename = ChooseFredFileToOpen();
    if (filename.empty())
        return {};

    std::print("chose file '{}' to open\n", filename);

    return {};
}

// --------------------------------------------------------------------------------

