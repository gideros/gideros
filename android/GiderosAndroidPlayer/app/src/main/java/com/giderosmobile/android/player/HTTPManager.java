package com.giderosmobile.android.player;

import android.util.Log;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.Authenticator;
import java.net.HttpURLConnection;
import java.net.InetSocketAddress;
import java.net.PasswordAuthentication;
import java.net.Proxy;
import java.net.Socket;
import java.net.URL;
import java.net.UnknownHostException;
import java.security.KeyManagementException;
import java.security.KeyStore;
import java.security.KeyStoreException;
import java.security.NoSuchAlgorithmException;
import java.security.UnrecoverableKeyException;
import java.security.cert.CertificateException;
import java.security.cert.X509Certificate;
import java.util.List;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;

import javax.net.ssl.HostnameVerifier;
import javax.net.ssl.HttpsURLConnection;
import javax.net.ssl.SSLContext;
import javax.net.ssl.SSLSession;
import javax.net.ssl.SSLSocketFactory;
import javax.net.ssl.TrustManager;
import javax.net.ssl.X509TrustManager;

public class HTTPManager {
	static boolean ignoreSslErrors = false;
	static String proxyName, proxyUser, proxyPass;
	static int proxyPort = 0;

	void Get(String url, String[] headers, boolean streaming, long udata, long id) {
		HTTPThread thread = new HTTPThread(url, headers, null, 0, streaming, udata, id,
				this);

		ids_.put(id, thread);

		thread.start();
	}

	void Post(String url, String[] headers, byte[] data, boolean streaming, long udata, long id) {
		HTTPThread thread = new HTTPThread(url, headers, data, 1, streaming, udata, id,
				this);

		ids_.put(id, thread);

		thread.start();
	}

	void Put(String url, String[] headers, byte[] data, boolean streaming, long udata, long id) {
		HTTPThread thread = new HTTPThread(url, headers, data, 2, streaming, udata, id,
				this);

		ids_.put(id, thread);

		thread.start();
	}

	void Delete(String url, String[] headers, boolean streaming, long udata, long id) {
		HTTPThread thread = new HTTPThread(url, headers, null, 3, streaming, udata, id,
				this);

		ids_.put(id, thread);

		thread.start();
	}

	void Close(long id) {
		HTTPThread thread = ids_.get(id);
		if (thread == null)
			return;


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
														  int size, int statusCode, int hdrCount, int hdrSize, boolean header, long udata);

	public static native void nativeghttpErrorCallback(long id, long udata);

	public static native void nativeghttpProgressCallback(long id,
														  int bytesLoaded, int bytesTotal, byte[] data, int size, long udata);

	ConcurrentHashMap<Long, HTTPThread> ids_ = new ConcurrentHashMap<Long, HTTPThread>();

	public void responseCallback(long id, long udata, byte[] data,
								 int statusCode, int hdrCount, int dataSize, boolean header) {
		if (!ids_.containsKey(id))
			return;

		nativeghttpResponseCallback(id, data, dataSize, statusCode, hdrCount, data.length - dataSize, header, udata);

		if (!header)
			ids_.remove(id);
	}

	public void errorCallback(long id, long udata) {
		if (!ids_.containsKey(id))
			return;

		nativeghttpErrorCallback(id, udata);

		ids_.remove(id);
	}

	public void progressCallback(long id, long udata, int bytesLoaded,
								 int bytesTotal, byte[] data, int size) {
		if (!ids_.containsKey(id))
			return;

		nativeghttpProgressCallback(id, bytesLoaded, bytesTotal, data, size, udata);
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

	static public void ghttp_SetProxy(String name, int port, String user, String pass) {
		HTTPManager.proxyName = name;
		HTTPManager.proxyPort = port;
		HTTPManager.proxyUser = user;
		HTTPManager.proxyPass = pass;
	}

	static public void ghttp_Get(String url, String[] headers, boolean streaming, long udata,
								 long id) {
		httpManager.Get(url, headers, streaming, udata, id);
	}

	static public void ghttp_Post(String url, String[] headers, byte[] data, boolean streaming,
								  long udata, long id) {
		httpManager.Post(url, headers, data, streaming, udata, id);
	}

	static public void ghttp_Put(String url, String[] headers, byte[] data, boolean streaming,
								 long udata, long id) {
		httpManager.Put(url, headers, data, streaming, udata, id);
	}

	static public void ghttp_Delete(String url, String[] headers, boolean streaming, long udata,
									long id) {
		httpManager.Delete(url, headers, streaming, udata, id);
	}

	static public void ghttp_Close(long id) {
		httpManager.Close(id);
	}

	static public void ghttp_CloseAll() {
		httpManager.CloseAll();
	}


	TrustManager trustAll = new X509TrustManager() {
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

	HostnameVerifier yesHotnameVerifier = new HostnameVerifier() {
		public boolean verify(String arg0, SSLSession arg1) {
			return true;
		}
	};


	class HTTPThread extends Thread {
		long id_;
		long udata_;
		String url_;
		String[] headers_;
		byte[] data_;
		HTTPManager manager_;
		int method_;
		boolean streaming_;

		volatile boolean close;

		public HTTPThread(String url, String[] headers, byte[] data, int method, boolean streaming,
						  long udata, long id, HTTPManager manager) {
			url_ = url;
			headers_ = headers;
			data_ = data;
			method_ = method;
			udata_ = udata;
			id_ = id;
			manager_ = manager;
			streaming_ = streaming;

			close = false;
		}

		@Override
		public void run() {
			try {
				HttpURLConnection conn;

				URL url = new URL(url_);
				if (HTTPManager.proxyName != null) {
					Proxy proxy = new Proxy(Proxy.Type.HTTP, new InetSocketAddress(HTTPManager.proxyName, HTTPManager.proxyPort));
					Authenticator authenticator = new Authenticator() {
						public PasswordAuthentication getPasswordAuthentication() {
							return (new PasswordAuthentication(HTTPManager.proxyUser, HTTPManager.proxyPass.toCharArray()));
						}
					};
					Authenticator.setDefault(authenticator);
					conn = (HttpURLConnection) url.openConnection(proxy);
				} else
					conn = (HttpURLConnection) url.openConnection();
				if (streaming_)
					conn.setChunkedStreamingMode(0);
				if (HTTPManager.ignoreSslErrors) {
					SSLContext sc = SSLContext.getInstance("SSL");
					sc.init(null, new TrustManager[]{trustAll}, new java.security.SecureRandom());
					SSLContext.setDefault(sc);
					((HttpsURLConnection) conn).setSSLSocketFactory(sc.getSocketFactory());
					((HttpsURLConnection) conn).setHostnameVerifier(yesHotnameVerifier);
				}

				conn.setConnectTimeout(5000);
				conn.setReadTimeout(300000);

				if (headers_ != null)
					for (int i = 0; i < headers_.length; i += 2)
						conn.setRequestProperty(headers_[i], headers_[i + 1]);
				conn.setRequestProperty("User-Agent", "Gideros");

				if (data_ != null) {
					conn.setDoOutput(true);
					OutputStream os = conn.getOutputStream();
					os.write(data_);
					os.flush();
				}

				InputStream input = conn.getInputStream();
				int statusCode = conn.getResponseCode();
				int contentLength = conn.getContentLength();
				ByteArrayOutputStream output = new ByteArrayOutputStream();
				Map<String, List<String>> hdr = conn.getHeaderFields();

				if (streaming_) {
					int hdrCount = 0;

					for (Map.Entry<String, List<String>> h : hdr.entrySet()) {
						for (String v : h.getValue()) {
							if ((h.getKey()!=null)&&(v!=null)) {
								output.write(h.getKey().getBytes());
								output.write(0);
								output.write(v.getBytes());
								output.write(0);
								hdrCount++;
							}
						}
					}
					manager_.responseCallback(id_, udata_, output.toByteArray(),
							statusCode, hdrCount, 0, true);
					output.reset();
				}
				int readBytes = 0;
				byte[] sBuffer = new byte[1024];
				while ((readBytes = input.read(sBuffer)) != -1) {
					if (close == true) {
						input.close();
						conn.disconnect();
						return;
					}
					if (!streaming_)
						output.write(sBuffer, 0, readBytes);
					manager_.progressCallback(id_, udata_, output.size(),
							contentLength, streaming_ ? sBuffer : null, readBytes);
				}
				input.close();
				int hdrCount = 0;
				int dataSize = output.size();

				for (Map.Entry<String, List<String>> h : hdr.entrySet()) {
					for (String v : h.getValue()) {
						if ((h.getKey()!=null)&&(v!=null)) {
							output.write(h.getKey().getBytes());
							output.write(0);
							output.write(v.getBytes());
							output.write(0);
							hdrCount++;
						}
					}
				}

				conn.disconnect();

				manager_.responseCallback(id_, udata_, output.toByteArray(),
						statusCode, hdrCount, dataSize, false);
			} catch (Exception e) {
				Log.e("Gideros", "HTTP exception", e);
				manager_.errorCallback(id_, udata_);
			}
		}
	}
}