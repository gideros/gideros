package com.giderosmobile.android.player;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.net.Socket;
import java.net.URI;
import java.net.UnknownHostException;
import java.security.KeyManagementException;
import java.security.KeyStore;
import java.security.KeyStoreException;
import java.security.NoSuchAlgorithmException;
import java.security.UnrecoverableKeyException;
import java.security.cert.CertificateException;
import java.security.cert.X509Certificate;
import java.util.concurrent.ConcurrentHashMap;

import javax.net.ssl.SSLContext;
import javax.net.ssl.TrustManager;
import javax.net.ssl.X509TrustManager;

import org.apache.http.Header;
import org.apache.http.HttpHost;
import org.apache.http.HttpResponse;
import org.apache.http.auth.AuthScope;
import org.apache.http.auth.UsernamePasswordCredentials;
import org.apache.http.client.methods.HttpDelete;
import org.apache.http.client.methods.HttpGet;
import org.apache.http.client.methods.HttpPost;
import org.apache.http.client.methods.HttpPut;
import org.apache.http.client.methods.HttpUriRequest;
import org.apache.http.conn.ClientConnectionManager;
import org.apache.http.conn.params.ConnRouteParams;
import org.apache.http.conn.scheme.PlainSocketFactory;
import org.apache.http.conn.scheme.Scheme;
import org.apache.http.conn.scheme.SchemeRegistry;
import org.apache.http.conn.ssl.SSLSocketFactory;
import org.apache.http.entity.ByteArrayEntity;
import org.apache.http.impl.client.DefaultHttpClient;
import org.apache.http.impl.conn.SingleClientConnManager;

public class HTTPManager {
	static boolean ignoreSslErrors = false;
	static String proxyName,proxyUser,proxyPass;
	static int proxyPort=0;

	void Get(String url, String[] headers, long udata, long id) {
		HTTPThread thread = new HTTPThread(url, headers, null, 0, udata, id,
				this);

		ids_.put(id, thread);

		thread.start();
	}

	void Post(String url, String[] headers, byte[] data, long udata, long id) {
		HTTPThread thread = new HTTPThread(url, headers, data, 1, udata, id,
				this);

		ids_.put(id, thread);

		thread.start();
	}

	void Put(String url, String[] headers, byte[] data, long udata, long id) {
		HTTPThread thread = new HTTPThread(url, headers, data, 2, udata, id,
				this);

		ids_.put(id, thread);

		thread.start();
	}

	void Delete(String url, String[] headers, long udata, long id) {
		HTTPThread thread = new HTTPThread(url, headers, null, 3, udata, id,
				this);

		ids_.put(id, thread);

		thread.start();
	}

	void Close(long id) {
		if (!ids_.containsKey(id))
			return;

		HTTPThread thread = ids_.get(id);

		ids_.remove(id);

		thread.close = true;
	}

	void CloseAll() {
		HTTPThread[] threads = ids_.values().toArray(new HTTPThread[0]);

		ids_.clear();

		for (int i = 0; i < threads.length; ++i)
			threads[i].close = true;
	}

	public static native void nativeghttpResponseCallback(long id, byte[] data,
			int size, int statusCode, int hdrCount, int hdrSize, long udata);

	public static native void nativeghttpErrorCallback(long id, long udata);

	public static native void nativeghttpProgressCallback(long id,
			int bytesLoaded, int bytesTotal, long udata);

	ConcurrentHashMap<Long, HTTPThread> ids_ = new ConcurrentHashMap<Long, HTTPThread>();

	public void responseCallback(long id, long udata, byte[] data,
			int statusCode, int hdrCount, int dataSize) {
		if (!ids_.containsKey(id))
			return;

		nativeghttpResponseCallback(id, data, dataSize, statusCode, hdrCount, data.length-dataSize, udata);

		ids_.remove(id);
	}

	public void errorCallback(long id, long udata) {
		if (!ids_.containsKey(id))
			return;

		nativeghttpErrorCallback(id, udata);

		ids_.remove(id);
	}

	public void progressCallback(long id, long udata, int bytesLoaded,
			int bytesTotal) {
		if (!ids_.containsKey(id))
			return;

		nativeghttpProgressCallback(id, bytesLoaded, bytesTotal, udata);
	}

	static private HTTPManager httpManager = null;

	static public void ghttp_Init() {
		httpManager = new HTTPManager();
	}

	static public void ghttp_Cleanup() {
		httpManager.CloseAll();
		httpManager = null;
	}

	static public void ghttp_IgnoreSslErrors() {
		HTTPManager.ignoreSslErrors = true;
	}

	static public void ghttp_SetProxy(String name,int port,String user,String pass) {
		HTTPManager.proxyName=name;
		HTTPManager.proxyPort=port;
		HTTPManager.proxyUser=user;
		HTTPManager.proxyPass=pass;
	}

	static public void ghttp_Get(String url, String[] headers, long udata,
			long id) {
		httpManager.Get(url, headers, udata, id);
	}

	static public void ghttp_Post(String url, String[] headers, byte[] data,
			long udata, long id) {
		httpManager.Post(url, headers, data, udata, id);
	}

	static public void ghttp_Put(String url, String[] headers, byte[] data,
			long udata, long id) {
		httpManager.Put(url, headers, data, udata, id);
	}

	static public void ghttp_Delete(String url, String[] headers, long udata,
			long id) {
		httpManager.Delete(url, headers, udata, id);
	}

	static public void ghttp_Close(long id) {
		httpManager.Close(id);
	}

	static public void ghttp_CloseAll() {
		httpManager.CloseAll();
	}
}

class TrustAllHttpClient extends DefaultHttpClient {
	TrustManager easyTrustManager = new X509TrustManager() {
		@Override
		public void checkClientTrusted(X509Certificate[] chain, String authType)
				throws CertificateException {
		}

		@Override
		public void checkServerTrusted(X509Certificate[] chain, String authType)
				throws CertificateException {
		}

		@Override
		public X509Certificate[] getAcceptedIssuers() {
			return null;
		}
	};

	public TrustAllHttpClient() {
	}

	@Override
	protected ClientConnectionManager createClientConnectionManager() {
		SchemeRegistry registry = new SchemeRegistry();
		registry.register(new Scheme("http", PlainSocketFactory
				.getSocketFactory(), 80));
		registry.register(new Scheme("https", newSslSocketFactory(), 443));
		return new SingleClientConnManager(getParams(), registry);
	}

	private MySSLSocketFactory newSslSocketFactory() {
		try {
			KeyStore trusted = KeyStore.getInstance("BKS");
			try {
				trusted.load(null, null);

			} finally {
			}

			MySSLSocketFactory sslfactory = new MySSLSocketFactory(trusted);
			sslfactory
					.setHostnameVerifier(SSLSocketFactory.ALLOW_ALL_HOSTNAME_VERIFIER);
			return sslfactory;
		} catch (Exception e) {
			throw new AssertionError(e);
		}

	}

	public class MySSLSocketFactory extends SSLSocketFactory {
		SSLContext sslContext = SSLContext.getInstance("TLS");

		public MySSLSocketFactory(KeyStore certs)
				throws NoSuchAlgorithmException, KeyManagementException,
				KeyStoreException, UnrecoverableKeyException {
			super(certs);
			sslContext.init(null, new TrustManager[] { easyTrustManager }, null);
		}

		@Override
		public Socket createSocket(Socket socket, String host, int port,
				boolean autoClose) throws IOException, UnknownHostException {
			return sslContext.getSocketFactory().createSocket(socket, host,
					port, autoClose);
		}

		@Override
		public Socket createSocket() throws IOException {
			return sslContext.getSocketFactory().createSocket();
		}
	}
}

class HTTPThread extends Thread {
	long id_;
	long udata_;
	String url_;
	String[] headers_;
	byte[] data_;
	HTTPManager manager_;
	int method_;

	volatile boolean close;

	public HTTPThread(String url, String[] headers, byte[] data, int method,
			long udata, long id, HTTPManager manager) {
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
	public void run() {
		try {
			DefaultHttpClient httpClient;
			if (HTTPManager.ignoreSslErrors)
				httpClient=new TrustAllHttpClient();
			else
				httpClient=new DefaultHttpClient();
			
			if (HTTPManager.proxyName!=null)
			{
				HttpHost proxy = new HttpHost(HTTPManager.proxyName, HTTPManager.proxyPort); 
				httpClient.getParams().setParameter(ConnRouteParams.DEFAULT_PROXY, proxy);
				if (HTTPManager.proxyUser!=null)
					httpClient.getCredentialsProvider().setCredentials(  
							new AuthScope(HTTPManager.proxyName, HTTPManager.proxyPort),  
							new UsernamePasswordCredentials(
									HTTPManager.proxyUser,HTTPManager.proxyPass));
			}

			HttpUriRequest method = null;
			switch (method_) {
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
			int contentLength = (int) response.getEntity().getContentLength();
			InputStream input = response.getEntity().getContent();
			ByteArrayOutputStream output = new ByteArrayOutputStream();

			int readBytes = 0;
			byte[] sBuffer = new byte[1024];
			while ((readBytes = input.read(sBuffer)) != -1) {
				if (close == true) {
					httpClient.getConnectionManager().shutdown();
					return;
				}
				output.write(sBuffer, 0, readBytes);
				manager_.progressCallback(id_, udata_, output.size(),
						contentLength);
			}
			int hdrCount=0;
			int dataSize=output.size();
			
			Header[] hdr=response.getAllHeaders();
			for (Header h:hdr)
			{
				output.write(h.getName().getBytes());
				output.write(0);
				output.write(h.getValue().getBytes());
				output.write(0);
				hdrCount++;
			}

			httpClient.getConnectionManager().shutdown();

			manager_.responseCallback(id_, udata_, output.toByteArray(),
					statusCode,hdrCount,dataSize);
		} catch (Exception e) {
			manager_.errorCallback(id_, udata_);
		}
	}
}
