import java.net.*;
import java.io.*;


public class client_threaded {


	public static void main(String [] args) throws IOException {

		try {
            Socket echoSocket = new Socket("127.0.0.1", 2020);
            PrintWriter out =
                new PrintWriter(echoSocket.getOutputStream(), true);
            BufferedReader in =
                new BufferedReader(
                    new InputStreamReader(echoSocket.getInputStream()));
            BufferedReader stdIn =
                new BufferedReader(
                    new InputStreamReader(System.in));
            
            String userInput;
            while ((userInput = stdIn.readLine()) != null) {
                out.println(userInput);
                System.out.println("echo: " + in.readLine());
            }
			
		} catch (Exception e){
			e.printStackTrace();
		}
			

	}

}
