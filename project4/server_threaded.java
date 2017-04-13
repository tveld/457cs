import java.io.*;
import java.net.*;
import java.lang.Thread;
import java.util.Arrays;
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
			String clientList="";
				for(int i = 1; i <= clients.size(); ++i){
					clientList += String.valueOf("ID: " + i + " ");
					clientList += "IP: " + clients.get(i).getRemoteSocketAddress().toString();
					if(i!=clients.size()){
						clientList+= "\n";
					}
				}
			return clientList;
	}
	
	/*****************************************************************************
	 * Function used to broadcast the received message from a client to all 
	 * connected clients that are detailed within the hashmap.
	 *****************************************************************************/
	static private void broadcastMessage(
		ConcurrentHashMap<Integer, Socket> clients, String[] inputSplit){
		for(int i = 1; i <= clients.size(); ++i){
			PrintWriter out = null;
			try {
				out = new PrintWriter(clients.get(i).getOutputStream(), true);
				String message = remakeBroadcast(inputSplit);
				out.println(message);
			} catch (IOException e) {
				e.printStackTrace();
			}                   
			
			System.out.println("Just printed to client " + i);
		}
	}

	static private String remakeBroadcast(String[] inputSplit){
		String message="";
		for(int i=0; i<inputSplit.length; i++){
			if (i==0){
				continue;
			}
			message += String.valueOf(inputSplit[i]) + " ";
		}
		return message;
	}

	static private boolean bootClient(int victim, ConcurrentHashMap<Integer, Socket> clients){
		try{
			if(clients.containsKey(victim)){
				Socket socket = clients.get(victim);
				PrintWriter out = new PrintWriter(socket.getOutputStream(), true);
				out.println("Later nerd. You have been kicked");
				socket.close();
				clients.remove(victim);
				return true;
			} else {
				return false;
			}
		} catch (IOException e){
			e.printStackTrace();
		}
		return false;
	}

	static private String chat(ConcurrentHashMap<Integer, Socket> clients, String message, int id){

		String resp = "";
		if(clients.get(id) != null){
			try {
				PrintWriter out = new PrintWriter(clients.get(id).getOutputStream(), true);
				out.println(message);
				System.out.println("Sent message to client: " + id);
				resp = "Sent message to client";
			} catch (Exception e) {
				System.err.println("Exception caught: client disconnected.");
			}
		} else {
			System.out.println("Could not find client: " + id);
			resp = "Could not find client";
		}

		return resp;
	}





	public void run () {

		try {

			BufferedReader in = new BufferedReader(
					new InputStreamReader(client.getInputStream()));

			String inputLine;
			while ((inputLine = in.readLine()) != null) {
				String inputSplit[] = inputLine.split("\\s+");
				PrintWriter out = new PrintWriter(this.client.getOutputStream(), true);

				if (inputSplit[0].equals("list")){
					out.println(getListOfClients(clients));
				}

				else if (inputSplit[0].equals("boot")){
					int victim = Integer.valueOf(inputSplit[1]);
					if (bootClient(victim, clients)){
						out.println("Client " + victim + " kicked");
					} else {
						out.println("Client does not exist");
					}
				}

				else if (inputSplit[0].equals("broadcast")){
					broadcastMessage(clients, inputSplit);
				}

				else if(inputSplit[0].equals("chat")){
					int id = Integer.parseInt(inputSplit[1]);
					String message = "";
					String[] tmp = Arrays.copyOfRange(inputSplit, 2, inputSplit.length);
					for(int i = 0; i < tmp.length; ++i){
						message += tmp[i]+" ";
					}

					System.out.println("in chat\n");
					PrintWriter send = new PrintWriter(this.client.getOutputStream(), true);
					send.println(chat(clients, message,id));	
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
