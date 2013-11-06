<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN"
"http://www.w3.org/TR/html4/loose.dtd">
<html>
<head>
<title>Five in line solver database</title>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8">
</head>
<body>
<h3><a href="stat.php">Statistic</a>&nbsp;&nbsp;&nbsp;&nbsp;Participate in solving <a href="solver.zip">Windows</a> <a href="solver.tar.gz">Ubuntu</a></h3>
<?
$EXEC_PATH='/home/shebeko/bin/db';
$x1=-7;
$y1=-7;
$x2=7;
$y2=7;

$neitrals=array();
$solved_wins=array();
$solved_fails=array();
$tree_wins=array();
$tree_fails=array();

class point
{
  var $x;
  var $y;
	function point()
	{
		$this->x=0;
		$this->y=0;
	}
};

define("st_empty",0);
define("st_krestik",1);
define("st_nolik",2);

class step_t extends point
{
	var $step;
	
	function step_t()
	{
		$this->step=st_krestik;
	}
};

class npoint extends point
{
	var $n;
	
	function npoint()
	{
		$this->n=0;
	}
};


function hex2steps($h)
{
	$items_count=(int)(strlen($h)/6);
	
	if($items_count*6!=strlen($h))
		throw new Exception("hex2steps(): invlaid input format");
	
	$ret=array();
	
	for($i=0;$i<$items_count;$i++)
	{
		$a=new step_t;
		$a->x= hexdec(substr($h,$i*6,2));
		if($a->x>127)$a->x=$a->x-256;
		
		$a->y= hexdec(substr($h,$i*6+2,2));
		if($a->y>127)$a->y=$a->y-256;
		
		$a->step=hexdec(substr($h,$i*6+4,2));
		
		array_push($ret,$a);
	}

	return $ret;
}

function step2hex($p)
{
	$x=$p->x;
	$y=$p->y;

	if($x<0)$x=256+$x;
	if($y<0)$y=256+$y;
	
	$h=sprintf("%02X%02X",$x,$y);
	
	if($p->step==st_krestik)$h=$h."01";
	else $h=$h."02";
	
	return $h;
}

function fill_points_array($prefix,$s,&$arr)
{
	$ps=strpos($s,$prefix);
	if($ps===false||$ps!=0)return;
	
	$s=substr($s,strlen($prefix));
	
	$spts=explode(";",$s);
	
	$arr=array();
	
	for($i=0;$i<count($spts);$i++)
	{
		$x=0;
		$y=0;
	  $sp=$spts[$i];

	  if(sscanf($sp,"(%d,%d)",$x,$y)!=2)
	    continue;
		
		$p=new point;
		$p->x=$x;
		$p->y=$y;
		array_push($arr,$p);
	}
}

function fill_steps_array($prefix,$s,&$arr)
{
	$ps=strpos($s,$prefix);
	if($ps===false||$ps!=0)return;
	
	$s=substr($s,strlen($prefix));
	
	$spts=explode(";",$s);
	
	$arr=array();
	
	for($i=0;$i<count($spts);$i++)
	{
		$x=0;
		$y=0;
		$st="";
	  $sp=$spts[$i];
	  if(sscanf($sp,"(%d,%d:%1s)",$x,$y,$st)!=3)
	    continue;
		
		$p=new step_t;
		$p->x=$x;
		$p->y=$y;
		
		if($st=="X"||$st=="x")$p->step=st_krestik;
		else $p->step=st_nolik;
		
		array_push($arr,$p);
	}
}

function fill_npoints_array($prefix,$s,&$arr)
{
	$ps=strpos($s,$prefix);
	if($ps===false||$ps!=0)return;
	
	$s=substr($s,strlen($prefix));
	
	$spts=explode(";",$s);
	
	$arr=array();
	
	for($i=0;$i<count($spts);$i++)
	{
		$x=0;
		$y=0;
		$n=0;
	  $sp=$spts[$i];
	  if(sscanf($sp,"(%d,%d:%d)",$x,$y,$n)!=3)
	    continue;
		
		$p=new point;
		$p->x=$x;
		$p->y=$y;
		$p->n=$n;
		array_push($arr,$p);
	}
}

function expand_bound($arr)
{
	global $x1,$y1,$x2,$y2;
	
	for($i=0;$i<count($arr);$i++)
	{
		$a=$arr[$i];
		if($a->x<$x1)$x1=$a->x;
		if($a->x>$x2)$x2=$a->x;
		if($a->y<$y1)$y1=$a->y;
		if($a->y>$y2)$y2=$a->y;
	}
}

function make_point_key_array($arr)
{
	$ret=array();
	
	for($i=0;$i<count($arr);$i++)
	{
		$p=$arr[$i];
		$key=$p->x."_".$p->y;
		$ret[$key]=$p;
	}
	
	return $ret;
}


if (count($_GET)>0) $HTTP_VARS=$_GET;
else $HTTP_VARS=$_POST;

$hex_state=NULL;
if(array_key_exists("st",$HTTP_VARS))
{
  $hex_state=$HTTP_VARS["st"];
}

if(!$hex_state)
{
	$steps=Array();
	$cmd="view_root";
}
else
{
	$steps=hex2steps($hex_state);
	$cmd="view_hex ".escapeshellarg($hex_state);
}

$cmd=$EXEC_PATH." ".$DB_PATH." ".$cmd;
exec($cmd,$output);

foreach($output as $s)
{
	if(count($steps)==0)fill_steps_array("key: ",$s,$steps);
	if(!$hex_state)
	{
		$ps=strpos($s,"hex_key: ");
		if(!($ps===false)&&$ps==0)
			$hex_state=substr($s,strlen("hex_key: "));
	}

	fill_points_array("neitrals: ",$s,$neitrals);
	fill_npoints_array("solved wins: ",$s,$solved_wins);
	fill_npoints_array("solved fails: ",$s,$solved_fails);
	fill_npoints_array("tree wins: ",$s,$tree_wins);
	fill_npoints_array("tree fails: ",$s,$tree_fails);
}

expand_bound($steps);
expand_bound($neitrals);
expand_bound($solved_wins);
expand_bound($solved_fails);
expand_bound($tree_wins);
expand_bound($tree_fails);

$key_steps=make_point_key_array($steps);
$neitrals=make_point_key_array($neitrals);
$solved_wins=make_point_key_array($solved_wins);
$solved_fails=make_point_key_array($solved_fails);
$tree_wins=make_point_key_array($tree_wins);
$tree_fails=make_point_key_array($tree_fails);

?>
<table border="1" cellpadding="0" cellspacing="0">
<?
for($y=$y1;$y<=$y2;$y++)
{
	echo '<tr height="32">';
	for($x=$x1;$x<=$x2;$x++)
	{
		$skey=$x."_".$y;
		$cnt="";
		
		if(array_key_exists($skey,$key_steps))
		{
			$p=$key_steps[$skey];
			
			$special=false;
			if(($x==0)&&($y==0))$special=true;
			else
			{
				$lp=$steps[count($steps)-1];
				if($lp->x==$p->x&&$lp->y==$p->y)$special=true;
				else if(count($steps)>2)
				{
					$lp=$steps[count($steps)-2];
					if($lp->x==$p->x&&$lp->y==$p->y)$special=true;
				}
			}
			
			if($p->step==st_krestik)
			{
				if($special)$cnt="x";
				else $cnt="X";
			}
			else
			{
				if($special)$cnt="o";
				else $cnt="O";
			}
		}
		else if(array_key_exists($skey,$neitrals))
		{
			$p=$neitrals[$skey];

			$o=new step_t;
			$o->x=$p->x;
			$o->y=$p->y;
			
			if((count($steps)%2)==0)$o->step=st_krestik;
			else $o->step=st_nolik;
		
			$h=$hex_state.step2hex($o);
			$cnt='<a href="'.$OWN_PATH.'?st='.$h.'">n</a>';
		}
		else if(array_key_exists($skey,$solved_wins))
		{
			$p=$solved_wins[$skey];
			$cnt="w".$p->n;
		}
		else if(array_key_exists($skey,$solved_fails))
		{
			$p=$solved_fails[$skey];
			$cnt="f".$p->n;
		}
		else if(array_key_exists($skey,$tree_wins))
		{
			$p=$tree_wins[$skey];
			
			$o=new step_t;
			$o->x=$p->x;
			$o->y=$p->y;
			
			if((count($steps)%2)==0)$o->step=st_krestik;
			else $o->step=st_nolik;
		
			$h=$hex_state.step2hex($o);
			$cnt='<a href="'.$OWN_PATH.'?st='.$h.'">'."W".$p->n.'</a>';
		}
		else if(array_key_exists($skey,$tree_fails))
		{
			$p=$tree_fails[$skey];
			
			$o=new step_t;
			$o->x=$p->x;
			$o->y=$p->y;
			
			if((count($steps)%2)==0)$o->step=st_krestik;
			else $o->step=st_nolik;
		
			$h=$hex_state.step2hex($o);
			$cnt='<a href="'.$OWN_PATH.'?st='.$h.'">'."F".$p->n.'</a>';
		}

		echo '<td width="32" align="center" valign="middle">';
		echo $cnt;
		echo "</td>";
	}
	echo "</tr>\r\n";
}
?>
</table>

<pre>
<?
passthru($cmd,$r);
?>
</pre>
</body>
</html>
