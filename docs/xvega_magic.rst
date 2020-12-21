XVega magics
============

Magics that allow you to create graph visualizations using `XVega`_ an implementation of vega-light to C++.

X_FIELD
~~~~~~~

.. object:: %X_FIELD name_of_column

  Represents the X axis of the graph. The name of the axis should be the same as the name of the SQLite column (or result of SQLite query).

TYPE
****

.. object:: %TYPE type_of_field

  Sub-attribute of **X_FIELD**.

    Bellow there's list of the types supported by `xeus-sqlite`. If you want to learn more about types please refer to `vega lite type official documentation`_.

  * QUANTITATIVE
  * NOMINAL
  * ORDINAL
  * TEMPORAL

BIN
***

.. object:: %BIN type_of_field

  Sub-attribute of **X_FIELD**.

  Binning discretizes numeric values into a set of bins. If bin is true, default binning parameters are used. 

  To customize binning parameters, you can set bin to a bin definition object, which can have the following properties:

  If you want to learn more about bin please refer to `vega lite bin official documentation`_.

ANCHOR
++++++

.. object:: %ANCHOR bin_position

  Sub-attribute of **BIN**.

  A value in the binned domain at which to anchor the bins, shifting the bin boundaries if necessary to ensure that a boundary aligns with the anchor value.

BASE
++++

.. object:: %BASE number

  Sub-attribute of **BIN**.

  The number base to use for automatic bin determination (default is base 10).

BINNED
++++++

.. object:: %BASE boolean

  Sub-attribute of **BIN**.

MAXBINS
+++++++

.. object:: %MAXBINS number

  Sub-attribute of **BIN**.

  Maximum number of bins.

MINSTEP
+++++++

.. object:: %MINSTEP number

  Sub-attribute of **BIN**.

  A minimum allowable step size (particularly useful for integer values).

NICE
++++

.. object:: %NICE bool

  Sub-attribute of **BIN**.

  If true, attempts to make the bin boundaries use human-friendly boundaries, such as multiples of ten.

STEP
++++

.. object:: %STEP number

  Sub-attribute of **BIN**.

  An exact step size to use between bins.

AGGREGATE
*********

.. object:: %AGGREGATE type_of_aggregation

  Sub-attribute of **X_FIELD**.

  The aggregate property of a field definition can be used to compute aggregate summary statistics (e.g., median, min, max) over groups of data.

  Bellow there's list of the aggregations supported by `xeus-sqlite`. If you want to learn more about aggregations please refer to `vega lite aggregate official documentation`_.

  * COUNT
  * VALID
  * MISSING
  * DISTINCT
  * SUM
  * PRODUCT
  * MEAN
  * AVERAGE
  * VARIANCE
  * VARIANCEP
  * STDEV
  * STEDEVP
  * STEDERR
  * MEDIAN
  * Q1
  * Q3
  * CI0
  * CI1
  * MIN
  * MAX
  * ARGMIN
  * ARGMAX

TIME_UNIT
*********

.. object:: %TIME_UNIT time

  Sub-attribute of **X_FIELD**.

  Time unit is used to discretize time. 

    Bellow there's list of the time units supported by `xeus-sqlite`. If you want to learn more about time units please refer to `vega lite time unit official documentation`_.

  * YEAR
  * QUARTER
  * MONTH
  * DAY
  * DATE
  * HOURS
  * MINUTES
  * SECONDS
  * MILISECONDS

Y_FIELD
~~~~~~~

.. object:: %Y_FIELD name_of_column

  Represents the Y axis of the graph. The name of the axis should be the same as the name of the SQLite column (or result of SQLite query).

TYPE
****

.. object:: %TYPE type_of_field

  Sub-attribute of **Y_FIELD**.

    Bellow there's list of the types supported by `xeus-sqlite`. If you want to learn more about types please refer to `vega lite type official documentation`_.

  * QUANTITATIVE
  * NOMINAL
  * ORDINAL
  * TEMPORAL

BIN
***

.. object:: %BIN type_of_field

  Sub-attribute of **Y_FIELD**.

  Binning discretizes numeric values into a set of bins. If bin is true, default binning parameters are used. 

  To customize binning parameters, you can set bin to a bin definition object, which can have the following properties:

  If you want to learn more about bin please refer to `vega lite bin official documentation`_.

ANCHOR
++++++

.. object:: %ANCHOR bin_position

  Sub-attribute of **BIN**.

  A value in the binned domain at which to anchor the bins, shifting the bin boundaries if necessary to ensure that a boundary aligns with the anchor value.

BASE
+++++

.. object:: %BASE number

  Sub-attribute of **BIN**.

  The number base to use for automatic bin determination (default is base 10).

BINNED
++++++

.. object:: %BASE boolean

  Sub-attribute of **BIN**.

MAXBINS
+++++++

.. object:: %MAXBINS number

  Sub-attribute of **BIN**.

  Maximum number of bins.

MINSTEP
+++++++

.. object:: %MINSTEP number

  Sub-attribute of **BIN**.

  A minimum allowable step size (particularly useful for integer values).

NICE
++++

.. object:: %NICE bool

  Sub-attribute of **BIN**.

  If true, attempts to make the bin boundaries use human-friendly boundaries, such as multiples of ten.

STEP
++++

.. object:: %STEP number

  Sub-attribute of **BIN**.

  An exact step size to use between bins.

AGGREGATE
*********

.. object:: %AGGREGATE type_of_aggregation

  Sub-attribute of **Y_FIELD**.

  The aggregate property of a field definition can be used to compute aggregate summary statistics (e.g., median, min, max) over groups of data.

  Bellow there's list of the aggregations supported by `xeus-sqlite`. If you want to learn more about aggregations please refer to `vega lite aggregate official documentation`_.

  * COUNT
  * VALID
  * MISSING
  * DISTINCT
  * SUM
  * PRODUCT
  * MEAN
  * AVERAGE
  * VARIANCE
  * VARIANCEP
  * STDEV
  * STEDEVP
  * STEDERR
  * MEDIAN
  * Q1
  * Q3
  * CI0
  * CI1
  * MIN
  * MAX
  * ARGMIN
  * ARGMAX

TIME_UNIT
*********

.. object:: %TIME_UNIT time

  Sub-attribute of **Y_FIELD**.

  Time unit is used to discretize time. 

    Bellow there's list of the time units supported by `xeus-sqlite`. If you want to learn more about time units please refer to `vega lite time unit official documentation`_.

  * YEAR
  * QUARTER
  * MONTH
  * DAY
  * DATE
  * HOURS
  * MINUTES
  * SECONDS
  * MILISECONDS

WIDTH
~~~~~

.. object:: %WIDTH number

  Width of the graph in pixels.

HEIGHT
~~~~~~

.. object:: %HEIGHT number

  Height of the graph in pixels.

MARK
~~~~

.. object:: %MARK mark

  Marcs can be one of the following:

  * ARC
  * AREA
  * BAR
  * CIRCLE
  * LINE
  * POINT
  * RECT
  * RULE
  * SQUARE
  * TICK
  * TRAIL

COLOR
*****

.. object:: %COLOR color

  Sub-attribute of **MARK**.

  Sets the color of a mark. The color can be one of the `valid CSS color string`_.

GRID
~~~~

.. object:: %HEIGHT boolean

  Enable or disable grid view on graph.


.. _XVega: https://github.com/Quantstack/xvega
.. _valid CSS color string: https://developer.mozilla.org/en-US/docs/Web/CSS/color_value
.. _vega lite aggregate official documentation: https://vega.github.io/vega-lite/docs/aggregate.html#ops
.. _vega lite type official documentation: https://vega.github.io/vega-lite/docs/type.html
.. _vega lite bin official documentation: https://vega.github.io/vega-lite/docs/bin.html#bin-parameters
.. _vega lite time unit official documentation: https://vega.github.io/vega-lite/docs/timeunit.html
