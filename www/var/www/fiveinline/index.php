<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN"
"http://www.w3.org/TR/html4/loose.dtd">
<html>
<head>
<title>Five-in-line project</title>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8">
</head>
<body>
<h1>Five-in-line solving tree project</h1>
<p>Project sources on <a href="https://code.google.com/p/five-in-line/">https://code.google.com/p/five-in-line/</a></p>
<table cellSpacing="8">
<?
function add_solution($path_name)
{
   echo "<tr><td><a href=\"solutions/$path_name/\"><img src=\"solutions/$path_name/avatar.png\"/ width=\"32\" height=\"32\"></a></p></td></tr>";
}

add_solution("root");
add_solution("v14");
?>
  
</table>
<table cellSpacing="8">
<?
function add_version($d,$m,$y)
{
	echo "<tr><td>$d.$m.$y</td><td><a href=\"download/five_in_line$y$m$d.zip\">GUI Game</a><td><a href=\"download/db$y$m$d.zip\">Server</a></td></tr>";
}

add_version("22","10","2013");
add_version("04","10","2013");
?>
  
</table>
</body>
</html>
