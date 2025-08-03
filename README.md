
  fredsheet
 ===========

_what if there was a tiny spreadsheet program that just worked for 80% of quick cases?_


## DESIGN GOALS

### fast, small, standalone
* < 100ms for all common operations
* < 5MB distro zip size
* minimal external dependencies beyond OS & gfx drivers
* no install needed

### focused on non-enterprise use cases
* excel has that wrapped up & locked in anyway
* openoffice calc is daunting and heavyweight
* numbers is only on apple things and a bit of a departure from sheet-based workflows
* "i just want to..."
    * manage household finances
    * open a csv file
    * explore & graph some data
        -> this might need limited .ods and .xlsx import to really be satisfied. that's probably fine.

### simple file storage
* `budget.fred` is a folder of mostly CSV files
    * `tableN.csv` -- the user-facing values of the data for one table of data (aka sheet)
    * `tableN_source.csv` -- the backing data for the table in machine-friendly stable format
    * `tableN_meta.json` -- styles, chart info etc for the table in question
    * `chartN.png` -- a rendered copy of the nth chart in the document
    * `doc.json` -- describes how the "document" should be assmebled from the various sources
* expectation is that opening a `.csv` file is equivalent to opening a `.fred` with the input file as `table1.csv`
* `.fred` data can be loaded from a zipped file or a folder transparently to ease integration with other scripts tools


### nice-to-have features
* easy integration with eg. jupyter / R for nice data analysis & exploration workflows
* basic .ods / .xlsx import
* printing


OUT OF SCOPE
------------
* excel compatibility besides what's trivially available (CSV export/import)
* macros
* OLE
* complex pivot table setups


UI LIB ASSUMPTIONS
------------------

1. cost must inversely match frequency
    * interaction: ultralow cost, ultrahigh frequency
    * render: low cost, high frequency
    * layout: medium cost, medium frequency
    * hierarchy change: high cost, low frequency

2. output of lower-frequency phases must be efficient for high-frequency phase

3. none of this matters yet


### Measuring & Layout

* all controls have a MinDesiredSize and a IsFixedSize flag
    * any controls that aren't Fixed Size are allowed to be stretched as desired by the layout of the parent
* DesiredSizeDirty is set true on any size- or layout-altering operation on a non-fixed-size control
    * the flag is immediately propagated up to parents that don't have the flag set already
    
1. recursively ensure that a hierarchy has been Measured
    * depth-first, ensure that MinDesiredSize has been updated for any dirty controls
2. top-down, apply layout algos to establish new current size for all parents, before recursing to children

