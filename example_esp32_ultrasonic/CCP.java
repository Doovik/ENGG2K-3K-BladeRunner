//UDP imports
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
//JSON imports
import org.json.simple.parser.JSONParser;

public class CCP {
    public static void main(String[] args) {
        //JSON read
        JSONParser parser = new JSONParser();

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
    }
}