import java.io.*;
import java.net.*;
import java.nio.*;
import java.nio.channels.*;

class udp{
	public static void main(String args[]){
		try{

			DatagramChannel ssc = DatagramChannel.open();

			ssc.bind(new InetSocketAddress(9333));
			
			//not parallel
			while(true){
				//SocketChannel sc = ssc.accept();
				
				sc.close();
			}

		} catch (IOException e){

			System.out.println("Got an IO Exception\n");
		}
	}
}

class TcpServerThread extends Thread{
	SocketChannel sc;

	TcpServerThread(SocketChannel channel){
		sc = channel;
	}

	public void run(){
		try{
			ServerSocketChannel c = ServerSocketChannel.open();
			c.bind(new InetSocketAddress(9333);
			ByteBuffer bbuf = ByteBuffer.allocate(4096);
			sc.read(bbuf);
			String message = new String(bbuf.array());
			System.out.println(message);
		} catch (IOException e){
			System.out.println("Got an IO Exception\n");
		}
	}



}
