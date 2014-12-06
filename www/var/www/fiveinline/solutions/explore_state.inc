<?php
$EXEC_PATH='/home/shebeko/bin/db';
putenv("mysql_db=f5");
putenv("stored_deep=1");
putenv("ant_count=100");

if (count($_GET)>0) $HTTP_VARS=$_GET;
else $HTTP_VARS=$_POST;

$access_verified=false;
if(array_key_exists("godmode",$HTTP_VARS))
{
	$godmode_val=file_get_contents($DB_PATH."/godmode");
	if(!($godmode_val === false) && $HTTP_VARS["godmode"]==$godmode_val)
	{
		$access_verified=true;
	}
}

if(!$access_verified)
{
	echo "Access denied<br>";
	exit(1);
}

$st=$HTTP_VARS["st"];
if(!$st)
{
	echo "'st' is empty<br>";
	exit(1);
}

$cmd=$EXEC_PATH." ".$DB_PATH." solve_ant ".escapeshellarg($st)." 1";
passthru($cmd,$r);


?>
