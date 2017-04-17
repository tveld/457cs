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

			SecretKey symmetricKey = serverKey.generateAESKey();
			byte encryptedsecret[] = serverKey.RSAEncrypt(serverKey.getPublicKey().getEncoded());
			SecureRandom rand = new SecureRandom();
			byte ivbytes[] = new byte[16];
			rand.nextBytes(ivbytes);
			IvParameterSpec iv = new IvParameterSpec(ivbytes);
			byte cipher[] = serverKey.encrypt(symmetricKey.getEncoded(), symmetricKey, iv);

			OutputStream out = server.getOutputStream(); 
    		DataOutputStream dos = new DataOutputStream(out);
    		out.write(cipher.length);
    		System.out.println("IV: " + ivbytes);
    		System.out.println("Cipher: " + cipher);
    		dos.write(cipher, 0, cipher.length);
    		dos.write(ivbytes, 0, ivbytes.length);

    		dos.flush();

    		System.out.println("Matches: " + symmetricKey.getEncoded());

			
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



