
  fredsheet
 ===========

_...probably nothing..._




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

