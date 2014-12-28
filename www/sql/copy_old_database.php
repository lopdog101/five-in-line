<?php
$EXEC_PATH='/home/shebeko/bin/db';
$DB_PATH='/home/shebeko/f5/root';
putenv("mysql_db=f5");

$file_name = tempnam(sys_get_temp_dir(), 'copy_db');
echo "file_name=$file_name\n";

$link = mysql_connect('localhost', 'www-data') or die('Could not connect: ' . mysql_error());
mysql_select_db('f5_old') or die('Could not select database');

$saved=0;
$cmd_line=$EXEC_PATH." ".$DB_PATH." ".escapeshellarg("save_job")." ".escapeshellarg($file_name);

for($i=1;$i<=150;$i++)
{
	$query = "select hex(k) as k,hex(neitrals) as n,hex(solved_wins) as w,hex(solved_fails) as f from s$i where solved_wins<>'' OR (solved_fails<>'' AND tree_fails='' and tree_wins='' and neitrals='')";
	$result = mysql_query($query) or die('Query failed: ' . mysql_error());
	
	$level_count=0;
	while ($line = mysql_fetch_array($result, MYSQL_ASSOC))
	{
		$k=$line["k"];
		$n=$line["n"];
		$w=$line["w"];
		$f=$line["f"];
		
		$str="$k;$n;$w;$f\n";
		
		echo "$i:$level_count:$saved=$str";
		file_put_contents($file_name,$str);
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

unlink($file_name);

?>
