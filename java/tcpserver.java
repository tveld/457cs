import java.io.*;
import java.net.*;
import java.nio.*;
import java.nio.channels.*;

class tcpserver{
	public static void main(String args[]){
		try{

			ServerSocketChannel ssc = ServerSocketChannel.open();

			ssc.bind(new InetSocketAddress(9856));
			
			//not parallel
			while(true){
				SocketChannel sc = ssc.accept();
				ByteBuffer bbuf = ByteBuffer.allocate(4096);
				sc.read(bbuf);
				String message = new String(bbuf.array());
				System.out.println(message);
				sc.close();
			}

		} catch (IOException e){

			System.out.println("Got an IO Exception\n");
		}
	}
}
