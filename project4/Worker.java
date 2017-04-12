import java.io.*;
import java.net.*;

class Worker implements Runnable
{

    Socket clientSocket;
    BufferedReader in = null;

    public client_threaded(Socket client){
        this.clientSocket = client;
    }

    public void run(){
        try{
            in = new BufferedReader(new InputStreamReader(clientSocket.getInputStream()));
            String line;

            while((line = in.readLine()) != null){
                System.out.println(line);
            }
        } catch (IOException e){
            e.printStackTrace();
        }
    }



}