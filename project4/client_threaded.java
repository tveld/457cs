import java.io.*;
import java.net.*;

class client_threaded
{
	public static void main(String argv[]) throws Exception
	{
		server_threaded server = new server_threaded(2020);
		new Thread(server).start();

		try {
			Thread.sleep(5000);
		} catch (InterruptedException e){
			e.printStackTrace();
		}

		System.out.println("Stopping server");
	}
}