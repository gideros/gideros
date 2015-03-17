<?php

class IOSPusher
{
	private $pass = "";
	private $cert = "";
	private $devices = array();
	private $errors = array();
	
	private $url = "ssl://gateway.sandbox.push.apple.com";
	private $port = "2195";
	
	function __construct($pass, $cert){
		$this->pass = $pass;
		$this->cert = $cert;
	}
	
	public function set_devices($devices){
		if(is_array($devices))
		{
			$this->devices = array_merge($this->devices, $devices);
		}
		else
		{
			$this->devices[] = $devices;
		}
	}
	
	public function get_errors(){
		return $this->errors;
	}
	
	public function push($message, $title, $badge, $sound, $custom){
		$body = array("aps" => array());
		if($title != "")
		{
			$body["aps"]["alert"] = array();
			$body["aps"]["alert"]["action-loc-key"] = $title;
			$body["aps"]["alert"]["body"] = $message;
		}
		else
		{
			$body["aps"]["alert"] = $message;
		}
		if($badge > 0)
		{
			$body["aps"]["badge"] = $badge;
		}
		if($sound != "")
		{
			$body["aps"]["sound"] = $sound;
		}
		if($custom != "")
		{
			$body["custom"] = $custom;
		}
		echo "iOS/n";
		print_r($body);
		$ctx = stream_context_create();
		stream_context_set_option($ctx, 'ssl', 'local_cert', $this->cert);
		stream_context_set_option($ctx, 'ssl', 'passphrase', $this->pass);

		// Open a connection to the APNS server
		$fp = stream_socket_client(
			($this->url).':'.$this->port, $err,
			$errstr, 60, STREAM_CLIENT_CONNECT|STREAM_CLIENT_PERSISTENT, $ctx);

		if (!$fp)
		{
			$this->errors[] = $err." ".$errstr;
		}
		else
		{
			// Encode the message as JSON
			$message = json_encode($body);
			foreach($this->devices as $device)
			{
				$msg = chr(0) . pack('n', 32) . pack('H*', str_replace(' ', '', $device)) . pack('n', strlen($message)) . $message;
				$result = fwrite($fp, $msg);
				if (!$result)
				{
					$this->errors[] = $device." not delivered";
				}
			}
		}
		fclose($fp);
	}
}

class AndroidPusher
{
	private $apikey = "";
	private $devices = array();
	private $errors = array();
	
	private $url = 'https://android.googleapis.com/gcm/send';
	
	function __construct($apikey){
		$this->apikey = $apikey;
	}
	
	public function set_devices($devices){
		if(is_array($devices))
		{
			$this->devices = array_merge($this->devices, $devices);
		}
		else
		{
			$this->devices[] = $devices;
		}
	}
	
	public function get_errors(){
		return $this->errors;
	}
	
	public function push($message, $title, $number, $sound, $custom){
		$headers = array( 
            'Authorization: key=' . $this->apikey,
            'Content-Type: application/json'
        );
		$data = array();
		$data["message"] = $message;
		if($title != "")
		{
			$data["title"] = $title;
		}
		if($sound != "")
		{
			$data["sound"] = $sound;
		}
		if($number > 0 )
		{
			$data["number"] = $number;
		}
		if($custom != "")
		{
			$data["custom"] = $custom;
		}
		$fields = array(
			'registration_ids'  => $this->devices,
			'data'              => $data
		);
		echo "Android/n";
		print_r($fields);
		// Open connection
		$ch = curl_init();
		
		// Set the url, number of POST vars, POST data
		curl_setopt( $ch, CURLOPT_URL, $this->url );
		
		curl_setopt( $ch, CURLOPT_POST, true );
		curl_setopt( $ch, CURLOPT_HTTPHEADER, $headers);
		curl_setopt( $ch, CURLOPT_RETURNTRANSFER, true );
		
		curl_setopt( $ch, CURLOPT_POSTFIELDS, json_encode( $fields ) );
		
		// Disabling SSL Certificate support temporarly
		curl_setopt($ch, CURLOPT_SSL_VERIFYPEER, false);
		
		// Execute post
		$result = curl_exec($ch);
		if ($result === FALSE) {
			$this->error[] = curl_error($ch);
		}
		
		// Close connection
		curl_close($ch);	
	}
}

class Pusher
{
	private $pushers = array();
	private $errors = array();
	private $message = "";
	private $title = "";
	private $number = 0;
	private $sound = "";
	private $custom = "";
	
	//IOS part
	public function set_ios($pass, $cert){
		$this->pushers["ios"] = new IOSPusher($pass, $cert);
	}
	
	public function set_ios_devices($devices){
		$this->pushers["ios"]->set_devices($devices);
	}
	
	//Android part
	public function set_android($apikey){
		$this->pushers["android"] = new AndroidPusher($apikey);
	}
	
	public function set_android_devices($devices){
		$this->pushers["android"]->set_devices($devices);
	}
	
	//Common part
	public function get_errors(){
		foreach($this->pushers as $key => $val)
		{
			$arr = $val->get_errors();
			if(!empty($arr))
			{
				$this->errors[$key] = $arr;
			}
		}
		return $this->errors;
	}
	
	public function set_message($message){
		$this->message = $message;
	}
	
	public function set_title($title){
		$this->title = $title;
	}
	
	public function set_number($number){
		$this->number = $number;
	}
	
	public function set_sound($sound){
		$this->sound = $sound;
	}
	
	public function set_custom($custom){
		$this->custom = $custom;
	}
	
	public function push(){
		if($this->message != "")
		{
			foreach($this->pushers as $val)
			{
				$val->push($this->message, $this->title, $this->number, $this->sound, $this->custom);
			}
		}
	}
}

?>