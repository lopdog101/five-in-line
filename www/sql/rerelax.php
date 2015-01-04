<?php
$EXEC_PATH='/home/shebeko/bin/db';
$DB_PATH='/home/shebeko/f5/root';
putenv("mysql_db=f5");

$link = mysql_connect('localhost', 'www-data') or die('Could not connect: ' . mysql_error());
mysql_select_db('f5') or die('Could not select database');

$saved=0;

for($i=1;$i<=150;$i++)
{
	$query = "select hex(k) as k from s$i where solved_wins<>'' OR (solved_fails<>'' AND tree_fails='' and tree_wins='' and neitrals='')";
	$result = mysql_query($query) or die('Query failed: ' . mysql_error());
	
	$level_count=0;
	while ($line = mysql_fetch_array($result, MYSQL_ASSOC))
	{
		$k=$line["k"];
		
		echo "$i:$level_count:$saved=$k\n";

		$cmd_line=$EXEC_PATH." ".$DB_PATH." ".escapeshellarg("relax")." ".escapeshellarg($k);
		passthru($cmd_line,$r);
		
		if($r)
		  exit($r);
			
		$saved++;
		$level_count++;
	}

	mysql_free_result($result);
}

// Closing connection
mysql_close($link);

?>
