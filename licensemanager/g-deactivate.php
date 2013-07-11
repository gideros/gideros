<?php
	header('Content-type: text/xml');
	
	require_once('wp-blog-header.php');

	$login = $_GET['login'];
	$uid = $_GET['uid'];
	
	if (is_null($login) || is_null($uid))
	{
		print '<error>system error (id=2)</error>';
		die();
	}

	$user = get_user_by('login', $login);
	
	if ($user == false)
		$user = get_user_by('email', $login);
	
	if ($user == false)
	{
		print '<error>The username is not found.</error>';
		die();
	}
	
	$count = $wpdb->query($wpdb->prepare('DELETE FROM g_uids WHERE ID=%d AND uid=%s', $user->ID, $uid));	
	/*
	if ($count == 0)
	{
		print '<error>This computer is not authorized yet.</error>';
		die();
	}
	*/
	
	print '<ok/>'
?>
