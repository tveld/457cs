import java.io.*;
import java.net.*;
import java.util.*;
import javax.crypto.*;
import javax.crypto.spec.*;
import java.security.*;
import java.security.spec.*;
import java.lang.Thread;
import java.util.concurrent.*;

public class server_threaded {
	public static void main (String[] args) {
		HashMap<Integer, Cryptoblob> encryption_map = new HashMap<Integer, Cryptoblob>();
		ConcurrentHashMap<Integer, Socket> clients = new ConcurrentHashMap<Integer, Socket>();
		Cryptoblob server_blob = new Cryptoblob();
		encryption_map.put(0, server_blob);
		server_blob.setPrivateKey("RSApriv.der");
		server_blob.setPublicKey("RSApub.der");

		try {
			int clientCnt = 0;
			ServerSocket server = new ServerSocket(2020);
			while (true) {
				Socket client = server.accept();
				++clientCnt;
				clients.put(clientCnt, client);
				EchoHandler handler = new EchoHandler(client, clients, encryption_map);
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
	HashMap<Integer, Cryptoblob> client_keys = new HashMap<Integer, Cryptoblob>();
	Cryptoblob server_blob;
	Boolean encryptionStatus;

	public EchoHandler (Socket client, ConcurrentHashMap<Integer, Socket> cl, HashMap<Integer, Cryptoblob> ck) {
		this.client = client;
		this.clients = cl;
		this.client_keys = ck;
		this.encryptionStatus = false;
		this.server_blob = client_keys.get(0);
	}

	private static void encryptionSetup(Socket client, Cryptoblob server_blob){
		try{
			PublicKey publicKey = server_blob.getPublicKey();
			PrintWriter out = new PrintWriter(client.getOutputStream(), true);
			ObjectOutputStream objectOut = new ObjectOutputStream(client.getOutputStream());
			objectOut.writeObject(publicKey);
			objectOut.flush();

			InputStream in = client.getInputStream();
    		DataInputStream dis = new DataInputStream(in);
    		byte ivbytes[] = new byte[16];
    		int cipher_length = dis.readChar();
    		System.out.println("Length: " + cipher_length);
    		byte cipher[] = new byte[cipher_length];
    		dis.readFully(cipher);
    		dis.readFully(ivbytes);

    		byte decryptedsecret[] = server_blob.RSADecrypt(cipher);
    		SecretKey symmetricClientKey = new SecretKeySpec(decryptedsecret,"AES");
    		
    		IvParameterSpec iv = new IvParameterSpec(ivbytes);

    		byte decryptedplaintext[] = server_blob.decrypt(cipher, symmetricClientKey, iv);

    		System.out.println("IV: " + iv);
    		System.out.println("Cipher: " + cipher);
    		System.out.println("Secret Key: " + decryptedplaintext);

		} catch (Exception e){
			e.printStackTrace();
		}

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
	 * connected clients that are detailed within the hashMap.
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
				out.println("You have been kicked");
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

		if (!encryptionStatus){
			encryptionSetup(client, server_blob);
			encryptionStatus = true;
		}

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
