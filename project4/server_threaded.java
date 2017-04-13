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
	/*****************************************************************************
	 * Function used to get the list of connected clients to the server
	 * This gets printed to the client that has requested the list.
	 *****************************************************************************/
	static private String getListOfClients(
			ConcurrentHashMap<Integer, Socket> clients){
		String clientList="";
		for(int i = 1; i <= clients.size(); ++i){
			clientList += String.valueOf("ID: " + i + " ");
			clientList += "IP: " + clients.get(i).getRemoteSocketAddress().toString();
		}
		return clientList;
	}
	
	/*****************************************************************************
	 * Function used to broadcast the received message from a client to all 
	 * connected clients that are detailed within the hashmap.
	 *****************************************************************************/
	static private String broadcastMessage(
			ConcurrentHashMap<Integer, Socket> clients, String inputLine){
		for(int i = 1; i <= clients.size(); ++i){
			PrintWriter out = null;
			try {
				out = new PrintWriter(clients.get(i).getOutputStream(), true);
			} catch (IOException e) {
				out.println(inputLine);
			}                   
			
			System.out.println("Just printed to client " + i);
		}
		return inputLine;
	}

	public void run () {

		try {

			BufferedReader in = new BufferedReader(
					new InputStreamReader(client.getInputStream()));

			String inputLine;
			while ((inputLine = in.readLine()) != null) {
				if (inputLine.equals("list")){
					PrintWriter out = new PrintWriter(this.client.getOutputStream(), true);
					out.println(getListOfClients(clients));
				}

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
