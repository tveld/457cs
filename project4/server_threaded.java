import java.io.*;
import java.net.*;
import java.lang.Thread;

public class server_threaded {
	public static void main (String[] args) {
		try {
            int clientCnt = 1;
			ServerSocket server = new ServerSocket(2020);
            while (true) {
				Socket client = server.accept();
				EchoHandler handler = new EchoHandler(client);
				handler.start();
                System.out.println("Clients Connected: " + clientCnt);
                ++clientCnt;
			}
		}
		catch (Exception e) {
			System.err.println("Exception caught:" + e);
		}
	}
}

class EchoHandler extends Thread {
	Socket client;
	EchoHandler (Socket client) {
		this.client = client;
	}

	public void run () {
		try {
		    PrintWriter out =
                new PrintWriter(client.getOutputStream(), true);                   
            BufferedReader in = new BufferedReader(
                new InputStreamReader(client.getInputStream()));
            
            String inputLine;
            while ((inputLine = in.readLine()) != null) {
                out.println(inputLine);
                System.out.println("client: " + inputLine);
            }
        }
		catch (Exception e) {
			System.err.println("Exception caught: client disconnected.");
		}
		finally {
			try { client.close(); }
			catch (Exception e ){ ; }
		}
	}
}
