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
import java.io.IOException;

public class CCP {
    enum Status {
        STOPPED,
        STARTED,
        ON,
        OFF,
        ERR,
        CRASH,
        STOPPED_AT_STATION
    }
    static String fileLoc = "";
    final static String client = "ccp";
    public static void main(String[] args) {
        //JSON read
        //initialise empty values;
        String client_type;
        String message;
        String client_id;
        String timestamp;
        String action;
        //initialise JSONParser
        JSONParser parser = new JSONParser();
        try {
            JSONObject obj = (JSONObject)parser.parse(new FileReader(fileLoc)); //Get JSONObject
            //fetching values from JSON Object
            client_type = (String) obj.get("client_type");
            message = (String) obj.get("message");
            client_id = (String) obj.get("client_id");
            timestamp = (String) obj.get("timestamp");
            action = (String) obj.get("action");
        }
        catch(FileNotFoundException e){
            e.printStackTrace();
        }
        catch(IOException e){
            e.printStackTrace();
        }
        catch(ParseException e){
            e.printStackTrace();
        }
        //UDP
        int port = 3014;
        try (DatagramSocket socket = new DatagramSocket(port)) {
            byte[] buffer = new byte[256];

            System.out.println("Server is listening on port " + port);

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
            }
        } catch (Exception ex) {
            System.out.println("Server exception: " + ex.getMessage());
            ex.printStackTrace();
        }

        //JSON write
        JSONObject jsonWrite = new JSONObject();
        jsonWrite(jsonWrite, " ", " ", Status.STOPPED, " "); //TODO - determine values


    }

    @SuppressWarnings("unchecked")
    static void jsonWrite(JSONObject jobj, String message, String clientID, Status status, String stationID) {
        jobj.put("client_type", client);
        jobj.put("message", message);
        jobj.put("client_id", clientID);
        jobj.put("timestamp", " "); //TODO - check which func can do the format
        jobj.put("status", status);
        if (status == Status.STOPPED_AT_STATION) {
            jobj.put("station_id", stationID);
        }
    }
}