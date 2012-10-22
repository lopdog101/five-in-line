<?
$BASE_DIR='/home/shebeko/src/gomoku';
$DB_PATH='/a/f5/db';

if (count($_GET)>0) $HTTP_VARS=$_GET;
else $HTTP_VARS=$_POST;

$cmd=$HTTP_VARS["cmd"];

if($cmd=='get_job')
{
	passthru($BASE_DIR."/db/db ".$DB_PATH." ".escapeshellarg($cmd),$r);
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
		passthru($BASE_DIR."/db/db ".$DB_PATH
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

//if($r) throw new Exception("prepare_tmp(): remove old failed");
?>