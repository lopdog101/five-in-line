<?
//cat /var/log/nginx/access.log | grep save_job | awk '{;u=substr($7,36);u=substr(u,1,index(u,"&")-1);d=substr($4,2);d=substr(d,1,index(d,":")-1);print u,$1,d;}' | sort | uniq -c

class Item { 
    public $ip = '';
    public $date = '';
    public $src_name='';
    public $rec_count=1;

    function getId() {
        return $this->src_name."_".$this->date."_".$this->ip;
    }
}

if($argc<2)
{
  echo "USE: php stat.php <nginx log file>\n";
  exit(1);
}


$file_name = $argv[1];

$handle = fopen($file_name, "r");
if (!$handle)
{
  echo "Cant open: ".$file_name."\n";
  exit(1);
}

$items=array();

while (($ln = fgets($handle, 4096)) !== false)
{
  if(strpos($ln,"cmd=save_job") === false)
     continue;

  $lna = split(' ', $ln);

  if(count($lna)<7)
    continue;

  $it=new Item;

  $it->ip=$lna[0];
  $it->date=$lna[3];
  $it->src_name=$lna[6];

  $it->date = substr($it->date,strpos($it->date,"[")+1);

  $ps=strpos($it->date,":");
  if($ps === false)
  {
    continue;
  }

  $it->date=substr($it->date,0,$ps);

  $ps=strpos($it->src_name,"src_name=");
  if($ps === false)
  {
    continue;
  }

  $it->src_name=substr($it->src_name,$ps+9);

  $ps=strpos($it->src_name,"&");
  if($ps === false)
  {
    continue;
  }

  $it->src_name=substr($it->src_name,0,$ps);

  if(array_key_exists($it->getId(),$items))
  {
     $oi=$items[$it->getId()];
     $oi->rec_count++;
  }
  else
  {
    $items[$it->getId()]=$it;
  }
}

fclose($handle);

foreach($items as $k => $it)
{
  echo $it->ip." ".$it->date." ".$it->src_name." ".$it->rec_count."\n";
}



exit(0);

?>
