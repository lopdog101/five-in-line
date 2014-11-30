<?php
$EXEC_PATH='/home/shebeko/bin/db';
putenv("mysql_db=f5");

if (count($_GET)>0) $HTTP_VARS=$_GET;
else $HTTP_VARS=$_POST;

$cmd=$HTTP_VARS["cmd"];

function write_result_file($file_name)
{
	$handle = fopen($file_name, "w");
	for($i=0;array_key_exists("k".$i,$_POST);$i++)
	{
		$k=$_POST["k".$i];
		
		if(array_key_exists("n".$i,$_POST))$n=$_POST["n".$i];
		else $n="";
		
		if(array_key_exists("w".$i,$_POST))$w=$_POST["w".$i];
		else $w="";

		if(array_key_exists("f".$i,$_POST))$f=$_POST["f".$i];
		else $f="";
	
		fwrite($handle,"$k;$n;$w;$f\n");
	}
	
	fclose($handle);
}

if($cmd=='get_job')
{
	if($USE_ANT)
		$cmd='get_ant_job';
	
	$cmd=$EXEC_PATH." ".$DB_PATH." ".escapeshellarg($cmd);
	
	if(array_key_exists("root",$HTTP_VARS))
	  $cmd=$cmd." ".escapeshellarg($HTTP_VARS["root"]);
	else if(isset($OWN_ROOT))
	  $cmd=$cmd." ".escapeshellarg($OWN_ROOT);

	passthru($cmd,$r);
}
else if($cmd=='save_job')
{
	if(!array_key_exists("k0",$_POST))
	{
		echo 'post k0 does not exist<br>';
		exit(1);
	}
	
	$file_name = tempnam(sys_get_temp_dir(), 'save_job');
	
	write_result_file($file_name);
	
	passthru($EXEC_PATH." ".$DB_PATH
		." ".escapeshellarg($cmd)
		." ".escapeshellarg($file_name)
		,$r);
	
	unlink($file_name);
}
else
{
  echo 'five in line solve database<br>';
}

?>
