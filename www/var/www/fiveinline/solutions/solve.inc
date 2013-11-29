<?
$EXEC_PATH='/home/shebeko/bin/db';

if (count($_GET)>0) $HTTP_VARS=$_GET;
else $HTTP_VARS=$_POST;

$cmd=$HTTP_VARS["cmd"];

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
	$key=$HTTP_VARS["key"];
	$n=$HTTP_VARS["n"];
	$w=$HTTP_VARS["w"];
	$f=$HTTP_VARS["f"];
	
	if($key==''||$n==''||$w==''||$f=='')echo "invalid params<br>";
	else
	{
		passthru($EXEC_PATH." ".$DB_PATH
			." ".escapeshellarg($cmd)
			." ".escapeshellarg($key)
			." ".escapeshellarg($n)
			." ".escapeshellarg($w)
			." ".escapeshellarg($f)
			,$r);
	}
}
else
{
  echo 'five in line solve database<br>';
}

?>
