import java.net.*;
import java.io.*;


public class client_threaded {


	public static void main(String [] args) throws IOException {


		Socket socket;
		String line;
		BufferedReader in;
		PrintStream os;

		try {
			socket = new Socket("127.0.0.1", 2020);
			in = new BufferedReader(new InputStreamReader(System.in));
			InputStream reader = socket.getInputStream();
			OutputStream outstream = socket.getOutputStream();
			PrintWriter out = new PrintWriter(outstream);

			for(int i=0; i<10; i++){
				System.out.print("Enter a message: ");
				line = in.readLine();
				out.print(line);
			}

			out.close();
			in.close();



			
		} catch (Exception e){
			e.printStackTrace();
		}
			

	}

}