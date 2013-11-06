<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN"
"http://www.w3.org/TR/html4/loose.dtd">
<html>
<head>
<title>Five in line solver database</title>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8">
</head>
<body>
<?
$conn=pg_connect("dbname=f5");
if($conn === FALSE)
{
  echo "Could not connect to f5 database\n";
  exit(0);
}
?>
<h3>All</h3>
<table border="1px" cellPadding="4px" cellSpacing="0px">
<?
  $sql="select src_name, sum(solve_count) as sum_cnt from solve_stat where root_name='".$ROOT_NAME."' group by src_name order by sum_cnt desc";
  
  $res = pg_query($conn,$sql);
  if($res === FALSE)
  {  
    echo "Error execute: ".$sql." error: ".pg_result_error($res);
	exit(0);
  }
  
  while ($row = pg_fetch_row($res))
  {
    echo "<tr><td>$row[0]</td><td>$row[1]</td></tr>";
  }
  pg_free_result($res);
?>
</table>
<h3>Yesterday</h3>
<table border="1px" cellPadding="4px" cellSpacing="0px">
<?
  $sql="select src_name, sum(solve_count) as sum_cnt from solve_stat where root_name='".$ROOT_NAME."' AND log_date>=now()::date-'2 day'::interval AND log_date<now()::date-'1 day'::interval group by src_name order by sum_cnt desc";

  $res = pg_query($conn,$sql);
  if($res === FALSE)
  {  
    echo "Error execute: ".$sql." error: ".pg_result_error($res);
	exit(0);
  }
  
  while ($row = pg_fetch_row($res))
  {
    echo "<tr><td>$row[0]</td><td>$row[1]</td></tr>";
  }
  pg_free_result($res);
?>
</table>
<?
pg_close($conn);
?>
</body>
</html>
