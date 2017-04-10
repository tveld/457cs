import java.io.InputStream;
import java.io.OutputStream;
import java.io.IOException;
import java.net.Socket;

public class Worker implements Runnable{
    protected Socket clientSocket = null;
    protected String text   = null;

    public Worker(Socket clientSocket, String text) {
        this.clientSocket = clientSocket;
        this.text   = text;
    }
    public void run() {
        try {
            InputStream inputstream  = clientSocket.getInputStream();
            OutputStream outputstream = clientSocket.getOutputStream();
            long timetaken = System.currentTimeMillis();
            outputstream.write(("Worker: " + this.text + " - " +timetaken +"").getBytes());
            outputstream.close();
            inputstream.close();
            System.out.println("Your request has processed in time : " + timetaken);
        } catch (IOException e) {           
            e.printStackTrace();
        }
    }
}
