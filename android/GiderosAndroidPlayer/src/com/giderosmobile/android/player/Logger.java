package com.giderosmobile.android.player;

import android.util.Log;

public class Logger
{
	public static void log(String text)
	{
		Log.d("Gideros_Java", text);

/*		File logFile = new File("sdcard/java.log");
		if (!logFile.exists())
		{
			try
			{
				logFile.createNewFile();
			} catch (IOException e)
			{
				e.printStackTrace();
			}
		}
		try
		{
			// BufferedWriter for performance, true to set append to file flag
			BufferedWriter buf = new BufferedWriter(new FileWriter(logFile,
					true));
			buf.append(text);
			buf.newLine();
			buf.close();
		} catch (IOException e)
		{
			e.printStackTrace();
		} */
	}
}
