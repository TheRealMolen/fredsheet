#include "sheet.h"

#include "test_framework.h"

// --------------------------------------------------------------------------------

TEST_CASE("[sheet] testing int -> columnName")
{
    CHECK_EQ(IntToColumnName(0), "A");
    CHECK_EQ(IntToColumnName(1), "B");
    CHECK_EQ(IntToColumnName(25), "Z");
    CHECK_EQ(IntToColumnName(26), "AA");
    CHECK_EQ(IntToColumnName(27), "AB");
    CHECK_EQ(IntToColumnName(26 + 26), "BA");
    CHECK_EQ(IntToColumnName(26 + 27), "BB");
    CHECK_EQ(IntToColumnName((2*26) + 26), "CA");
    CHECK_EQ(IntToColumnName((2*26) + 27), "CB");
    CHECK_EQ(IntToColumnName((26*26)-1), "YZ");
    CHECK_EQ(IntToColumnName((26*26)), "ZA");
    CHECK_EQ(IntToColumnName((26*26)+25), "ZZ");
    CHECK_EQ(IntToColumnName((27*26)), "AAA");
    CHECK_EQ(IntToColumnName((27*26)+1), "AAB");
    CHECK_EQ(IntToColumnName((27*26)+25), "AAZ");
    CHECK_EQ(IntToColumnName((27*26)+26), "ABA");
    CHECK_EQ(IntToColumnName((27*26)+(26*2)), "ACA");
    CHECK_EQ(IntToColumnName((27*26)+(26*25)), "AZA");
    CHECK_EQ(IntToColumnName((27*26)+(26*25)+25), "AZZ");
    CHECK_EQ(IntToColumnName((27*26)+(26*26)), "BAA");
    CHECK_EQ(IntToColumnName((27*26)+(2*26*26)), "CAA");
    CHECK_EQ(IntToColumnName((1 + 26*26) * 26), "ZAA");
    CHECK_EQ(IntToColumnName((2 + 26*26) * 26), "ZBA");
    CHECK_EQ(IntToColumnName((27*26) * 26), "ZZA");
    CHECK_EQ(IntToColumnName(((27*26) * 26) + 25), "ZZZ");

    CHECK_ASSERTS(IntToColumnName(((27 * 26) * 26) + 26));
}

// --------------------------------------------------------------------------------

TEST_CASE("[sheet] testing columnName -> int")
{
    CHECK_EQ(ColumnNameToInt("A"), 0);
    CHECK_EQ(ColumnNameToInt("B"), 1);
    CHECK_EQ(ColumnNameToInt("Z"), 25);
    CHECK_EQ(ColumnNameToInt("AA"), 26);
    CHECK_EQ(ColumnNameToInt("AB"), 27);
    CHECK_EQ(ColumnNameToInt("BA"), 26 + 26);
    CHECK_EQ(ColumnNameToInt("BB"), 26 + 27);
    CHECK_EQ(ColumnNameToInt("CA"), (2*26) + 26);
    CHECK_EQ(ColumnNameToInt("CB"), (2*26) + 27);
    CHECK_EQ(ColumnNameToInt("YZ"), (26*26)-1);
    CHECK_EQ(ColumnNameToInt("ZA"), (26*26));
    CHECK_EQ(ColumnNameToInt("ZZ"), (26*26)+25);
    CHECK_EQ(ColumnNameToInt("AAA"), (27*26));
    CHECK_EQ(ColumnNameToInt("AAB"), (27*26)+1);
    CHECK_EQ(ColumnNameToInt("AAZ"), (27*26)+25);
    CHECK_EQ(ColumnNameToInt("ABA"), (27*26)+26);
}

// --------------------------------------------------------------------------------

TEST_CASE("[sheet] testing int -> columnName -> int")
{
    for (int ix = 0; ix < MaxSheetIndex; ++ix)
    {
        std::string s = IntToColumnName(ix);
        int roundTripIx = ColumnNameToInt(s);
        CHECK_EQ(ix, roundTripIx);
    }
}

// --------------------------------------------------------------------------------

TEST_CASE("[sheet] testing rowName -> int")
{
    CHECK_EQ(RowNameToInt("1"), 0);
    CHECK_EQ(RowNameToInt("1000"), 999);

    CHECK_THROWS(RowNameToInt(""));
    CHECK_THROWS(RowNameToInt("0"));
    CHECK_THROWS(RowNameToInt("A"));
    CHECK_THROWS(RowNameToInt("-1"));
    CHECK_THROWS(RowNameToInt("0x1"));
    CHECK_THROWS(RowNameToInt("9999999999999999999999999999999999999999"));
}

// --------------------------------------------------------------------------------
