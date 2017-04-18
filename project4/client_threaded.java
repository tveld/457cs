import java.net.*;
import java.io.*;
import java.util.*;
import javax.crypto.*;
import javax.crypto.spec.*;
import java.security.*;
import java.security.spec.*;
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
            ObjectInputStream objectIn = new ObjectInputStream(echoSocket.getInputStream());
            Object obj = objectIn.readObject();
            PublicKey key = (PublicKey) obj;
            if(!handler.getEncryptionStatus()){
            	handler.encryptionsetup(key);
				handler.setEncryptionStatus(true);
            }
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

	public void setEncryptionStatus(Boolean status){
		this.encryptionStatus = status;
	}

	public Boolean getEncryptionStatus(){
		return this.encryptionStatus;
	}

	public void encryptionsetup(PublicKey serverPublic){
		try {
			Socket server = this.server;
			Cryptoblob serverKey = this.serverKey;
			serverKey.setPublicKey(serverPublic);
			serverKey.setPrivateKey("RSApriv.der");

			SecretKey symmetricKey = serverKey.generateAESKey();
			byte encryptedsecret[] = serverKey.RSAEncrypt(symmetricKey.getEncoded());
		    System.out.println("Encrypted size: " + encryptedsecret.length);
	
            SecureRandom rand = new SecureRandom();
			byte ivfinal[] = new byte[16];
			rand.nextBytes(ivfinal);

            for(int i = 0; i < 16; ++i){
               System.out.print(ivfinal[i]);     
            }

            System.out.println("Size: " + ivfinal.length);

            // join the arrays
            byte send[] = new byte[80];
    	    
            // store iv
            for(int i = 0; i < 16; ++i){
                send[i] = ivfinal[i];
            }	

            // get client symmetric
            for(int i = 16; i < 80; ++i){
                send[i] = encryptedsecret[i-16];
            }

            OutputStream out = server.getOutputStream(); 
    		DataOutputStream dos = new DataOutputStream(out);
    		dos.write(send, 0, 80);
    		dos.flush();

    		//SecureRandom rand = new SecureRandom();
			byte ivbytes[] = {16, 13, 65, 20, 48, 102, 72,  15, 25, 18,  26, 64, 16,  50, 38, 17};
			//rand.nextBytes(ivbytes);
			IvParameterSpec iv = new IvParameterSpec(ivbytes);

			String x = "Hello";
			byte x1[] = x.getBytes();
    		byte message[] = serverKey.encrypt(x1, symmetricKey, iv);
    		byte finaltest[] = serverKey.decrypt(message, symmetricKey, iv);
    		String s = new String(finaltest);
    		System.out.println(s);
    	
			/*
			SecureRandom rand = new SecureRandom();
			byte ivbytes[] = new byte[16];
			rand.nextBytes(ivbytes);
			IvParameterSpec iv = new IvParameterSpec(ivbytes);
			byte cipher[] = serverKey.encrypt(symmetricKey.getEncoded(), symmetricKey, iv);

    		byte[] encoded = Base64.getEncoder().encode(cipher.getBytes());
			dos.write(new String(encoded));   // Outputs "SGVsbG8="

    		System.out.println(c);
    		System.out.println("Trying a write");
       		dos.writeUTF(c);
       		dos.flush();
    		dos.writeUTF(ivString);
    		dos.flush();

    		System.out.println("Matches: " + symmetricKey.getEncoded());
			*/
			
		} catch (Exception e) {
			e.printStackTrace();
		}
	}

	public void run () {

		try {
			

			BufferedReader in =
			new BufferedReader(
				new InputStreamReader(server.getInputStream()));

			String inputLine;
			while ((inputLine = in.readLine()) != null) {
				


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



