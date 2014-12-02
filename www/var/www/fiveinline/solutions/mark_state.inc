<?php
$EXEC_PATH='/home/shebeko/bin/db';
putenv("mysql_db=f5");

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


$file_name=$DB_PATH."/marked.txt";
$lst=file_get_contents($file_name);

if($lst === false)
  $lst=array();
else
  $lst=explode("\n",$lst);
  
foreach ($lst as $k => $v) 
{
  if($v==$st)
    unset($lst[$k]);
}

while(count($lst)>$MAX_MARKED)
  array_pop($lst);
  
array_unshift($lst,$st);

var_dump($lst);
file_put_contents($file_name,implode("\n",$lst));

?>
