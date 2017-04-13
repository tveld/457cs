import java.io.*;
import java.net.*;
import java.lang.Thread;
import java.util.concurrent.*;

public class server_threaded {
	public static void main (String[] args) {
		ConcurrentHashMap<Integer, Socket> clients = new ConcurrentHashMap<Integer, Socket>();
		try {
			int clientCnt = 0;
			ServerSocket server = new ServerSocket(2020);
			while (true) {
				Socket client = server.accept();
				++clientCnt;
				clients.put(clientCnt, client);
				EchoHandler handler = new EchoHandler(client, clients);
				handler.start();
				System.out.println("Clients Connected: " + clientCnt);
				System.out.println(
					clients.get(clientCnt).getRemoteSocketAddress().toString()
					);
			}
		}
		catch (Exception e) {
			System.err.println("Exception caught:" + e);
		}
	}

}

class EchoHandler extends Thread {
	Socket client;
	ConcurrentHashMap<Integer, Socket> clients;

	public EchoHandler (Socket client, ConcurrentHashMap<Integer, Socket> cl) {
		this.client = client;
		this.clients = cl;
	}

	static private String getListOfClients(ConcurrentHashMap<Integer, Socket> clients){
			String clientList=null;
				for(int i = 1; i <= clients.size(); ++i){
					clientList += String.valueOf(i + clients.get(i).getRemoteSocketAddress().toString() +'\n');
				}
			return clientList;
		}

	public void run () {

		try {

			BufferedReader in = new BufferedReader(
				new InputStreamReader(client.getInputStream()));

			String inputLine;
			while ((inputLine = in.readLine()) != null) {
				if (String.valueOf(inputLine) == "list"){
					System.out.println("Here");
					PrintWriter out = new PrintWriter(client.getOutputStream(), true);
					out.println(getListOfClients(clients));
				}


/*
				for(int i = 1; i <= clients.size(); ++i){
					PrintWriter out = new PrintWriter(clients.get(i).getOutputStream(), true);                   
					out.println(inputLine);
					System.out.println("Just printed to client " + i);
				}
*/


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

