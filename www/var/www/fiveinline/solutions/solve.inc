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

function get_job_from_list()
{
	global $EXEC_PATH,$DB_PATH,$USE_ANT;
	
	if($USE_ANT)
		$cmd='get_ant_job';
	else 
		$cmd='get_job';

	$cmd=$EXEC_PATH." ".$DB_PATH." ".escapeshellarg($cmd);
	$file_name=$DB_PATH."/marked.txt";
	$lst=file_get_contents($file_name);

	if($lst === false)
	  return false;
  
	$lst=explode("\n",trim($lst));
	
	while(count($lst)>0)
	{
		$k=mt_rand(0,count($lst)+1);
		//force first state
		if($k<2)$k=0;
		else $k=$k-2;
		
		$v=$lst[$k];
		
		if(!v || v=='')
		  return false;
		
		$cc=$cmd." ".escapeshellarg($v);
		
		$output=array();
		exec($cc,$output);
		
		$output=implode("\n",$output);
		
		if(strpos($output,"no job anymore") === false)
		{
			echo $output;
			return true;
		}
		
		unset($lst[$k]);
		
		$content = trim(implode("\n",$lst));
		file_put_contents($file_name, $content );
		$lst=explode("\n",$content);
	}
	
	return false;
}


if($cmd=='get_job')
{
	if($USE_ANT)
		$cmd='get_ant_job';
	
	$cmd=$EXEC_PATH." ".$DB_PATH." ".escapeshellarg($cmd);
	
	if(array_key_exists("root",$HTTP_VARS))
	  $root=$HTTP_VARS["root"];
	else if(isset($OWN_ROOT))
	  $root=$OWN_ROOT;
	  
    if(!$root)
	{
	  if(get_job_from_list())
	    exit(0);
	}
	
    if($root)
	  $cmd=$cmd." ".escapeshellarg($root);
	  
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
