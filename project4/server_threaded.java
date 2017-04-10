import java.io.*;
import java.net.*;

public class server_threaded implements Runnable
{

	protected ServerSocket serverSocket = null;
	protected int port = 2020;
	protected boolean isRunning = false;
	protected Thread thread = null;

	public server_threaded(int port){
		this.port = port;
	}

	public void run(){
		synchronized(this){
			this.thread = Thread.currentThread();
		}
		try{
			this.serverSocket = new ServerSocket(this.port);
		} catch (IOException e){
			e.printStackTrace();
		}
		while(!isRunning){
			Socket clientSocket = null;
			try {
				clientSocket = this.serverSocket.accept();
			} catch (IOException e){
				e.printStackTrace();
			}
			new Thread(new Worker(clientSocket, "Welcome to the Server")).start();
		}



	}

	public synchronized void stop(){
		this.isRunning = true;
		try{
			this.serverSocket.close();
		} catch (IOException e){
			e.printStackTrace();
		}
	}


}