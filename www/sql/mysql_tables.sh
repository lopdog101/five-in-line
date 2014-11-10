#!/bin/sh

COUNTER=1
while [  $COUNTER -lt 151 ]; do
  let key_len=COUNTER*3

echo "CREATE TABLE s$COUNTER("
echo "  k varbinary($key_len) NOT NULL DEFAULT '',"
 
echo "  state integer NOT NULL default 0,"
echo "  wins_count BIGINT NOT NULL default 0,"
echo "  fails_count BIGINT NOT NULL default 0,"

echo "  neitrals varbinary(800) NOT NULL,"
echo "  solved_wins varbinary(800) NOT NULL,"
echo "  solved_fails varbinary(800) NOT NULL,"
echo "  tree_wins varbinary(800) NOT NULL,"
echo "  tree_fails varbinary(800) NOT NULL,"

echo "  PRIMARY KEY (k) USING HASH"
echo ") ENGINE=MyISAM;"
echo ""


  let COUNTER=COUNTER+1 
done
