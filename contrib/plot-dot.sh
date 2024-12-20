#!/bin/sh

dot -V 2>/dev/null
if [ $? -ne 0 ]; then
  echo "Graphviz dot program is necessary to plot graph files."
  exit 1
fi

evince -h > /dev/null
if [ $? -ne 0 ]; then
  echo "Evince PDF reader is necessary to display the graph."
  exit 1
fi

if [ $# -ne 1 ]; then
  echo "Usage: plot-dot.sh <dot-file>"
  exit 1
fi


TEMP=$(mktemp)

# Convert to pdf
echo "Converting to PDF..."
dot -Tpdf $1 -o $TEMP.pdf
evince $TEMP.pdf
rm -f $TEMP.pdf
