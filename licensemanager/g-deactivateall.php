<?php
	header('Content-type: text/xml');

	require_once('wp-blog-header.php');
	
	$login = $_GET['login'];
	$password = $_GET['password'];
	
	if (is_null($login) || is_null($password))
	{
		print '<error>system error (id=3)</error>';
		die();
	}
		
	$user = get_user_by('login', $login);
	
	if ($user == false)
		$user = get_user_by('email', $login);

	if ($user == false)
	{
		print '<error>The username or password you entered is incorrect.</error>';
		die();
	}

	if (user_pass_ok($user->user_login, $password) == false)
	{
		print '<error>The username or password you entered is incorrect.</error>';
		die();
	}
	
	$count = $wpdb->query($wpdb->prepare('DELETE FROM g_uids WHERE ID=%d', $user->ID));	

	print "<ok>Deauthorized $count computer(s).</ok>";
?>
