import java.net.*;
import java.io.*;
import java.lang.Thread;
import java.util.concurrent.*;

public class client_threaded {


	public static void main(String [] args) throws IOException {

		try {
            Socket echoSocket = new Socket("127.0.0.1", 2020);
            PrintWriter out =
                new PrintWriter(echoSocket.getOutputStream(), true);
            
            BufferedReader stdIn =
                new BufferedReader(
                    new InputStreamReader(System.in));

            //EchoHandler handler = new EchoHandler(echoSocket);
            //handler.start();

						String userInput;
            while ((userInput = stdIn.readLine()) != null) {
                out.println(userInput);
            }
			
		} catch (Exception e){
			e.printStackTrace();
		}
			

	}

}

class EchoHandler extends Thread {
	Socket server;

	EchoHandler (Socket server) {
			this.server = server;
	}

	public void run () {
		try {
			BufferedReader in =
		              new BufferedReader(
		                  new InputStreamReader(server.getInputStream()));
		
			String inputLine;
			while ((inputLine = in.readLine()) != null) {
				System.out.println("echo: " + inputLine);
			}
		}
		catch (Exception e) {
			System.err.println("Exception caught: client disconnected.");
		}
		finally {
			try { server.close(); }
			catch (Exception e ){ ; }
		}
	}
}
		


