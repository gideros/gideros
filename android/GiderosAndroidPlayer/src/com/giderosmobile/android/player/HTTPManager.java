package com.giderosmobile.android.player;

import java.io.ByteArrayOutputStream;
import java.io.InputStream;
import java.net.URI;
import java.util.concurrent.ConcurrentHashMap;
import org.apache.http.HttpResponse;
import org.apache.http.client.methods.HttpDelete;
import org.apache.http.client.methods.HttpGet;
import org.apache.http.client.methods.HttpPost;
import org.apache.http.client.methods.HttpPut;
import org.apache.http.client.methods.HttpUriRequest;
import org.apache.http.entity.ByteArrayEntity;
import org.apache.http.impl.client.DefaultHttpClient;

public class HTTPManager
{
	void Get(String url, String[] headers, long udata, long id)
	{
		HTTPThread thread = new HTTPThread(url, headers, null, 0, udata, id, this);
		
		ids_.put(id, thread);
		
		thread.start();
	}

	void Post(String url, String[] headers, byte[] data, long udata, long id)
	{
		HTTPThread thread = new HTTPThread(url, headers, data, 1, udata, id, this);
		
		ids_.put(id, thread);
		
		thread.start();
	}

	void Put(String url, String[] headers, byte[] data, long udata, long id)
	{
		HTTPThread thread = new HTTPThread(url, headers, data, 2, udata, id, this);
		
		ids_.put(id, thread);
		
		thread.start();
	}

	void Delete(String url, String[] headers, long udata, long id)
	{
		HTTPThread thread = new HTTPThread(url, headers, null, 3, udata, id, this);
		
		ids_.put(id, thread);
		
		thread.start();
	}

	void Close(long id)
	{
		if (!ids_.containsKey(id))
			return;
		
		HTTPThread thread = ids_.get(id);
		
		ids_.remove(id);

		thread.close = true;
	}
	
	void CloseAll()
	{
		HTTPThread[] threads = ids_.values().toArray(new HTTPThread[0]);
		
		ids_.clear();

		for (int i = 0; i < threads.length; ++i)
			threads[i].close = true;
	}
	
	public static native void nativeghttpResponseCallback(long id, byte[] data, int size, int statusCode, long udata);
	public static native void nativeghttpErrorCallback(long id, long udata);
	public static native void nativeghttpProgressCallback(long id, int bytesLoaded, int bytesTotal, long udata);
	
	ConcurrentHashMap<Long, HTTPThread> ids_ = new ConcurrentHashMap<Long, HTTPThread>();
	
	public void responseCallback(long id, long udata, byte[] data, int statusCode)
	{
		if (!ids_.containsKey(id))
			return;
		
		nativeghttpResponseCallback(id, data, data.length, statusCode, udata);

		ids_.remove(id);
	}

	public void errorCallback(long id, long udata)
	{
		if (!ids_.containsKey(id))
			return;
		
		nativeghttpErrorCallback(id, udata);
		
		ids_.remove(id);
	}
	
	public void progressCallback(long id, long udata, int bytesLoaded, int bytesTotal)
	{
		if (!ids_.containsKey(id))
			return;

		nativeghttpProgressCallback(id, bytesLoaded, bytesTotal, udata);
	}

	static private HTTPManager httpManager = null;
	
	static public void ghttp_Init()
	{
		httpManager = new HTTPManager();
	}

	static public void ghttp_Cleanup()
	{
		httpManager.CloseAll();
		httpManager = null;
	}
	
	static public void ghttp_Get(String url, String[] headers, long udata, long id)
	{
		httpManager.Get(url, headers, udata, id);		
	}

	static public void ghttp_Post(String url, String[] headers, byte[] data, long udata, long id)
	{
		httpManager.Post(url, headers, data, udata, id);		
	}

	static public void ghttp_Put(String url, String[] headers, byte[] data, long udata, long id)
	{
		httpManager.Put(url, headers, data, udata, id);		
	}

	static public void ghttp_Delete(String url, String[] headers, long udata, long id)
	{
		httpManager.Delete(url, headers, udata, id);		
	}
	
	static public void ghttp_Close(long id)
	{
		httpManager.Close(id);
	}

	static public void ghttp_CloseAll()
	{
		httpManager.CloseAll();
	}
}

class HTTPThread extends Thread
{
	long id_;
	long udata_;
	String url_;
	String[] headers_;
	byte[] data_;
	HTTPManager manager_;
	int method_;

	volatile boolean close;
	
	public HTTPThread(String url, String[] headers, byte[] data, int method, long udata, long id, HTTPManager manager)
	{
		url_ = url;
		headers_ = headers;
		data_ = data;
		method_ = method;
		udata_ = udata;
		id_ = id;
		manager_ = manager;	
		
		close = false;
	}
	
	@Override
	public void run()
	{
		try
		{
			DefaultHttpClient httpClient = new DefaultHttpClient();
			
			HttpUriRequest method = null;			
			switch (method_)
			{
			case 0:
				HttpGet get = new HttpGet(new URI(url_));
				get.setHeader("User-Agent", "Gideros");
				if (headers_ != null)
					for (int i = 0; i < headers_.length; i += 2)
						get.setHeader(headers_[i], headers_[i + 1]);
				method = get;
				break;
			case 1:
				HttpPost post = new HttpPost(new URI(url_));
				post.setHeader("User-Agent", "Gideros");
				if (headers_ != null)
					for (int i = 0; i < headers_.length; i += 2)
						post.setHeader(headers_[i], headers_[i + 1]);
				if (data_ != null)
					post.setEntity(new ByteArrayEntity(data_));
				method = post;				
				break;
			case 2:
				HttpPut put = new HttpPut(new URI(url_));
				put.setHeader("User-Agent", "Gideros");
				if (headers_ != null)
					for (int i = 0; i < headers_.length; i += 2)
						put.setHeader(headers_[i], headers_[i + 1]);
				if (data_ != null)
					put.setEntity(new ByteArrayEntity(data_));
				method = put;				
				break;
			case 3:
				HttpDelete delete = new HttpDelete(new URI(url_));
				delete.setHeader("User-Agent", "Gideros");
				if (headers_ != null)
					for (int i = 0; i < headers_.length; i += 2)
						delete.setHeader(headers_[i], headers_[i + 1]);
				method = delete;
				break;			
			}
			HttpResponse response = httpClient.execute(method);
			int statusCode = response.getStatusLine().getStatusCode();
			int contentLength = (int)response.getEntity().getContentLength();
			InputStream input = response.getEntity().getContent();
			ByteArrayOutputStream output = new ByteArrayOutputStream();
	
			int readBytes = 0;
			byte[] sBuffer = new byte[1024];
			while ((readBytes = input.read(sBuffer)) != -1)
			{
				if (close == true)
				{
					httpClient.getConnectionManager().shutdown();
					return;					
				}
				output.write(sBuffer, 0, readBytes);
				manager_.progressCallback(id_, udata_, output.size(), contentLength);
			}
		
			httpClient.getConnectionManager().shutdown();
			
			manager_.responseCallback(id_, udata_, output.toByteArray(), statusCode);
		}
		catch (Exception e)
		{
			manager_.errorCallback(id_, udata_);
		}
	}
}
