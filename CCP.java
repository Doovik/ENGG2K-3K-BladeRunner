//UDP imports
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
//JSON imports
//import org.json.simple.*;
import org.json.simple.parser.*;
//import java.util.Iterator;
import java.io.FileNotFoundException;
import java.io.FileReader;
//import java.io.FileWriter;
import java.io.IOException;
import org.json.JSONObject;

public class CCP {
    private static final int PORT = 3014;
    private static final int BUFFER_SIZE = 256;
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
    static String sequence_number;
    static String action;
    static String status;
    static String br_id;
    //initialise JSONParser
    static JSONParser parser = new JSONParser();
    static JSONObject jsonWrite = new JSONObject();

    final static String client = "ccp";
    public static void main(String[] args) {
        //UDP

        Boolean checkedIn = false;

        String status = "";

        Boolean statusChanged = false;

        try(DatagramSocket socket = new DatagramSocket(PORT)) {
            
            byte[] buffer = new byte[BUFFER_SIZE];

            System.out.println("Server is listening on port " + PORT);

            while (true) {
                DatagramPacket packet = new DatagramPacket(buffer, buffer.length);
                //socket.receive(packet);

                //String received = new String(packet.getData(), 0, packet.getLength());
                //System.out.println("Received from client: " + received);

                //InetAddress address = packet.getAddress();
                //int clientPort = packet.getPort();
                InetAddress address = InetAddress.getByName("10.20.30.177");
                int clientPort = 2000;

                JSONObject toSend = new JSONObject();

                // Send response to client

                /*
                String response = "Echo: " + received;
                InetAddress address = packet.getAddress();
                int clientPort = packet.getPort();
                packet = new DatagramPacket(response.getBytes(), response.length(), address, clientPort);
                socket.send(packet);
                */

                // ///////////////////////////// //

                // Send initialisation message if not sent or not acknowledged
                if (!checkedIn) {
                    System.out.println("something");
                    String initiationMessage = jsonWrite(toSend, "CCIN", "BRXX", "s_ccp");
                    packet = new DatagramPacket(initiationMessage.getBytes(), initiationMessage.length(), address, clientPort);
                    socket.send(packet);
                }
                
                socket.receive(packet);

                System.out.println(packet.getLength());

                String received = new String(packet.getData(), 0, packet.getLength());
                System.out.println("Received from client: " + received);

                JSONObject json = new JSONObject(received);
                String messageContent = json.getString("message");

                // Check if initialisation was acknowledged
                if (messageContent.equals("AKIN")) {
                    checkedIn = true;
                } else { continue; }

                // Acknowledge received message
                if (messageContent.equals("EXEC")) {
                    String acknowledge = jsonWrite(toSend, "AKEX", "BRXX", "s_ccp");
                    packet = new DatagramPacket(acknowledge.getBytes(), acknowledge.length(), address, clientPort);
                    socket.send(packet);
                }

                // CCP status message
                // TODO send upon completing an action + actual status
                if(statusChanged) {
                    String statusMsg = jsonWrite(toSend, "STAT", "BRXX", "s_ccp", "ERR");
                    packet = new DatagramPacket(statusMsg.getBytes(), statusMsg.length(), address, clientPort);
                    socket.send(packet);

                    statusChanged = false;
                }
                
                // ///////////////////////////// //
                
            }
        } catch (Exception ex) {
            System.out.println("Server exception: " + ex.getMessage());
            ex.printStackTrace();
        }

        //JSON read/write
        //jsonWrite(jsonWrite, " ", " ", Status.STOPPED, " "); //TODO - determine values


    }

    @SuppressWarnings("unchecked")
    static String jsonWrite(JSONObject jobj, String message, String clientID, String sequence_number) {
        jobj.put("client_type", client);
        jobj.put("message", message);
        jobj.put("client_id", clientID);
        jobj.put("sequence_number", sequence_number);

        return jobj.toString();
        /*
        try (FileWriter file = new FileWriter("C:\\ENGG3000_BR_2\\New folder\\ENGG2K-3K-BladeRunner\\test.json")) {
            //We can write any JSONArray or JSONObject instance to the file
            file.write(jobj.toJSONString()); 
            file.flush();
 
        } catch (IOException e) {
            e.printStackTrace();
        }
        */
    }
    static String jsonWrite(JSONObject jobj, String message, String clientID, String sequence_number, String status) {
        jobj.put("client_type", client);
        jobj.put("message", message);
        jobj.put("client_id", clientID);
        jobj.put("sequence_number", sequence_number);
        jobj.put("status", status);

        return jobj.toString();
    }

    static void jsonRead() {
        try {
            JSONObject obj = (JSONObject)parser.parse(new FileReader("C:\\ENGG3000_BR_2\\New folder\\ENGG2K-3K-BladeRunner\\test.json")); //Get JSONObject
            //fetching values from JSON Object
            client_type = (String) obj.get("client_type");
            message = (String) obj.get("message");
            client_id = (String) obj.get("client_id");
            sequence_number = (String) obj.get("sequence number");
            action = (String) obj.get("action");
            status = (String) obj.get("status");
            br_id = (String) obj.get("br_id");
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