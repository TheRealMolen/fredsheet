#include "sheet.h"

#include "test_framework.h"

// --------------------------------------------------------------------------------

TEST_CASE("[sheet] testing int -> sheetIx")
{
    CHECK_EQ(IntToSheetIndex(0), "A");
    CHECK_EQ(IntToSheetIndex(1), "B");
    CHECK_EQ(IntToSheetIndex(25), "Z");
    CHECK_EQ(IntToSheetIndex(26), "AA");
    CHECK_EQ(IntToSheetIndex(27), "AB");
    CHECK_EQ(IntToSheetIndex(26 + 26), "BA");
    CHECK_EQ(IntToSheetIndex(26 + 27), "BB");
    CHECK_EQ(IntToSheetIndex((2*26) + 26), "CA");
    CHECK_EQ(IntToSheetIndex((2*26) + 27), "CB");
    CHECK_EQ(IntToSheetIndex((26*26)-1), "YZ");
    CHECK_EQ(IntToSheetIndex((26*26)), "ZA");
    CHECK_EQ(IntToSheetIndex((26*26)+25), "ZZ");
    CHECK_EQ(IntToSheetIndex((27*26)), "AAA");
    CHECK_EQ(IntToSheetIndex((27*26)+1), "AAB");
    CHECK_EQ(IntToSheetIndex((27*26)+25), "AAZ");
    CHECK_EQ(IntToSheetIndex((27*26)+26), "ABA");
    CHECK_EQ(IntToSheetIndex((27*26)+(26*2)), "ACA");
    CHECK_EQ(IntToSheetIndex((27*26)+(26*25)), "AZA");
    CHECK_EQ(IntToSheetIndex((27*26)+(26*25)+25), "AZZ");
    CHECK_EQ(IntToSheetIndex((27*26)+(26*26)), "BAA");
    CHECK_EQ(IntToSheetIndex((27*26)+(2*26*26)), "CAA");
    CHECK_EQ(IntToSheetIndex((1 + 26*26) * 26), "ZAA");
    CHECK_EQ(IntToSheetIndex((2 + 26*26) * 26), "ZBA");
    CHECK_EQ(IntToSheetIndex((27*26) * 26), "ZZA");
    CHECK_EQ(IntToSheetIndex(((27*26) * 26) + 25), "ZZZ");

    CHECK_ASSERTS(IntToSheetIndex(((27 * 26) * 26) + 26));
}

// --------------------------------------------------------------------------------

TEST_CASE("[sheet] testing sheetIx -> int")
{
    CHECK_EQ(SheetIndexToInt("A"), 0);
    CHECK_EQ(SheetIndexToInt("B"), 1);
    CHECK_EQ(SheetIndexToInt("Z"), 25);
    CHECK_EQ(SheetIndexToInt("AA"), 26);
    CHECK_EQ(SheetIndexToInt("AB"), 27);
    CHECK_EQ(SheetIndexToInt("BA"), 26 + 26);
    CHECK_EQ(SheetIndexToInt("BB"), 26 + 27);
    CHECK_EQ(SheetIndexToInt("CA"), (2*26) + 26);
    CHECK_EQ(SheetIndexToInt("CB"), (2*26) + 27);
    CHECK_EQ(SheetIndexToInt("YZ"), (26*26)-1);
    CHECK_EQ(SheetIndexToInt("ZA"), (26*26));
    CHECK_EQ(SheetIndexToInt("ZZ"), (26*26)+25);
    CHECK_EQ(SheetIndexToInt("AAA"), (27*26));
    CHECK_EQ(SheetIndexToInt("AAB"), (27*26)+1);
    CHECK_EQ(SheetIndexToInt("AAZ"), (27*26)+25);
    CHECK_EQ(SheetIndexToInt("ABA"), (27*26)+26);
}

// --------------------------------------------------------------------------------

TEST_CASE("[sheet] testing int -> sheetIx -> int")
{
    for (int ix = 0; ix < MaxSheetIndex; ++ix)
    {
        std::string s = IntToSheetIndex(ix);
        int roundTripIx = SheetIndexToInt(s);
        CHECK_EQ(ix, roundTripIx);
    }
}

// --------------------------------------------------------------------------------
