//UDP imports
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
//JSON imports
import org.json.simple.*;
import org.json.simple.parser.*;
//import java.util.Iterator;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;

public class CCP {
    //enum for current carriage status
    enum Status {
        STOPC, //stopped + door closed
        STOPO, //stopped + door opened
        FSLOWC, //forward, slow, door closed
        FFASTC, //forward, fast, door closed
        RSLOWC, //backwards, slow, door closed
        ERR, //CPP <-> BRC contact lost
    }
    //initialise empty values for jsonRead
    static String client_type;
    static String message;
    static String client_id;
    static String timestamp;
    static String action;
    static String fileLoc = "";
    //initialise JSONParser
    static JSONParser parser = new JSONParser();
    static JSONObject jsonWrite = new JSONObject();

    final static String client = "ccp";
    public static void main(String[] args) {
        //UDP
        int port = 3014;
        try (DatagramSocket socket = new DatagramSocket(port)) {
            byte[] buffer = new byte[256];

            System.out.println("Server is listening on port " + port);

            jsonRead(); //TODO put jsonRead and jsonWrite inside the loop
            jsonWrite(jsonWrite, "TEST-MSG", "TEST-ID", 9999, Status.ERR, "TEST-SID");
            while (true) {
                DatagramPacket packet = new DatagramPacket(buffer, buffer.length);
                socket.receive(packet);

                String received = new String(packet.getData(), 0, packet.getLength());
                System.out.println("Received from client: " + received);

                // Send response to client
                String response = "Echo: " + received;
                InetAddress address = packet.getAddress();
                int clientPort = packet.getPort();
                packet = new DatagramPacket(response.getBytes(), response.length(), address, clientPort);
                socket.send(packet);
                //Write to JSON
                
            }
        } catch (Exception ex) {
            System.out.println("Server exception: " + ex.getMessage());
            ex.printStackTrace();
        }

        //JSON read/write
        //jsonWrite(jsonWrite, " ", " ", Status.STOPPED, " "); //TODO - determine values


    }

    @SuppressWarnings("unchecked")
    static void jsonWrite(JSONObject jobj, String message, String clientID, int sequence_number, Status status, String stationID) {
        jobj.put("client_type", client);
        jobj.put("message", message);
        jobj.put("client_id", clientID);
        jobj.put("sequence_number", " ");
        jobj.put("action", action);
        jobj.put("status", status.toString());
        if (status == Status.ERR) {
            jobj.put("station_id", stationID);
        }
        try (FileWriter file = new FileWriter("C:\\ENGG3000_BR_2\\New folder\\ENGG2K-3K-BladeRunner\\test.json")) {
            //We can write any JSONArray or JSONObject instance to the file
            file.write(jobj.toJSONString()); 
            file.flush();
 
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    static void jsonRead() {
        try {
            JSONObject obj = (JSONObject)parser.parse(new FileReader("C:\\ENGG3000_BR_2\\New folder\\ENGG2K-3K-BladeRunner\\test.json")); //Get JSONObject
            //fetching values from JSON Object
            client_type = (String) obj.get("client_type");
            message = (String) obj.get("message");
            client_id = (String) obj.get("client_id");
            timestamp = (String) obj.get("timestamp");
            action = (String) obj.get("action");
            System.out.println(client_type + "\n" + message + "\n" + client_id + "\n" + 
                               timestamp + "\n" + action + "\n" + action); //TODO - remove or replace with logger
        }
        catch(FileNotFoundException e) {
            e.printStackTrace();
        }
        catch(IOException e){
            e.printStackTrace();
        }
        catch(ParseException e){
            e.printStackTrace();
        }
    }
}