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
		ConcurrentHashMap<Integer, Cryptoblob> encryption_map = new ConcurrentHashMap<Integer, Cryptoblob>();
		ConcurrentHashMap<Integer, Socket> clients = new ConcurrentHashMap<Integer, Socket>();
		ConcurrentHashMap<Integer, SecretKey> symKey = new ConcurrentHashMap<Integer, SecretKey>();
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
				EchoHandler handler = new EchoHandler(clientCnt, client, clients, encryption_map, symKey);
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
	int clientId;
    Socket client;
	ConcurrentHashMap<Integer, Socket> clients;
	ConcurrentHashMap<Integer, Cryptoblob> client_keys = new ConcurrentHashMap<Integer, Cryptoblob>();
	ConcurrentHashMap<Integer, SecretKey> symKey = new ConcurrentHashMap<Integer, SecretKey>();
	
    Cryptoblob server_blob;
	Boolean encryptionStatus;
	SecretKey sym;
	IvParameterSpec iv;

	public EchoHandler (int clientId, Socket client, ConcurrentHashMap<Integer, Socket> cl, ConcurrentHashMap<Integer, Cryptoblob> ck, ConcurrentHashMap<Integer, SecretKey> symKey) {
		this.clientId = clientId;
        this.client = client;
		this.clients = cl;
		this.client_keys = ck;
		this.encryptionStatus = false;
		this.server_blob = client_keys.get(0);
	    this.symKey = symKey;
    }

	private void encryptionSetup(){
		try{
			PublicKey publicKey = this.server_blob.getPublicKey();
			PrintWriter out = new PrintWriter(this.client.getOutputStream(), true);
			ObjectOutputStream objectOut = new ObjectOutputStream(client.getOutputStream());
			objectOut.writeObject(publicKey);
			objectOut.flush();

			InputStream in = client.getInputStream();
    		DataInputStream dis = new DataInputStream(in);
            byte rec[] = new byte[80];
    		byte ivbytes[] = new byte[16];
            byte clientSymmetric[] = new byte[64];
    		byte decryptedsecret[] = new byte[128];
    		dis.readFully(rec);
            //get iv
            for(int i = 0; i < 16; ++i){
                ivbytes[i] = rec[i];
            }
						
			System.out.println("IV Encrypted bytes");

			for(int i = 0; i < 16; ++i){
               System.out.print(ivbytes[i]);     
            }


            // get client symmetric
            for(int i = 16; i < 80; ++i){
               clientSymmetric[i - 16] = rec[i];
            }

			System.out.println("\n\nClient Symmetric Encrypted bytes");

    		for(int i = 0; i < 64; ++i){
               System.out.print(clientSymmetric[i]);     
            }


    		iv = new IvParameterSpec(ivbytes);
    		decryptedsecret = server_blob.RSADecrypt(clientSymmetric);
    		sym = new SecretKeySpec(decryptedsecret, 0, decryptedsecret.length, "AES");
		    symKey.put(clientId, sym);	
                
            
                
            
			// testing
            /*
			String x = "Hello";
			byte x1[] = x.getBytes();
    		byte message[] = server_blob.encrypt(x1, sym, iv);
    		byte finaltest[] = server_blob.decrypt(message, sym, iv);
    		String s = new String(finaltest);
    		System.out.println("\n\n" + s);

    		/*
    		String cipher = ""; 
    		String ivbytes = "";
    		System.out.println("Listening");
    		if(dis.available() > 0){
    			cipher = dis.readUTF();
    			ivbytes = dis.readUTF();
    		}

    		byte decryptedsecret[] = server_blob.RSADecrypt(cipher.getBytes());
    		SecretKey symmetricClientKey = new SecretKeySpec(decryptedsecret,"AES");
    		
    		IvParameterSpec iv = new IvParameterSpec(ivbytes.getBytes());

    		byte decryptedplaintext[] = server_blob.decrypt(cipher.getBytes(), symmetricClientKey, iv);

    		System.out.println("IV: " + iv);
    		System.out.println("Cipher: " + cipher);
    		System.out.println("Secret Key: " + decryptedplaintext);
    		*/

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


	private String receiveEncrypted(){
		try {
            byte input[] = new byte[128];
			byte ivBytes[] = new byte[16];
            byte size[] = new byte[4];
            int byteLength;
            System.out.println("Waiting to recieve encrypted text");
    
			InputStream in = client.getInputStream();
    		DataInputStream dis = new DataInputStream(in);
            byteLength = dis.readInt(); 
            System.out.println("Attempting to read " + byteLength + " bytes");
            byte[] cipherText = new byte[byteLength];
            dis.readFully(cipherText);
            System.out.println();
             for(int i = 0; i < cipherText.length; ++i){
                 System.out.print(cipherText[i]);
             }
             System.out.println();
 
           // System.out.printf("CipherText: %s%n",DatatypeConverter.printHexBinary(cipherText)); 
            
            System.out.println("After getting data");
            byte[] output = server_blob.decrypt(cipherText, sym, iv);        
		    	
            String out = new String(output);
            return out;
		}
		catch (Exception e) {
			System.err.println("Exception caught: client disconnected.");
			return null;
		}
		finally {
			try { client.close(); }
			catch (Exception e ){ ; }
		}

	}

	public void run () {

		if (!encryptionStatus){
			encryptionSetup();
			encryptionStatus = true;
		}

		try {

			String inputLine;
			while (true){
				
				System.out.println("Before recieve encrypted");
				inputLine = receiveEncrypted();
				System.out.println("Input Line " + inputLine);
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
