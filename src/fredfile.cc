
#include "fileio.h"
#include "fredlib.h"
#include "fredplatform.h"
#include "sheet.h"

#include "raylib.h"

#include <cstdio>
#include <cstring>
#include <filesystem>
#include <limits>
#include <print>


// --------------------------------------------------------------------------------

EFredFileType IdentifyFileType(const std::string& filename)
{
    if (filename.ends_with("doc.json"))
        return EFredFileType::DocumentMeta;

    if (filename.ends_with(".csv"))
        return EFredFileType::TableCsv;

    return EFredFileType::Invalid;
}

// --------------------------------------------------------------------------------

FredFileEntry::FredFileEntry(std::string&& filename, std::string&& contents)
    : Filename(filename)
    , Contents(contents)
    , FileType(IdentifyFileType(filename))
{
    /**/
}

// --------------------------------------------------------------------------------
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

FredDocFilePtr ReadGridFileFromDir_Sync(const std::string& fredFilename)
{
    namespace fs = std::filesystem;
    if (!fs::is_directory(fredFilename))
        throw std::invalid_argument(fredFilename);

    FredDocFilePtr docFile = std::make_unique<FredDocFile>(fredFilename);

    for (const auto& dirEntry : fs::directory_iterator { fredFilename })
    {
        std::string filename = dirEntry.path().string();
        std::string contents = SlurpFile(filename);
        docFile->Subfiles.emplace_back(std::move(filename), std::move(contents));
    }

    return docFile;
}

// --------------------------------------------------------------------------------

GridSheet::StringGrid ParseCsv(const std::string_view& raw)
{
    GridSheet::StringGrid rows;

    GridSheet::StringList currRow;
    std::string currCell;

    auto finishCell = [&]()
    {
        currRow.emplace_back(std::move(currCell));
        currCell = {};
    };
    auto finishRow = [&]()
    {
        finishCell();
        
        rows.emplace_back(std::move(currRow));
        currRow = {};
    };

    for (auto itC = begin(raw); itC != end(raw); ++itC)
    {
        char c = *itC;

        if (c == ',')
        {
            finishCell();
        }
        else if (c == '\n')
        {
            finishRow();
            currRow.reserve(size(rows.back()));
        }
        else if (c == '\r')
        {
            continue;
        }
        else if (c == '"')
        {
            bool finished = false;
            for (++itC; itC != end(raw); ++itC)
            {
                c = *itC;
                if (c == '\\')
                {
                    ++itC;
                    if (itC == end(raw))
                        throw std::invalid_argument(std::format("unfinished escaped string near '{}'", currCell));
                    
                    char escaped = *itC;
                    if (escaped == 'n')
                        escaped = '\n';
                    else if (escaped == 'r')
                        escaped = '\r';
                    else if (escaped == 't')
                        escaped = '\t';

                    currCell.push_back(escaped);
                }
                else if (c == '"')
                {
                    // sometimes " is escaped as "", sometimes it's \"
                    auto itNextC = itC + 1;
                    if ((itNextC != end(raw)) && (*itNextC == '"'))
                    {
                        currCell.push_back('"');
                        itC = itNextC;
                    }
                    else
                    {
                        finished = true;
                        break;
                    }
                }
                else
                {
                    currCell.push_back(c);
                }
            }
            if (!finished)
                throw std::invalid_argument(std::format("unfinished quoted string near '{}'", currCell));

            // TODO: maybe check for malformed snippets like
            // "banana"banana,
        }
        else
        {
            currCell.push_back(c);
        }
    }
    if (!currCell.empty() || !currRow.empty())
    {
        finishRow();
    }

    return rows;
}

// --------------------------------------------------------------------------------

FredDocPtr OpenFredDoc(const std::string& filename)
{
    if (filename.empty())
        throw std::invalid_argument("no fredfile specified");

    std::println("opening fredfile '{}'", filename);
    fflush(stdout);

    FredDocPtr docP = std::make_shared<FredDoc>();
    docP->SourceData = ReadGridFileFromDir_Sync(filename);

    for (const FredFileEntry& entry : docP->SourceData->Subfiles)
    {
        if (entry.FileType == EFredFileType::TableCsv)
        {
            GridSheet::StringGrid parsed = ParseCsv(entry.Contents);
        }
    }

    return docP;
}

// --------------------------------------------------------------------------------
