<?php
	header('Content-type: text/xml');
	
	require_once('wp-blog-header.php');
	
	function printUser($user)
	{
		$mgm_fields = mgm_get_member($user->ID);
		$pack_id = $mgm_fields->pack_id;
		$expire_date = $mgm_fields->expire_date;
		$date = date("Y-m-d");

		if ($pack_id == 2) // free user
		{
			print '<user type="1"></user>';
		}
		else if ($pack_id == 3) // indie user
		{
			print "<user type=\"2\" expire=\"$expire_date\" date=\"$date\"></user>";
		}
		else if ($pack_id == 4) // professional user
		{
			print "<user type=\"3\" expire=\"$expire_date\" date=\"$date\"></user>";
		}
		else
		{
			print '<error>Unknown license type. Please contact info@giderosmobile.com</error>';
		}
	}

	$login = $_GET['login'];
	$uid = $_GET['uid'];
	
	if (is_null($login) || is_null($uid))
	{
		print '<error>system error (id=4)</error>';
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

	$count = $wpdb->get_var($wpdb->prepare('SELECT count(*) FROM g_uids WHERE ID=%d AND uid=%s', $user->ID, $uid));
	
	if ($count == 0)
	{		
		print '<error>This computer is not authorized.</error>';
		die();
	}

	printUser($user);

?>
