import java.net.*;
import java.io.*;
import java.util.*;
import java.security.*;
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

            Listener handler = new Listener(echoSocket);
            handler.start();

			String userInput;
			while ((userInput = stdIn.readLine()) != null) {
				out.println(userInput);
			}
			
		} catch (Exception e){
			e.printStackTrace();
		}


	}

}

class Listener extends Thread {
	Socket server;
	Cryptoblob clientInfo = new Cryptoblob();
	Cryptoblob serverKey = new Cryptoblob();
	Boolean encryptionStatus;

	public Listener (Socket server) {
		this.server = server;
		this.encryptionStatus=false;
	}

	static private void encryptionsetup(Socket server, Cryptoblob serverKey){
		try {
			ObjectInputStream objectIn = new ObjectInputStream(server.getInputStream());
			Object obj = objectIn.readObject();
			System.out.println("Read");
			PublicKey serverPublic = (PublicKey) obj;
			serverKey.setPublicKey(serverPublic);
			
		} catch (Exception e) {
			e.printStackTrace();
		}
	}

	public void run () {

		if(!encryptionStatus){
			encryptionsetup(server, serverKey);
			encryptionStatus = true;
		}

		try {
			BufferedReader in =
			new BufferedReader(
				new InputStreamReader(server.getInputStream()));

			String inputLine;
			while ((inputLine = in.readLine()) != null) {
				
				System.out.println(serverKey.getPublicKey());


				}
		} catch (Exception e) {
			System.err.println("Exception caught: client disconnected.");
		}
		finally {
			try { server.close(); }
			catch (Exception e ){ ; }
		}
	}
}



